/** CONFIDENTIAL - do not disclose, distribute, or alter without express permission by TORC Robotics.
All use subject to express agreement only. No implied use or license.

Software copyrights by TORC Robotics, LLC as of initial publish date.

Unless required by applicable law or agreed in writing, use of software is on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/

#include "pinpoint_utility_functions.h"

#include "unpack_macros.h"
#include "socket_includes.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


/**
 * Creates and opens a UDP port on the specified port
 * @param  udp_socket socket descriptor to use
 * @param  udp_server pointer to where to store the socket information (sockaddr_in)
 * @param  port       port number to open (choosing 0 finds an open port for you)
 * @return            TRUE if the socket is created successfully, FALSE otherwise
 */
BOOL create_udp_socket(SOCKET *udp_socket, struct sockaddr_in *udp_server, uint16_t port)
{
    char host_name[256];
    struct hostent *hp; // info about this computer

    // Setup the socket to use the UDP protocol (SOCK_DGRAM)
    *udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (*udp_socket == INVALID_SOCKET)
    {
        printf("Could not create socket\n");
        return FALSE;
    }

    // Clear out server struct
    memset((void *)udp_server, '\0', sizeof(struct sockaddr_in));

    // Set family and port of this server
    udp_server->sin_family = AF_INET;
    udp_server->sin_port = port; //default should be 0, which chooses an available port

    // Get host name of this computer
    gethostname(host_name, sizeof(host_name));
    hp = gethostbyname(host_name);

    /* Check for NULL pointer */
    if (hp == NULL)
    {
        fprintf(stderr, "Could not get host name.\n");
        return FALSE;
    }

    // Assign the address or the server (this computer)
#ifdef _WIN32
    udp_server->sin_addr.S_un.S_un_b.s_b1 = hp->h_addr_list[0][0];
    udp_server->sin_addr.S_un.S_un_b.s_b2 = hp->h_addr_list[0][1];
    udp_server->sin_addr.S_un.S_un_b.s_b3 = hp->h_addr_list[0][2];
    udp_server->sin_addr.S_un.S_un_b.s_b4 = hp->h_addr_list[0][3];
#elif __linux
    inet_pton(AF_INET, hp->h_addr_list[0], &(udp_server->sin_addr));
#endif

    // Bind address to socket
    if (bind(*udp_socket, (struct sockaddr *)udp_server, sizeof(struct sockaddr_in)) == -1)
    {
        printf("Could not bind address to socket.\n");
        return FALSE;
    }
    else
    {
        // Get the open UDP port details
        int addrlen = sizeof(struct sockaddr_in);
        if(getsockname(*udp_socket, (struct sockaddr *) udp_server, (socklen_t *)&addrlen) == 0)
        {
            port = htons(udp_server->sin_port);
            printf("Opened Local udp port: %u\n", port);
        }
        else
        {
            printf("Get sock name failed\n");
            return FALSE;
        }
    }

    if (*udp_socket == INVALID_SOCKET)
    {
        printf("ERROR: Receive socket invalid\n");
        return FALSE;
    }
    return TRUE;
}


/**
 * Creates and connects to the PinPoint
 * @param  tcp_socket socket descriptor to use
 * @param  ip_address Address of the PinPoint
 * @return            TRUE if the socket is created successfully, FALSE otherwise
 */
BOOL create_tcp_connection(SOCKET *tcp_socket, const char* ip_address, uint16_t port)
{
    int i_result;
    // address structures for resolving the connection to the PinPoint
    struct hostent *hp;

    // String representation of the port number, for use in getaddrinfo
    char pinpoint_tcp_port_str[6];


    // Resolve the server address and port
    sprintf(pinpoint_tcp_port_str, "%u", port);
    hp = gethostbyname(ip_address);
    if ( hp == NULL ) {
        printf("gethostbyaddr failed\n");
        return FALSE;
    }

    // Create a socket for connecting to PinPoint
    *tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*tcp_socket == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", GET_SOCK_ERRNO);
        return FALSE;
    }

    // Attempt to connect to the PinPoint
    {
        struct sockaddr_in addr;
        char* remoteIP = inet_ntoa (*(struct in_addr *)*hp->h_addr_list);
        
        // Set up the sockaddr structure
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(remoteIP);
        addr.sin_port = htons(port);

        i_result = connect( *tcp_socket, (struct sockaddr*) &addr, sizeof(struct sockaddr));
    }

    if (i_result == SOCKET_ERROR) {
        closesocket(*tcp_socket);
        *tcp_socket = INVALID_SOCKET;
        printf("Socket error: %d\n", GET_SOCK_ERRNO);
        return FALSE;
    }

    // Check that the socket is valid
    if (*tcp_socket == INVALID_SOCKET) {
        printf("Unable to connect to server! error: %d\n", GET_SOCK_ERRNO);
        return FALSE;
    }
    return TRUE;
}

uint32_t get_message_size(uint8_t size_length, SOCKET _socket)
{
    uint8_t buffer[5];
    uint32_t size = 0;
    uint32_t i_result;

    //get the size of the data
    switch(size_length)
    {
    case 0:
        size = 0;
        break;
    case 1:
        i_result = recv(_socket, (char *) buffer, 1, 0);
        if ( i_result == 1 )
        {
            size = UNPACK_UINT8(buffer);
        }
        break;
    case 2:
        i_result = recv(_socket, (char *) buffer, 2, 0);
        if ( i_result == 2 )
        {
            size = UNPACK_UINT16(buffer);
        }
        break;
    case 3:
        i_result = recv(_socket, (char *) buffer, 4, 0);
        if ( i_result == 4 )
        {
            size = UNPACK_UINT32(buffer);
        }
        break;
    default:
        printf("ERROR: unknown size packet length\n");
    }
    return size;
}


/**
* Returns the plain text representation of the status condition passed in
* @param  condition uint8_t condition code
* @return           string representing the condition in plain text
*/
char const* get_status_condition(uint8_t condition)
{
    switch(condition)
    {
    case 0:
        return "clear";
    case 1:
        return "info";
    case 2:
        return "warning";
    case 3:
        return "error";
    default:
        return "ERROR: UNKOWN STATUS CONDITION. DEFAULT PICKED";
    }
    return "not a valid status condition";
}


/**
 * Handles the cleanup of the Winsock api if running windows
 * @return  error code if WSACleanup() fails
 */
int cleanup(void)
{
#ifdef _WIN32
    return WSACleanup();
#else
    return 0;
#endif
}
