#ifndef NET_CONF__H
#define NET_CONF__H 


enum{
	SOCK_RELAY_CTL = 1,
    SOCK_DHCP   =    6
};

#define MY_MAX_DHCP_RETRY           2
#define DHCP_BUF_SIZE            2048

#define RELAY_CTL_PORT     5500
#define EXPIRE_SECS        120
#define THRESHOLD_ADJ_TIME 60
#endif