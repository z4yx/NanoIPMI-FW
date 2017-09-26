#ifndef NET_CONF__H
#define NET_CONF__H 


enum{
    SOCK_MQTT   = 1,
	SOCK_SOL   = 2,
    SOCK_FWUPGRADE = 3,
    SOCK_DHCP   =    6
};

#define MY_MAX_DHCP_RETRY           2
#define DHCP_BUF_SIZE            2048

#endif