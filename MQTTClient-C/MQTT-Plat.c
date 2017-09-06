/*******************************************************************************
 * Copyright (c) 2014, 2017 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *    Ian Craggs - return codes from linux_read
 *******************************************************************************/

#include "MQTT-Plat.h"

#include "stm32f1xx_hal.h"
#include "socket.h"
#include "wizchip_conf.h"
#include "net_conf.h"
#include "network.h"
#include "common.h"

static int sock_read(Network*, unsigned char*, int, int);
static int sock_write(Network*, unsigned char*, int, int);

void TimerInit(Timer* timer)
{
	timer->end_time = 0;
}

char TimerIsExpired(Timer* timer)
{
	uint32_t now = HAL_GetTick();
	return now >= timer->end_time;
}


void TimerCountdownMS(Timer* timer, unsigned int timeout)
{
	uint32_t now = HAL_GetTick();
	timer->end_time = now + timeout;
}


void TimerCountdown(Timer* timer, unsigned int timeout)
{
	uint32_t now = HAL_GetTick();
	timer->end_time = now + timeout*1000;
}


int TimerLeftMS(Timer* timer)
{
	uint32_t now = HAL_GetTick();
	return (timer->end_time <= now) ? 0 : timer->end_time - now;
}


static int sock_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	Timer tim;
	// LOG_DBG("Reading %d bytes", len);
	// setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));
	TimerCountdownMS(&tim, timeout_ms);

	int bytes = 0;
	while (bytes < len && !TimerIsExpired(&tim))
	{
		int rc = recv(n->my_socket, &buffer[bytes], (size_t)(len - bytes));
		if (rc == SOCK_BUSY){
			continue;
		}
		else if(rc > 0){
			bytes += rc;
		}
		else{
			LOG_WARN("recv with %d", rc);
			bytes = -1;
			break;
		}
	}
	if(bytes > 0){
		LOG_VERBOSE("Got %d bytes", bytes);
	}
	// if(TimerIsExpired(&tim))
	// 	LOG_DBG("Reading timed out!");
	return bytes;
}


static int sock_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	LOG_VERBOSE("Writing %d bytes", len);
	// setsockopt(n->my_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(struct timeval));
	int	rc = send(n->my_socket, buffer, len);
	//SOCK_BUSY is 0, just return it
	if(rc < 0)
		LOG_WARN("send with %d", rc);

	return rc;
}


void MQTTPlat_NetworkInit(Network* n)
{
	n->my_socket = 0;
	n->tcp_connected = 0;
	n->mqttread = sock_read;
	n->mqttwrite = sock_write;
}


int MQTTPlat_NetworkConnect(Network* n, char* addr, int port)
{
	int rc = -1;
	if(n->tcp_connected)
		disconnect(n->my_socket);
	n->my_socket = socket(SOCK_MQTT, Sn_MR_TCP, (rand()&0x7fff)+0x8000, SF_IO_NONBLOCK/*|SF_TCP_NODELAY*/);
	if (n->my_socket >= 0)
		rc = connect(n->my_socket, Network_GetMQTTBrokerIP(), port);
	LOG_DBG("NetworkConnect rc=%d", rc);
	if(rc == SOCK_OK)
		n->tcp_connected = 1;
	else if(rc == SOCK_BUSY){
		//In non-blocking mode
		Timer tim;
		TimerCountdownMS(&tim, 5000);
		do{
			uint8_t status;
			getsockopt(n->my_socket, SO_STATUS, &status);
			if(status == SOCK_ESTABLISHED){
				rc = SOCK_OK;
				break;
			}
		}while(!TimerIsExpired(&tim));
		if(rc != SOCK_OK)
			LOG_INFO("TCP connection timeout!");
	}

	if(rc != SOCK_OK)
		return -1;
	else
		return 0;
}


void MQTTPlat_NetworkDisconnect(Network* n)
{
	if(n->tcp_connected)
		disconnect(n->my_socket);
	close(n->my_socket);
	n->tcp_connected = 0;
}
