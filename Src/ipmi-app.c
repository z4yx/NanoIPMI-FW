#include "common.h"
#include "ipmi-app.h"
#include "network.h"
#include "MQTTClient.h"
#include "MQTT-Plat.h"

Network mqtt_net;
MQTTClient mqtt_client = DefaultClient;

static void messageArrived(MessageData* md)
{
    MQTTMessage* message = md->message;

    LOG_DBG("messageArrived %.*s", (int)message->payloadlen, (char*)message->payload);
    // if (opts.showtopics)
    //     printf("%.*s\t", md->topicName->lenstring.len, md->topicName->lenstring.data);
    // if (opts.nodelimiter)
    //     printf("%.*s", (int)message->payloadlen, (char*)message->payload);
    // else
        // printf("%.*s", (int)message->payloadlen, (char*)message->payload);
    //fflush(stdout);
}

static void IPMIApp_InitConn(void)
{
    static unsigned char buf[256];
    static unsigned char readbuf[256];
    int rc;
    char* topic = "/Command/#";

    LOG_DBG("IPMIApp_InitConn called");
    MQTTPlat_NetworkInit(&mqtt_net);
    rc = MQTTPlat_NetworkConnect(&mqtt_net, "dummy", 1883);
    if(rc < 0){
        LOG_INFO("Failed to connect to broker");
        return;
    }
    MQTTClientInit(&mqtt_client, &mqtt_net, 1000, buf, sizeof(buf), readbuf, sizeof(readbuf));
 
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
    data.willFlag = 0;
    data.MQTTVersion = 3;
    data.clientID.cstring = "opts.clientid";
    // data.username.cstring = opts.username;
    // data.password.cstring = opts.password;

    data.keepAliveInterval = 10;
    data.cleansession = 1;
    LOG_INFO("Connecting to broker");
    
    rc = MQTTConnect(&mqtt_client, &data);
    LOG_INFO("MQTTConnect() = %d", rc);
    if(rc != SUCCESS){
        MQTTPlat_NetworkDisconnect(&mqtt_net);
        return;
    }
    
    LOG_INFO("Subscribing to %s", topic);
    rc = MQTTSubscribe(&mqtt_client, topic, QOS1, messageArrived);
    LOG_INFO("MQTTSubscribe() = %d", rc);
    if(rc != SUCCESS){
        MQTTPlat_NetworkDisconnect(&mqtt_net);
        return;
    }
}

void IPMIApp_Task(void)
{
    if(!Network_IsNetworkReady())
        return;
    if(!MQTTIsConnected(&mqtt_client))
        IPMIApp_InitConn();
    else
        MQTTYield(&mqtt_client, 200);
}
