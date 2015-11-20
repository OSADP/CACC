/** CONFIDENTIAL - do not disclose, distribute, or alter without express permission by TORC Robotics.
All use subject to express agreement only. No implied use or license.

Software copyrights by TORC Robotics, LLC as of initial publish date.

Unless required by applicable law or agreed in writing, use of software is on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/

//Set the TCP port on the PinPoint that this computer connects to
#define LOCALIZATION_TCP_PORT 9501

#include "unpack_macros.h"
#include "socket_includes.h"
#include "handlers.h"
#include "pinpoint_utility_functions.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Define the default buffer length for data coming in
#define DEFAULT_BUFLEN 2048

/**
* Main Program, which contains all major functionality for connecting to,
*  communicating with, and controlling the TORC PinPoint localization
*  platform, using the Microsoft WINSOCK C library.
*  The supported PinPoint software protocol is version 4.x
*      (All protocol versions from 0x40 to 0x4F are supported by this software)
*
* The program has the following flow to it:
*
* 1) Connect to PinPoint TCP socket
* 2) Initialize communications
*    2a) Verify Protocol Version
*    2b) Receive PinPoint setAPI message and respond
*    2c) Setup UDP port for UDP ping
* 3) Send a BoardInfo message, demonstrating how to request data from the board. This message returns hardware and software information about the PinPoint
* 4) Connect to 3 signals: Global Pose, Local Pose, and Velocity
* 5) Begin processing loop (handle any non-initialization incoming messages including signals and boardinfo message)
*
* @param  argc
* @param  argv argument 1 = IP address of PinPoint
* @return      0 on success, 1 on failure
*/
int main(int argc, char **argv)
{
#ifdef _WIN32
    // Data structure for use with Winsock initialization
    WSADATA wsaData;
#endif
    // Socket data structures for communication
    SOCKET tcp_socket = INVALID_SOCKET;
    SOCKET udp_socket = INVALID_SOCKET;

    // Information about the udp socket on this computer
    struct sockaddr_in udp_server;

    // Receive buffers for tcp and udp data
    uint8_t tcp_recv_buf[DEFAULT_BUFLEN];
    uint8_t udp_recv_buf[DEFAULT_BUFLEN];

    // Temp variable that stores the results of send/recv throughout the program
    int i_result;

    // Create and re-initialize the signal output CSV files
    FILE * local_pose_out = NULL;
    FILE * global_pose_out = NULL;
    FILE * velocity_out = NULL;
    FILE * accel_out = NULL;

    // file output headers
    char local_pose_header[256] = "Date and Time, North (m), East (m), Down (m), Roll (deg), Pitch (deg), Yaw (deg)\n";
    char global_pose_header[256] = "Date and Time, Latitude (deg), Longitude (deg), Altitude (m), Roll (deg), Pitch (deg), Yaw (deg)\n";
    char velocity_header[256] = "Date and Time, Forward Velocity (m/s), Right Velocity (m/s), Down Velocity (m/s), Roll Rate (rad/s), Pitch Rate (rad/s), Yaw Rate (rad/s)\n";
    char accel_header[256] = "Date and Time, Forward Acceleration (m/s^2), Right Acceleration (m/s^2), Down Acceleration (m/s^2)\n";

    // Validate the program input parameters
    if (argc != 4) {
        printf("usage: %s PinPoint_ip_address MicroAutobox_ip_address MicroAutobox_port\n", argv[0]);
        return 1;
    }

	// Open the output socket to MicroAutobox
	char* mab_ip_address = argv[2];
    uint16_t mab_port = (uint16_t) atoi(argv[3]);
	int mab_sock_fd;
	struct sockaddr_in local_sockaddr;
	if (!create_udp_socket(&mab_sock_fd, &local_sockaddr, 0)) {
		printf("Could not open local socket for communication with the MicroAutobox.\n");
		close(mab_sock_fd);
		return 1;
	}

	// Set up the outgoing sockaddr_in struct
	struct sockaddr_in remote_sockaddr;
	memset(&remote_sockaddr, 0, sizeof(struct sockaddr_in));
	remote_sockaddr.sin_family = AF_INET;
	remote_sockaddr.sin_port = htons(mab_port);
	inet_pton(AF_INET, mab_ip_address, &(remote_sockaddr.sin_addr));

    // Open the CSV files
    local_pose_out = fopen ( "local_pose.csv", "w" );
    if (local_pose_out == INVALID_HANDLE_VALUE)
    {
        printf("Could not open local_pose.csv file\n");
        fclose(local_pose_out);
        return 1;
    }

    global_pose_out = fopen ( "global_pose.csv", "w" );
    if (global_pose_out == INVALID_HANDLE_VALUE)
    {
        printf("Could not open global_pose.csv file\n");
        fclose(local_pose_out);
        fclose(global_pose_out);
        return 1;
    }

    velocity_out = fopen ( "velocity_state.csv", "w" );
    if (velocity_out == INVALID_HANDLE_VALUE)
    {
        printf("Could not open velocity_state.csv file\n");
        fclose(local_pose_out);
        fclose(global_pose_out);
        fclose(velocity_out);
        return 1;
    }

    accel_out = fopen ( "body_acceleration.csv", "w" );
    if (accel_out == INVALID_HANDLE_VALUE)
    {
        printf("Could not open body_acceleration.csv file\n");
        fclose(local_pose_out);
        fclose(global_pose_out);
        fclose(velocity_out);
        fclose(accel_out);
        return 1;
    }

    // write headers to csv file
    i_result = fprintf(local_pose_out, "%s", local_pose_header);
    if (i_result < 0)
    {
        perror("writing local pose header to file failed\n");
        fclose(local_pose_out);
        fclose(global_pose_out);
        fclose(velocity_out);
        fclose(accel_out);
        return 1;
    }

    i_result = fprintf(global_pose_out, "%s", global_pose_header);
    if (i_result < 0)
    {
        perror("writing global pose header to file failed\n");
        fclose(local_pose_out);
        fclose(global_pose_out);
        fclose(velocity_out);
        fclose(accel_out);
        return 1;
    }

    i_result = fprintf(velocity_out, "%s", velocity_header);
    if (i_result < 0)
    {
        perror("writing velocity header to file failed\n");
        fclose(local_pose_out);
        fclose(global_pose_out);
        fclose(velocity_out);
        fclose(accel_out);
        return 1;
    }

    i_result = fprintf(accel_out, "%s", accel_header);
    if (i_result < 0)
    {
        perror("writing acceleration header to file failed\n");
        fclose(local_pose_out);
        fclose(global_pose_out);
        fclose(velocity_out);
        fclose(accel_out);
        return 1;
    }

    // retrieve the IP address from the command line
    printf("Connecting to PinPoint on ip address: %s and TCP port %u\n", argv[1], LOCALIZATION_TCP_PORT);

    //***********************************
    // Initialize Winsock on Windows
    //***********************************
#ifdef _WIN32
    i_result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (i_result != 0) {
        printf("WSAStartup failed with error: %d\n", i_result);
        fclose(local_pose_out);
        fclose(global_pose_out);
        fclose(velocity_out);
        fclose(accel_out);
        return 1;
    }
#endif


    //***********************************
    // Connect to the PinPoint TCP port for the 'localization' service
    //***********************************

    //***********************************
    // Connect to the localization service name
    // NOTE: This connection will only be able to communicate with the 'localization'
    // interface. If you want to connect to other interfaces and access their methods and signals,
    // such as the IMU interface, you must connect via the port that is designated for
    // that interface and receive the messages on that socket. Use the web interface to easily
    // identify which ports are used for which interfaces.
    //***********************************
    if (create_tcp_connection(&tcp_socket, argv[1], LOCALIZATION_TCP_PORT) == FALSE)
    {
        printf("ERROR creating TCP connection\n");
        cleanup();
        fclose(local_pose_out);
        fclose(global_pose_out);
        fclose(velocity_out);
        fclose(accel_out);
        return 1;
    }

    //***********************************
    // Disable nagle algorithm
    // The nagle algorithm can cause delays in communication since it tries to combine
    //   small chunks of data before sending. This can cause problems in our case since we
    //   may often send small messages at low frequencies.
    //***********************************
    // Disable nagle flag, 1 = disable, 0 = enable
    {
        char disable_nagle = 1;
        setsockopt( tcp_socket, IPPROTO_TCP, TCP_NODELAY, &disable_nagle, sizeof( disable_nagle ) );
    }

    //***********************************
    // Finished connecting to and setting up the PinPoint TCP Port
    //***********************************

    //***********************************
    // Initial PinPoint communications procedure (see page 20, in version 1.4 of the User Manual)
    // 1) Send/Receive protocol message from PinPoint
    //    1a) Verify that the version are compatible (Most significant nibbles should match)
    // 2) Send/Receive API
    //    2a) Receive the setAPI message from PinPoint (and disregard)
    //    2b) Send an empty setAPI message to PinPoint
    // 3) Setup UDP port communication with the PinPoint
    //    3a) Send/Receive GetUDP port message
    //    3b) Create UDP port on this computer
    //    3c) respond to the getUDP port request from PinPoint with our UDP port
    //    3d) parse the udp port response from PinPoint which tells us which port is open for communication
    //***********************************

    //***********************************
    // 1) Send/Receive protocol message from PinPoint
    //***********************************
    {
        // Version of the protocol that this program is using. see manual for proper version info
        uint8_t protocolVersion = 0x42;
        // Send protocol version message to start communications with PinPoint
        i_result = send( tcp_socket, (char*) &protocolVersion, sizeof(protocolVersion), 0 );
        if (i_result == SOCKET_ERROR) {
            printf("send failed with error: %d\n", GET_SOCK_ERRNO);
            cleanup();
            return 1;
        }

        // Receive protocol version of the PinPoint
        i_result = recv(tcp_socket, (char*) tcp_recv_buf, 1, 0);
        if ( i_result > 0 )
        {
            //check if the 'major' version byte of the protocol matches
            printf("PinPoint protocol version = %02X\n", tcp_recv_buf[0]);
            if ((0xF0 & tcp_recv_buf[0]) != (0xF0 & protocolVersion))
            {
                printf("ERROR: expected protocol version major 0x%02X, received version 0x%02X\n", 0xF0 & protocolVersion, 0xF0 & tcp_recv_buf[0]);
                cleanup();
                return 1;
            }
        }
        else if ( i_result == 0 )
        {
            printf("Connection closed during protocol version receive call\n");
            return 1;
        }
        else
        {
            printf("Protocol Version recv failed with error: %d\n", GET_SOCK_ERRNO);
            return 1;
        }
    }

    //***********************************
    // 2) Send/Receive API
    //***********************************

    // Receive setAPI message. This message contains text describing the
    //      messages and signals that the PinPoint accepts
    // first, get 2 bytes of header...
    i_result = recv(tcp_socket, (char*) tcp_recv_buf, 2, 0);
    if ( i_result == 2 )
    {
        uint8_t control = tcp_recv_buf[0];
        uint8_t id = tcp_recv_buf[1];

        uint8_t protocol_flag = (0x80&control)>>7;
        uint8_t message_type = (0x78&control)>>3;
        uint8_t size_length = (0x03&control);

        // Empty SetApi message that we will send to PinPoint. The setAPI from our program is blank
        //   empty, because our program does not respond to messages, only queries and receives
        uint8_t send_set_api[6] = {0x89, 0x01, 0x03, 0, 0, 0};


        // Determines the number of bytes needed to read in the size portion of the message
        uint32_t size = get_message_size(size_length, tcp_socket);

        if (protocol_flag == 1 && message_type == 0x01 && id == 0x01)
        {
            //Received SetAPI
            printf("Received setAPI message. control = 0x%02X, (message type = 0x%02X), message id=0x%02X, payload size = %u\n", control, message_type, id, size);
        }
        else
        {
            printf("ERROR: DID NOT RECEIVE A SETAPI message: control = 0x%02X, id = 0x%02X\n", control, id);
            cleanup();
            return 1;
        }

        // Read in the API data
        i_result = recv(tcp_socket, (char*) tcp_recv_buf, size, 0);
        if ( i_result > 0 )
        {

            // code for viewing the api data if desired
            // int i;
            // for(i=0; i<payload_size; i++)
            // {
            //     printf("%c", tcp_recv_buf[i]);
            // }
            // printf("\n");

        }
        else if ( i_result == 0 )
        {
            printf("Connection closed while reading the data from the setAPI message\n");
            cleanup();
            return 1;
        }
        else
        {
            printf("SetAPI message data recv failed with error: %d\n", GET_SOCK_ERRNO);
            printf("recv failed with error: %d\n", GET_SOCK_ERRNO);
            cleanup();
            return 1;
        }

        // send empty setAPI message to PinPoint, as per the protocol
        i_result = send( tcp_socket, (char*) send_set_api, 6, 0 );
        if (i_result == SOCKET_ERROR) {
            printf("send setAPI failed with error: %d\n", GET_SOCK_ERRNO);
            cleanup();
            return 1;
        }
    }
    else if ( i_result == 0 )
    {
        printf("Connection closed during setAPI call\n");
        cleanup();
        return 1;
    }
    else
    {
        printf("SetAPI message recv failed with error: %d\n", GET_SOCK_ERRNO);
        cleanup();
        return 1;
    }

    //***********************************
    // 3) Setup UDP port communication with the PinPoint
    //***********************************

    //***********************************
    // Create a UDP port/socket that the pinpoint can connect to
    //***********************************
    {
        // UDP Port on the local machine that the PinPoint will send data (such as keep alive pings) to
        uint16_t local_udp_port = 0;
        if (create_udp_socket(&udp_socket, &udp_server, local_udp_port) == FALSE)
        {
            printf("ERROR creating UDP socket\n");
            cleanup();
            return 1;
        }
    }

    //***********************************
    // The PinPoint sends a GetUdpPort message to us, so it knows which port to send
    // the UDP messages on (such as the UDP port ping message)
    //***********************************
    i_result = recv(tcp_socket, (char*) tcp_recv_buf, 2, 0);
    if ( i_result == 2 )
    {
        uint8_t control = tcp_recv_buf[0];
        uint8_t id = tcp_recv_buf[1];

        if (control == 0x90 && id == 0x05)
        {
            printf("Received GetUdpPort request\n");
            {
                //send the return message to PinPoint, letting it know which UDP port is open on this computer
                uint8_t send_udp_response[5] = {0x81, 0x05, 0x02, 0, 0};
                uint16_t local_udp_port = htons(udp_server.sin_port);
                send_udp_response[3] = ((uint8_t*)(&local_udp_port))[0];
                send_udp_response[4] = ((uint8_t*)(&local_udp_port))[1];

                // Send the response to the PinPoint letting it know what UDP port to send messages to
                i_result = send( tcp_socket, (char*) send_udp_response, sizeof(send_udp_response), 0 );
                if (i_result == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", GET_SOCK_ERRNO);
                    cleanup();
                    return 1;
                }
                else if(i_result == 0)
                {
                    printf("TCP connection closed");
                    cleanup();
                    return 1;
                }
                printf("Sent UDP port response to PinPoint. Local UDP port = %u\n", local_udp_port);
            }
        }
        else
        {
            printf("ERROR: GetUdpPort message should have been received\n");
            return 1;
        }
    }
    else if ( i_result == 0 )
        printf("Connection closed while expecting getudpport request message\n");
    else
        printf("GetUDP port expected message recv failed with error: %d\n", GET_SOCK_ERRNO);

    //***********************************
    //Send getUdpPort message to PinPoint
    //  This message queries the PinPoint to request a response that tells us which UDP port
    //  to connect to on the device
    //***********************************
    {
        uint8_t send_get_udp_port[2] = {0x90, 0x05};
        i_result = send( tcp_socket, (char*) send_get_udp_port, 2, 0 );
        if (i_result == SOCKET_ERROR) {
            printf("send failed with error: %d\n", GET_SOCK_ERRNO);
            cleanup();
            return 1;
        }
        else
        {
            printf("Sent getUdpPort message to pinpoint\n");
        }
    }

    //***********************************
    // Handle the getUDPPort response, which will give us the UDP port
    //   that the PinPoint has opened for communication
    //***********************************
    i_result = recv(tcp_socket, (char*) tcp_recv_buf, 2, 0);
    if ( i_result == 2 )
    {
        uint8_t control = tcp_recv_buf[0];
        uint8_t id = tcp_recv_buf[1];
        if (control == 0x81 && id == 0x05) //verify it's the right message
        {
            uint16_t pinpoint_udp_port;
            uint8_t size_length = (0x03&control);
            uint32_t size;

            printf("Got getUdpPort response\n");
            //read size...
            size = get_message_size(size_length, tcp_socket);

            // Get the port number & unpack it to a uint16
            i_result = recv(tcp_socket, (char*) tcp_recv_buf, size, 0);
            pinpoint_udp_port = UNPACK_UINT16(tcp_recv_buf);
            printf("PinPoint is using UDP port %u\n", pinpoint_udp_port);
        }
    }
    else if ( i_result == 0 )
        printf("Connection closed\n");
    else
        printf("recv failed with error: %d\n", GET_SOCK_ERRNO);

    memset(tcp_recv_buf, 0, sizeof(tcp_recv_buf));
    //***********************************
    // Finished setting up the UDP port
    //***********************************

    //***********************************
    // Finished initial communications procedure
    //***********************************


    //***********************************
    // This program will send a getBoardInfo message to the PinPoint and process the result
    // Next the program will request the status of various status codes of the system
    // It will also subscribe to several signals being output by the PinPoint:
    // 0x06) Global Pose Changd
    // 0x07) Local Pose Changed
    // 0x08) Velocity Changed
    //***********************************

    //***********************************
    // Invoke the getBoardInfo message
    //  To send a message like getBoardInfo (which returns a response), the control byte is 0x10
    //  The getBoardInfo message has an id of 0x04
    //  The response to this message is handled in the main socket reading loop below
    //***********************************
    {
        //control bit 0x10 says to invoke a message with payload size 0
        uint8_t send_string[2] = {0x10, 4};
        i_result = send( tcp_socket, (char*) send_string, 2, 0 );
        if (i_result == SOCKET_ERROR)
        {
            printf("Sending boardinfo message failed\n");
            cleanup();
            return 1;
        }
        printf("Invoked the getBoardInfo message\n");
    }

    //***********************************
    // Receive status communications from the PinPoint
    //
    // NOTE: Invoke the status reporter signal, THEN request several initial status'
    //   The reason to invoke the status reporter first is because status' can change
    //   between when we send the status request message and when we start listening
    //   to the signals being sent by the device (even in the few milliseconds it takes
    //   to setup the signal)
    //
    // After setting up the status reporter signal, we invoke the status request message
    //  for all of the status codes that the localization interface supports
    //***********************************
    {
        // connectSignal message
        uint8_t connect_signal_msg[5] = {0x89, 0x02, 0x02, 0x00, 0x01};
        // invoke message
        // control = 0x11 (protocolflag = 0, message type = 1, sizelength = 1), message id = 2, payload size = 1 byte
        //        (minimum condition, picking 0 will show the status of all codes)
        uint8_t send_status_request[4] = {0x11, 0x02, 0x01, 0x00};

        // connectSignal to signal #0 (status reporter)
        i_result = send( tcp_socket, (char*) connect_signal_msg, sizeof(connect_signal_msg), 0 );
        if (i_result == SOCKET_ERROR) {
            printf("send failed with error: %d\n", GET_SOCK_ERRNO);
            cleanup();
            return 1;
        }
        
        // send getStatusWithCondition(0) message
        i_result = send( tcp_socket, (char*) send_status_request, sizeof(send_status_request), 0 );
        if (i_result == SOCKET_ERROR) {
            printf("send failed with error: %d\n", GET_SOCK_ERRNO);
            cleanup();
            return 1;
        }

    }

    //***********************************
    // connect to rest of the desired signals
    // connetSignal messages have the form [0x89, 0x02, 0x02, signal_id, disconnect/use TCP/use UDP]
    // The control byte may change depending on if you want a return response after sending the message
    // (invoke vs invoke with return)
    //***********************************
    {
        size_t i;
        // Signals to connect to (6 = global pose, 7 = local pose, 8 = velocity, 12 = acceleration)
        const uint8_t signals[] = {0x06, 0x07, 0x08, 0x0C};

        for (i = 0; i<sizeof(signals); i++)
        {
            uint8_t connect_signal_msg[5] = {0x89, 0x02, 0x02, signals[i], 0x01};
            i_result = send( tcp_socket, (char*) connect_signal_msg, sizeof(connect_signal_msg), 0 );
            if (i_result == SOCKET_ERROR) {
                printf("send failed with error: %d\n", GET_SOCK_ERRNO);
                cleanup();
                return 1;
            }
        }
    }
    
    // Setup the sockets to not block
    {
        // Blocking mode for the sockets (0 = block, 1 = no block)
        uint32_t blocking_mode = 1;
        ioctlsocket(tcp_socket, FIONBIO, &blocking_mode);
        ioctlsocket(udp_socket, FIONBIO, &blocking_mode);
    }

    printf("\n\n\n");

    //***********************************
    // Begin socket read loop
    //   first, read in any UDP messages and process them
    //   next, read in any TCP messages and process them
    //***********************************
    while(1)
    {
        int struct_size = sizeof(struct sockaddr_in);

        // Clear received buffers before reading in header bits
        memset(udp_recv_buf, 0, sizeof(udp_recv_buf));
        memset(tcp_recv_buf, 0, sizeof(tcp_recv_buf));

        // Receive UDP message. This program only uses UDP for monitoring and responding to the ping, so we will just read in all 11 bytes at once
        i_result = recvfrom(udp_socket, (char*) udp_recv_buf, 11, 0, (struct sockaddr *) &udp_server, (socklen_t *) &struct_size);
        if (i_result == 11)
        {
            if (handle_udp_message(udp_recv_buf, udp_socket, udp_server) == FALSE)
            {
                printf("Error handling UDP message\n");
            }
        }
        else if (i_result == 0)
        {
            printf("UDP Connection closed\n");
            cleanup();
            return 1;
        }
        else if (i_result < 0)
        {
            //If this is just an error that specifies that the call would have blocked if allowed, ignore it, since we are intentionally using a non-blocking call
            if (GET_SOCK_ERRNO != WOULD_BLOCK_ERROR)
            {
                fprintf(stderr, "Could not receive datagram.\n");
                printf("UDP recv failed with error: %d\n", GET_SOCK_ERRNO);
                cleanup();
                return 1;
            }
        }

        // Receive message from the TCP socket
        i_result = recv(tcp_socket, (char*) tcp_recv_buf, 2, 0);
        if ( i_result == 2 )
        {
            handle_tcp_message(tcp_recv_buf, tcp_socket, global_pose_out, local_pose_out, velocity_out, accel_out, mab_sock_fd, remote_sockaddr);
        }
        else if ( i_result == 0 )
        {
            printf("TCP Connection closed\n");
            cleanup();
            return 1;
        }
        else
        {
            if (GET_SOCK_ERRNO != WOULD_BLOCK_ERROR)
            {
                printf("TCP recv failed with error: %d\n", GET_SOCK_ERRNO);
                cleanup();
                return 1;
            }
        }
    }

    // close csv files
    fclose(local_pose_out);
    fclose(global_pose_out);
    fclose(velocity_out);
    fclose(accel_out);

    // shutdown the connection since no more data will be sent
    i_result = shutdown(tcp_socket, 2);
    if (i_result == SOCKET_ERROR) {
        printf("TCP socket shutdown failed with error: %d\n", GET_SOCK_ERRNO);
        cleanup();
        return 1;
    }

    // shutdown the connection since no more data will be sent
    i_result = shutdown(udp_socket, 2);
    if (i_result == SOCKET_ERROR) {
        printf("UDP receive socket shutdown failed with error: %d\n", GET_SOCK_ERRNO);
        cleanup();
        return 1;
    }

    cleanup();

    return 0;
}
