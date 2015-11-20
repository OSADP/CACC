/** CONFIDENTIAL - do not disclose, distribute, or alter without express permission by TORC Robotics.
All use subject to express agreement only. No implied use or license.

Software copyrights by TORC Robotics, LLC as of initial publish date.

Unless required by applicable law or agreed in writing, use of software is on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef PINPOINT_UTILITY_FUNCTIONS_H
#define PINPOINT_UTILITY_FUNCTIONS_H

#include "socket_includes.h"
#include <stdint.h>

extern BOOL create_udp_socket(SOCKET *udp_socket, struct sockaddr_in *udp_server, uint16_t port);
extern BOOL create_tcp_connection(SOCKET *tcp_socket, const char* ip_address, uint16_t port);
extern char const* get_status_condition(uint8_t condition);
extern uint32_t get_message_size(uint8_t size_length, SOCKET _socket);
extern int cleanup(void);

#endif
