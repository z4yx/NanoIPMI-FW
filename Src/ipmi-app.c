#include "common.h"
#include "ipmi-app.h"
#include "network.h"
#include "dhcp.h"
#include "MQTTClient.h"
#include "MQTT-Plat.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "control-channel.pb.h"
#include "atx.h"
#include "app-version.h"


static Network mqtt_net;
static MQTTClient mqtt_client = DefaultClient;
static char TOPIC_EVENT_MSG[40], TOPIC_STATUS_MSG[40], TOPIC_HELLO_MSG[40], TOPIC_CMD_MSG[40];
static bool is_ID_function_on, OS_monitor_enabled;
static volatile uint32_t OS_monitor_fed;

static void initTopics(const char* hostname)
{
    if(!hostname || strlen(hostname)==0){
        hostname = "(unknown)";
    }
    snprintf(TOPIC_EVENT_MSG, sizeof(TOPIC_EVENT_MSG)-1, "/%s/Event", hostname);
    snprintf(TOPIC_STATUS_MSG, sizeof(TOPIC_STATUS_MSG)-1, "/%s/Status", hostname);
    snprintf(TOPIC_HELLO_MSG, sizeof(TOPIC_HELLO_MSG)-1, "/%s/Hello", hostname);
    snprintf(TOPIC_CMD_MSG, sizeof(TOPIC_CMD_MSG)-1, "/%s/Command", hostname);
}

static void handleMessage(Command *cmd)
{
    switch(cmd->which_command){
        case 1: //noCommand
            break;
        case 2: //idCommand
            is_ID_function_on = cmd->command.idCommand.on;
            LOG_DBG("set is_ID_function_on = %d", (int)is_ID_function_on);
            break;
        case 3: //powerCommand
            ATX_PowerCommand(cmd->command.powerCommand.op);
            break;
        case 4: //fanCommand
            LOG_DBG("fanCommand op=%d [%d] %d/255",
                cmd->command.fanCommand.op, cmd->command.fanCommand.whichFan, cmd->command.fanCommand.dutyCycle);
            break;
        default:
            LOG_WARN("unknown cmd %d", cmd->which_command);
            break;
    }
}

static int publishStruct(void * data, const pb_field_t fields[], const char* topic, enum QoS qos)
{
    int rc;
    static uint8_t buffer[64];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if(!pb_encode(&stream, fields, data)){
        LOG_ERR("pb_encode failed");
        return FAILURE;
    }
    LOG_DBG("stream.bytes_written=%u", stream.bytes_written);
    MQTTMessage msg = {
        .qos = qos,
        .retained = 0,
        .dup = 0,
        .payload = buffer,
        .payloadlen = stream.bytes_written
    };
    rc = MQTTPublish(&mqtt_client, topic, &msg);
    if(rc != SUCCESS){
        LOG_WARN("%s failed with %d", topic, rc);
        return FAILURE;
    }
    return SUCCESS;
}

static void reportStatus(void)
{
    bool encode_fanRPM(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
    {
        /* This encodes the header for the field, based on the constant info
         * from pb_field_t. */
        if (!pb_encode_tag_for_field(stream, field))
            return false;
        if (!pb_encode_varint(stream, -1))
            return false;

        return true;
    }
    static uint32_t lastReport;
    if(HAL_GetTick() - lastReport > 5000){
        LOG_DBG("prepare to report");
        Status s = {
            .isPowerOn = ATX_GetPowerOnState(),
            .coreTemp = ADC_getTemperatureReading(),
            .isIDOn = is_ID_function_on,
            .isManualFanControl = false,
            .fanRPMs = {
                .funcs.encode = encode_fanRPM,
                .arg = NULL
            },
        };

        publishStruct(&s, Status_fields, TOPIC_STATUS_MSG, QOS0);
        lastReport = HAL_GetTick();
    }

}

static void messageArrived(MessageData* md)
{
    MQTTMessage* message = md->message;

    LOG_DBG("messageArrived %d", (int)message->payloadlen);
    // LOG_DBG("messageArrived %.*s", (int)message->payloadlen, (char*)message->payload);
    // if (opts.showtopics)
    //     printf("%.*s\t", md->topicName->lenstring.len, md->topicName->lenstring.data);
    // if (opts.nodelimiter)
    //     printf("%.*s", (int)message->payloadlen, (char*)message->payload);
    // else
        // printf("%.*s", (int)message->payloadlen, (char*)message->payload);
    //fflush(stdout);

    Command cmd;
    pb_istream_t stream = pb_istream_from_buffer(message->payload, message->payloadlen);
    if(pb_decode(&stream, Command_fields, &cmd))
        handleMessage(&cmd);
}

static void sayHello(void)
{
    int rc;
    Hello hello = {.version = APP_VERSION};
    publishStruct(&hello, Hello_fields, TOPIC_HELLO_MSG, QOS1);
}

static int IPMIApp_InitConn(void)
{
    static unsigned char buf[256];
    static unsigned char readbuf[256];
    int rc;
    char* topic = TOPIC_CMD_MSG;

    LOG_DBG("IPMIApp_InitConn called");
    MQTTPlat_NetworkInit(&mqtt_net);
    rc = MQTTPlat_NetworkConnect(&mqtt_net, "dummy", 1883);
    if(rc != SUCCESS){
        LOG_INFO("Failed to connect to broker");
        return rc;
    }
    MQTTClientInit(&mqtt_client, &mqtt_net, 1000, buf, sizeof(buf), readbuf, sizeof(readbuf));
 
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
    data.willFlag = 0;
    data.MQTTVersion = 3;
    data.clientID.cstring = getHostnamefromDHCP();
    // data.username.cstring = opts.username;
    // data.password.cstring = opts.password;

    data.keepAliveInterval = 10;
    data.cleansession = 1;
    LOG_INFO("Connecting to broker");
    
    rc = MQTTConnect(&mqtt_client, &data);
    LOG_INFO("MQTTConnect() = %d", rc);
    if(rc < 0){
        MQTTPlat_NetworkDisconnect(&mqtt_net);
        return FAILURE;
    }
    
    LOG_INFO("Subscribing to %s", topic);
    rc = MQTTSubscribe(&mqtt_client, topic, QOS1, messageArrived);
    LOG_INFO("MQTTSubscribe() = %d", rc);
    if(rc != SUCCESS){
        MQTTPlat_NetworkDisconnect(&mqtt_net);
        return rc;
    }
    return rc;
}

void IPMIApp_HostUART_RecvCallback(uint8_t data)
{
    //feed watchdog
    OS_monitor_fed = HAL_GetTick();
}
void IPMIApp_CDC_RecvCallback(uint32_t len)
{
    LOG_DBG("CDC_Recv %u", len);
}

void IPMIApp_Task(void)
{
    if(OS_monitor_enabled && HAL_GetTick()-OS_monitor_fed > 15000){
        LOG_INFO("OS Monitor reset triggered");
        ATX_PowerCommand(Command_PowerCommand_PowerOp_RESET);
        OS_monitor_enabled = false;
    }
    else if(OS_monitor_enabled && HAL_GetTick()-OS_monitor_fed > 10000){
        LOG_INFO("No UART output since 10s");
        HostUART_Send_NoWait("\r");
    }
    if(!Network_IsNetworkReady())
        return;
    if(!MQTTIsConnected(&mqtt_client)){
        initTopics(getHostnamefromDHCP());
        if(IPMIApp_InitConn() == SUCCESS){
            LOG_INFO("control channel established");
            sayHello();
        }
    }
    else{
        MQTTYield(&mqtt_client, 200);
        reportStatus();
    }
}
