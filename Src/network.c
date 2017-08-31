#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_spi.h"
#include "wizchip_conf.h"
#include "network.h"
#include "socket.h"
#include "dhcp.h"
#include "common.h"
#include "net_conf.h"
#include "settings.h"
#include "led.h"

wiz_NetInfo gWIZNETINFO = { .mac = {0x78,0xCA,0x83,0x40,0x01,0x08},
                            .ip = {192, 168, 1, 80},
                            .sn = {255, 255, 255, 0},
                            .gw = {192, 168, 1, 1},
                            .dns = {8, 8, 8, 8},
                            .logserver = {192, 168, 1, 1},
                            .dhcp = NETINFO_DHCP
                          };

static uint8_t gDATABUF[DHCP_BUF_SIZE];

static int my_dhcp_retry;

static uint8_t last_link_state;

static __inline unsigned char SPIWriteRead(unsigned char val)
{
    while(!LL_SPI_IsActiveFlag_TXE (SPI1));

    LL_SPI_TransmitData8(SPI1, val);

    while(!LL_SPI_IsActiveFlag_RXNE (SPI1));
    
    return LL_SPI_ReceiveData8(SPI1);
}

static void  wizchip_select(void)
{
    HAL_GPIO_WritePin(W_SEL_GPIO_Port, W_SEL_Pin, GPIO_PIN_RESET);
    // SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Reset);
}

static void  wizchip_deselect(void)
{
    HAL_GPIO_WritePin(W_SEL_GPIO_Port, W_SEL_Pin, GPIO_PIN_SET);
    // SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);
}

static uint8_t wizchip_read()
{
    return SPIWriteRead (0);
}

static void  wizchip_write(uint8_t wb)
{
    SPIWriteRead (wb);
}

static void Net_Conf()
{
    // memcpy(gWIZNETINFO.mac, fixedMAC, sizeof(gWIZNETINFO.mac)); //override mac address
    /* wizchip netconf */
    ctlnetwork(CN_SET_NETINFO, (void*) &gWIZNETINFO);
}

static void Display_Net_Conf(wiz_NetInfo* netInfo)
{
    uint8_t tmpstr[6] = {0,};

    // Display Network Information
    ctlwizchip(CW_GET_ID, (void*)tmpstr);

    if (netInfo->dhcp == NETINFO_DHCP) 
        printf("\r\n===== %s NET CONF : DHCP =====\r\n", (char*)tmpstr);
    else 
        printf("\r\n===== %s NET CONF : Static =====\r\n", (char*)tmpstr);
    printf(" MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", netInfo->mac[0], netInfo->mac[1], netInfo->mac[2], netInfo->mac[3], netInfo->mac[4], netInfo->mac[5]);
    printf(" IP : %d.%d.%d.%d\r\n", netInfo->ip[0], netInfo->ip[1], netInfo->ip[2], netInfo->ip[3]);
    printf(" GW : %d.%d.%d.%d\r\n", netInfo->gw[0], netInfo->gw[1], netInfo->gw[2], netInfo->gw[3]);
    printf(" SN : %d.%d.%d.%d\r\n", netInfo->sn[0], netInfo->sn[1], netInfo->sn[2], netInfo->sn[3]);
    printf(" LOG: %d.%d.%d.%d\r\n", netInfo->logserver[0], netInfo->logserver[1], netInfo->logserver[2], netInfo->logserver[3]);
    printf("=======================================\r\n");
}

static void my_ip_assign(void)
{
    getIPfromDHCP(gWIZNETINFO.ip);
    getGWfromDHCP(gWIZNETINFO.gw);
    getSNfromDHCP(gWIZNETINFO.sn);
    getDNSfromDHCP(gWIZNETINFO.dns);
    getLogServerfromDHCP(gWIZNETINFO.logserver);
    gWIZNETINFO.dhcp = NETINFO_DHCP;
    /* Network initialization */
    Net_Conf();      // apply from DHCP
    Display_Net_Conf(&gWIZNETINFO);
    printf("DHCP LEASED TIME : %ld Sec.\r\n", getDHCPLeasetime());
}

/************************************
 * @ brief Call back for ip Conflict
 ************************************/
void my_ip_conflict(void)
{
    LOG_ERR("CONFLICT IP from DHCP\r\n");
   //halt or reset or any...
   // while(1); // this example is halt.
}

void Network_ChipInit(void)
{
    LL_SPI_Enable (SPI1);
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
    reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);

    uint8_t memsize[2][8] = { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 2, 2, 2, 2, 2, 2, 2, 2 } };
    /* wizchip initialize*/
    if (ctlwizchip(CW_INIT_WIZCHIP, (void*) memsize) == -1) {
        LOG_ERR("WIZCHIP Initialized fail.");
        while (1);
    }

    HAL_Delay (100);

    uint8_t tmpstr[6] = {0};
    ctlwizchip(CW_GET_ID, (void*)tmpstr);
    LOG_INFO("WIZCHIP Found: %s", tmpstr);
}

void Network_AppInit()
{
    if(Settings_IsValid()) {
        LOG_INFO("Loading saved settings");
        gWIZNETINFO = Settings_getNetworkConf();
    }
    Net_Conf();

    // if you want different action instead default ip assign, update, conflict.
    // if cbfunc == 0, act as default.
    reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);

    /* DHCP client Initialization */
    if (gWIZNETINFO.dhcp == NETINFO_DHCP) {
        DHCP_init(SOCK_DHCP, gDATABUF);

    } else {
        // Static
        Display_Net_Conf(&gWIZNETINFO);
    }

    // RelayApp_Init();

    LOG_DBG("%s return", __func__);
}

void Network_ModifyNetconf(wiz_NetInfo* newconf)
{
    dhcp_mode oldmode = gWIZNETINFO.dhcp;

    LOG_INFO("New config:");
    Display_Net_Conf(newconf);

    gWIZNETINFO = *newconf;

    if(oldmode == NETINFO_DHCP) {
        if(newconf->dhcp != NETINFO_DHCP){ //DHCP -> Static
            LOG_INFO("Stopping DHCP");
            DHCP_stop();
            Net_Conf();
        }
    }else if(oldmode == NETINFO_STATIC){
        Net_Conf();
        if(newconf->dhcp == NETINFO_DHCP){
            LOG_INFO("Starting DHCP");
            DHCP_init(SOCK_DHCP, gDATABUF);
        }
    }

    LOG_INFO("Updated:");
    ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);
    Display_Net_Conf(&gWIZNETINFO);
    LOG_INFO("Saveing new settings");
    Settings_updateNetworkConf(newconf);
    LOG_INFO("Done");
}

void link_state_changed(void)
{
    if (gWIZNETINFO.dhcp == NETINFO_DHCP){
        if(last_link_state) {
            LOG_INFO("Link UP, DHCP started");
            DHCP_init(SOCK_DHCP, gDATABUF);
        }else if(!last_link_state){
            LOG_INFO("Link down, DHCP stopped");
            DHCP_stop();
        }
    }

}

uint8_t* Network_GetMQTTBrokerIP(void)
{
    return gWIZNETINFO.logserver;
}

bool Network_IsNetworkReady(void)
{
    return last_link_state && (gWIZNETINFO.dhcp == NETINFO_STATIC ||
        (gWIZNETINFO.dhcp == NETINFO_DHCP && isDHCPLeased()));
}

void Network_Task()
{

    static uint32_t timerPhyState = 0, timerDHCP = 0;
    if(HAL_GetTick()-timerPhyState > 1000){
        uint8_t tmp;
        ctlwizchip(CW_GET_PHYLINK, (void*) &tmp);
        if(tmp != last_link_state){
            last_link_state = tmp;
            LOG_INFO("Link state changed to %d", tmp);
            link_state_changed();
        }
        timerPhyState = HAL_GetTick();
    }
    if(HAL_GetTick()-timerDHCP > 1000){
        DHCP_time_handler();
        timerDHCP = HAL_GetTick();
    }

    if(!last_link_state)
        return;
    
    /* DHCP */
    /* DHCP IP allocation and check the DHCP lease time (for IP renewal) */
    if (gWIZNETINFO.dhcp == NETINFO_DHCP) {
        switch (DHCP_run()) {
        case DHCP_STOPPED:
            LED_Board(2, LED_OFF);
            break;
        case DHCP_IP_ASSIGN:
        case DHCP_IP_CHANGED:
            /* If this block empty, act with default_ip_assign & default_ip_update */
            //
            // This example calls my_ip_assign in the two case.
            //
            // Add to ...
            //
            LOG_DBG("IP Assigned or Changed");
            break;
        case DHCP_IP_LEASED:
            //
            // TODO: insert user's code here
            // run_user_applications = true;
            //
            // LOG_DBG("IP Leased");
            LED_Board(2, LED_ON);
            break;
        case DHCP_FAILED:
            /* ===== Example pseudo code =====  */
            // The below code can be replaced your code or omitted.
            // if omitted, retry to process DHCP

            // my_dhcp_retry++; // never fallback
            if (my_dhcp_retry > MY_MAX_DHCP_RETRY) {
                // gWIZNETINFO.dhcp = NETINFO_STATIC;
                DHCP_stop();      // if restart, recall DHCP_init()
                LOG_ERR(">> DHCP %d Failed\r\n", my_dhcp_retry);
                Net_Conf();
                Display_Net_Conf(&gWIZNETINFO);   // print out static netinfo to serial
                my_dhcp_retry = 0;
            }
            break;
        default:
            break;
        }
    }else{
        LED_Board(2, LED_OFF);
    }
}
