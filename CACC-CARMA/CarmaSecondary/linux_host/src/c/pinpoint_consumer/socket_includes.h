/** CONFIDENTIAL - do not disclose, distribute, or alter without express permission by TORC Robotics.
All use subject to express agreement only. No implied use or license.

Software copyrights by TORC Robotics, LLC as of initial publish date.

Unless required by applicable law or agreed in writing, use of software is on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef SOCKET_INCLUDES_H
#define SOCKET_INCLUDES_H

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>

#define GET_SOCK_ERRNO WSAGetLastError()
#define WOULD_BLOCK_ERROR WSAEWOULDBLOCK

#if defined(_MSC_VER)
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#endif

#elif defined(__linux)

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>

// Function for getting the error number if it calls
#define GET_SOCK_ERRNO errno
#define WOULD_BLOCK_ERROR EAGAIN

#define SOCKET int
#define BOOL int
#define TRUE 1
#define FALSE 0

#define INVALID_SOCKET -1
#define INVALID_HANDLE_VALUE NULL
#define SOCKET_ERROR   -1

#define ioctlsocket(s, i, m) ioctl(s, i, m)
#define closesocket(s) close(s)

#endif

#endif
