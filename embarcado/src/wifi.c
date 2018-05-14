/*
* wifi.c
*
* Created: 5/2/2018 3:19:54 PM
*  Author: Frederico
*/

#include "asf.h"
#include "main.h"
#include "conf_board.h"
#include <string.h>
#include "bsp/include/nm_bsp.h"
#include "driver/include/m2m_wifi.h"
#include "socket/include/socket.h"


/** IP address of host. */
uint32_t gu32HostIp = 0;

/** TCP client socket handlers. */
static SOCKET tcp_client_socket = -1;

/** Receive buffer definition. */
static uint8_t gau8ReceivedBuffer[MAIN_WIFI_M2M_BUFFER_SIZE] = {0};

/** Wi-Fi status variable. */
static bool gbConnectedWifi = false;

/** Get host IP status variable. */
/** Wi-Fi connection state */
static uint8_t wifi_connected;

/** Instance of HTTP client module. */
static bool gbHostIpByName = false;

/** TCP Connection status variable. */
static bool gbTcpConnection = false;

static int counter = 0;

/** Server host name. */
static char server_host_name[] = MAIN_SERVER_NAME;

volatile xQueueHandle xQueueWifiSend;
volatile xQueueHandle xQueueWifiReceive; 

volatile char userId[32]; // guarda o ID do proximo request




/*
* Check whether "cp" is a valid ascii representation
* of an Internet address and convert to a binary address.
* Returns 1 if the address is valid, 0 if not.
* This replaces inet_addr, the return value from which
* cannot distinguish between failure and a local broadcast address.
*/
/* http://www.cs.cmu.edu/afs/cs/academic/class/15213-f00/unpv12e/libfree/inet_aton.c */
int inet_aton(const char *cp, in_addr *ap)
{
	int dots = 0;
	register u_long acc = 0, addr = 0;

	do {
		register char cc = *cp;

		switch (cc) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			acc = acc * 10 + (cc - '0');
			break;

			case '.':
			if (++dots > 3) {
				return 0;
			}
			/* Fall through */

			case '\0':
			if (acc > 255) {
				return 0;
			}
			addr = addr << 8 | acc;
			acc = 0;
			break;

			default:
			return 0;
		}
	} while (*cp++) ;

	/* Normalize the address */
	if (dots < 3) {
		addr <<= 8 * (3 - dots) ;
	}

	/* Store it if requested */
	if (ap) {
		ap->s_addr = _htonl(addr);
	}

	return 1;
}


/**
* \brief Callback function of IP address.
*
* \param[in] hostName Domain name.
* \param[in] hostIp Server IP.
*
* \return None.
*/
static void resolve_cb(uint8_t *hostName, uint32_t hostIp)
{
	gu32HostIp = hostIp;
	gbHostIpByName = true;
	printf("resolve_cb: %s IP address is %d.%d.%d.%d\r\n\r\n", hostName,
	(int)IPV4_BYTE(hostIp, 0), (int)IPV4_BYTE(hostIp, 1),
	(int)IPV4_BYTE(hostIp, 2), (int)IPV4_BYTE(hostIp, 3));
}

/**
* \brief Callback function of TCP client socket.
*
* \param[in] sock socket handler.
* \param[in] u8Msg Type of Socket notification
* \param[in] pvMsg A structure contains notification informations.
*
* \return None.
*/
static void socket_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg)
{
	//char endpoint[] = "GET / HTTP/1.1\r\n Accept: */*\r\n\r\n";
	/* Check for socket event on TCP socket. */
	if (sock == tcp_client_socket) {
		
		switch (u8Msg) {
			case SOCKET_MSG_CONNECT:
			{
				printf("[Wifi] Socket connected\n");
				if (gbTcpConnection) {
					memset(gau8ReceivedBuffer, 0, sizeof(gau8ReceivedBuffer));
					sprintf((char *)gau8ReceivedBuffer, "GET /?id=%s HTTP/1.1\r\n Accept: */*\r\n\r\n", userId);

					tstrSocketConnectMsg *pstrConnect = (tstrSocketConnectMsg *)pvMsg;
					if (pstrConnect && pstrConnect->s8Error >= SOCK_ERR_NO_ERROR) {
						printf("[Wifi] Sending request \n");
						send(tcp_client_socket, gau8ReceivedBuffer, strlen((char *)gau8ReceivedBuffer), 0);
						memset(gau8ReceivedBuffer, 0, MAIN_WIFI_M2M_BUFFER_SIZE);
						recv(tcp_client_socket, &gau8ReceivedBuffer[0], MAIN_WIFI_M2M_BUFFER_SIZE, 0);
					} else {
						//printf("socket_cb: connect error!\r\n");
						gbTcpConnection = false;
						close(tcp_client_socket);
						tcp_client_socket = -1;
					}
				}
			}
			break;
			
			//SOCKET_MSG_SEND,
			//case


			case SOCKET_MSG_RECV:
			{
				char *pcIndxPtr;
				char *pcEndPtr;
				uint8_t *result;
				int position;
				int response;
			 

				/* No tasks have yet been unblocked. */

				
				//char *str = "sdfadabcGGGGGGGGG";
				//char *result = strstr(str, "abc");
				//int position = result - str;
				//int substringLength = strlen(str) - position;


				tstrSocketRecvMsg *pstrRecv = (tstrSocketRecvMsg *)pvMsg;
				if (pstrRecv && pstrRecv->s16BufferSize > 0) {
					//printf("---- Response ---- \n");
					//printf("%d %s\n" , counter, pstrRecv->pu8Buffer);
					result = strstr(pstrRecv->pu8Buffer, "status");
					
					// FOUND THE RESULT!
					if (result != NULL) {
						position = result - pstrRecv->pu8Buffer;
						response = pstrRecv->pu8Buffer[position + strlen("status\": \"")] == '1' ? 1 : 0;
						printf("[Wifi] RESPONSE %d \n", response);
						xQueueSendFromISR(xQueueWifiReceive, &response, NULL );
					
					}
					
					memset(gau8ReceivedBuffer, 0, sizeof(gau8ReceivedBuffer));
					recv(tcp_client_socket, &gau8ReceivedBuffer[0], MAIN_WIFI_M2M_BUFFER_SIZE, 0);
					
				} else {
					//printf("socket_cb: recv error!\r\n");
					close(tcp_client_socket);
					tcp_client_socket = -1;
				}
			}
			break;

			default:
			break;
		}
	}
}

static void set_dev_name_to_mac(uint8_t *name, uint8_t *mac_addr)
{
	/* Name must be in the format WINC1500_00:00 */
	uint16 len;

	len = m2m_strlen(name);
	if (len >= 5) {
		name[len - 1] = MAIN_HEX2ASCII((mac_addr[5] >> 0) & 0x0f);
		name[len - 2] = MAIN_HEX2ASCII((mac_addr[5] >> 4) & 0x0f);
		name[len - 4] = MAIN_HEX2ASCII((mac_addr[4] >> 0) & 0x0f);
		name[len - 5] = MAIN_HEX2ASCII((mac_addr[4] >> 4) & 0x0f);
	}
}

/**
* \brief Callback to get the Wi-Fi status update.
*
* \param[in] u8MsgType Type of Wi-Fi notification.
* \param[in] pvMsg A pointer to a buffer containing the notification parameters.
*
* \return None.
*/
static void wifi_cb(uint8_t u8MsgType, void *pvMsg)
{
	switch (u8MsgType) {
		case M2M_WIFI_RESP_CON_STATE_CHANGED:
		{
			tstrM2mWifiStateChanged *pstrWifiState = (tstrM2mWifiStateChanged *)pvMsg;
			if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED) {
				printf("[Wifi]: M2M_WIFI_CONNECTED\r\n");
				m2m_wifi_request_dhcp_client();
				} else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
				printf("[Wifi] M2M_WIFI_DISCONNECTED\r\n");
				gbConnectedWifi = false;
				wifi_connected = 0;
			}

			break;
		}

		case M2M_WIFI_REQ_DHCP_CONF:
		{
			uint8_t *pu8IPAddress = (uint8_t *)pvMsg;
			printf("[Wifi]: IP address is %u.%u.%u.%u\r\n",
			pu8IPAddress[0], pu8IPAddress[1], pu8IPAddress[2], pu8IPAddress[3]);
			wifi_connected = M2M_WIFI_CONNECTED;
			
			/* Obtain the IP Address by network name */
			//gethostbyname((uint8_t *)server_host_name);
			break;
		}

		default:
		{
			break;
		}
	}
}

/**
* \brief Main application function.
*
* Initialize system, UART console, network then start weather client.
*
* \return Program return value.
*/
uint8_t taskWifi(void) {
	tstrWifiInitParam param;
	int8_t ret;
	uint8_t mac_addr[6];
	uint8_t u8IsMacAddrValid;
	struct sockaddr_in addr_in;
	
	// limpa o vetor que guarda o ID que sera checado
	memset(userId, 0, 32);
	
	xQueueWifiSend = xQueueCreate(1,sizeof(char[32]));
	if (xQueueWifiSend == NULL) {
		printf("Falha em criar a fila\n");
	}
	
	xQueueWifiReceive = xQueueCreate(1,sizeof(int));
	if (xQueueWifiSend == NULL) {
		printf("Falha em criar a fila\n");
	}
	
	
	/* Initialize the BSP. */
	nm_bsp_init();
	/* Initialize Wi-Fi parameters structure. */
	memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));

	/* Initialize Wi-Fi driver with data and status callbacks. */
	param.pfAppWifiCb = wifi_cb;
	ret = m2m_wifi_init(&param);
	if (M2M_SUCCESS != ret) {
		printf("[Wifi] m2m_wifi_init call error!(%d)\r\n", ret);
		while (1) {
		}
	}
	
	/* Initialize socket module. */
	socketInit();
	/* Register socket callback function. */
	registerSocketCallback(socket_cb, resolve_cb);

	/* Connect to router. */
	printf("[Wifi] connecting to WiFi AP %s...\r\n", (char *)MAIN_WLAN_SSID);
	m2m_wifi_connect((char *)MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID), MAIN_WLAN_AUTH, (char *)MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);

	addr_in.sin_family = AF_INET;
	addr_in.sin_port = _htons(MAIN_SERVER_PORT);
	inet_aton(server_host_name, &addr_in.sin_addr);
	printf("[Wifi] Inet aton : %d", addr_in.sin_addr);

	while(1) {
		m2m_wifi_handle_events(NULL);
		if (wifi_connected == M2M_WIFI_CONNECTED) {
			/* Open client socket. */
			//printf("[Wifi] Wifi connected, waiting for requests...\n");
			if (xQueueReceive(xQueueWifiSend, &userId, 0) == pdTRUE) {
				
				if (tcp_client_socket < 0) {
					printf("[Wifi] socket init \n");
					if ((tcp_client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
						printf("[Wifi] failed to create TCP client socket error!\r\n");
						continue;
					}

					/* Connect server */
					//printf("[Wifi] socket connecting\n");
					printf("[Wifi] userID is %s \n", userId);
				
				
					if (connect(tcp_client_socket, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in)) != SOCK_ERR_NO_ERROR) {
						close(tcp_client_socket);
						tcp_client_socket = -1;
						printf("[Wifi] error\n");
						} else {
						gbTcpConnection = true;
					}
				}
			}
		}
		
		//printf("[Wifi] loop\n");
		vTaskDelay(250);
		
	}
	


	return 0;
}
