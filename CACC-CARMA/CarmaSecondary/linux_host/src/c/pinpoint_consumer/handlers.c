/** CONFIDENTIAL - do not disclose, distribute, or alter without express permission by TORC Robotics.
All use subject to express agreement only. No implied use or license.

Software copyrights by TORC Robotics, LLC as of initial publish date.

Unless required by applicable law or agreed in writing, use of software is on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/

// Variable for determining whether or not to display the pose and velocity signals
#define DISPLAY_SIGNALS 0

#include "handlers.h"

#include "socket_includes.h"
#include "unpack_macros.h"
#include "pinpoint_utility_functions.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


/**
 * Handles a UDP message. In this program, the only UDP messages will be pings
 * @param  udp_recv_buf buffer that the 2 byte message header contains
 * @param  udp_socket   UDP socket that the message came in on, and should continue reading from/sending
 *                      to from for any more data
 * @param  udp_server   socket information about this socket, such as port and ip address
 * @return              TRUE if successfully read/parsed/responded to a UDP ping
 */
BOOL handle_udp_message(uint8_t * const udp_recv_buf, const SOCKET udp_socket, struct sockaddr_in const udp_server)
{
    // Successfully received a message from UDP
    uint8_t control = udp_recv_buf[0];
    uint8_t id = udp_recv_buf[1];

    if (control == 0x91 && id == 0x06)
    {
        int i_result;

        printf("Got a ping from pinpoint\n");
        udp_recv_buf[0] = 0x81;
        // Send data back
        i_result = sendto(udp_socket, (char*) udp_recv_buf, 11, 0, (const struct sockaddr*) &udp_server, sizeof(udp_server));
        if (i_result == SOCKET_ERROR )
        {
            printf("ERROR: Could not send ping response\n");
            return FALSE;
        }
        else if(i_result != 11)
        {
            printf("UDP Ping error: did not send 11 byte response, instead sent %d\n", i_result);
            return FALSE;
        }
    }
    return TRUE;
}

/**
 * Handles a UDP message. In this program, it will handle signals and message returns
 * @param  tcp_recv_buf    buffer that contains the 2 byte message header
 * @param  tcp_socket      TCP socket that the message came in on, and should continue reading from/sending
 *                         to from for any more data
 * @param  global_pose_out Global Pose csv FILE pointer
 * @param  local_pose_out  Local Pose csv FILE pointer
 * @param  velocity_out    Velocity csv FILE pointer
 * @return                 TRUE if no error occurred, or FALSE if an error occurred
 */
BOOL handle_tcp_message(uint8_t * const tcp_recv_buf, const SOCKET tcp_socket, FILE * const global_pose_out, FILE * const local_pose_out, FILE * const velocity_out, FILE * const accel_out, const int mab_sockfd, const struct sockaddr_in remote_sockaddr)
{
    int i_result;

    uint8_t control       = tcp_recv_buf[0];
    uint8_t id            = tcp_recv_buf[1];
    uint8_t protocol_flag = (0x80&control)>>0x07;
    uint8_t message_type  = (0x78&control)>>0x03;
    uint8_t size_length   = (0x03&control);

    uint32_t size = get_message_size(size_length, tcp_socket);

    if (protocol_flag == 0)
    {
        switch(message_type)
        {
        case 0: // method return
            switch (id)
            {
            // message id 1 is the return from the getStatus message
            case 1:
                {
                    i_result = recv(tcp_socket, (char *) tcp_recv_buf, size, 0);
                    if (i_result == (int) size)
                    {
                        uint8_t condition = UNPACK_UINT8(tcp_recv_buf);
                        uint16_t code = UNPACK_UINT16( (tcp_recv_buf + 0x01) );
                        if (handle_status_message(condition, code) == FALSE)
                        {
                            return FALSE;
                        }
                    }
                }
                break; // Done with message id 1
            case 2:
            // message id 2 is the return from the getStatusWithCondition message
                {
                    uint16_t num_statuses;
                    int i;
                    //receive the message
                    i_result = recv(tcp_socket, (char*) tcp_recv_buf, size, 0);

                    //get num of statuses
                    num_statuses = UNPACK_UINT16(tcp_recv_buf);
                    printf("\nGetting Statuses - Number of statuses: %u\n", num_statuses);
                    for (i = 0; i < num_statuses; i++)
                    {
                        uint8_t status_condition = 0;
                        uint16_t status_code = 0;
                        
                        status_condition = UNPACK_UINT8( (tcp_recv_buf + 2 + i * 3) );
                        status_code = UNPACK_UINT16( (tcp_recv_buf + 3 + i * 3) );
                        
                        handle_status_message(status_condition, status_code);
                    }
                    printf("\n");
                }
                break;
            // message id 4 is the return from the getBoardInfo message we requested early
            case 4:
                {
                    uint8_t num_pairs;
                    int i, len=0;
                    //receive the message
                    i_result = recv(tcp_socket, (char*) tcp_recv_buf, size, 0);

                    //get num of pairs
                    num_pairs = UNPACK_UINT8(tcp_recv_buf);
                    printf("\nGetting Board Info - Number of key/value pairs: %d\n", num_pairs);
                    for (i = 0; i < num_pairs; i++)
                    {
                        uint8_t key_size;
                        char ascii_key[256];
                        uint8_t value_size;
                        char ascii_value[256];
                        int j = 0;

                        memset(ascii_key, 0, sizeof(ascii_key));
                        memset(ascii_value, 0, sizeof(ascii_key));

                        key_size = UNPACK_UINT8( (tcp_recv_buf + 1 + 2*i + len) );
                        // read in the key name
                        for (j = 0; j < key_size; j++)
                        {
                            ascii_key[j] = UNPACK_UINT8( (tcp_recv_buf + 2 + 2*i + len) );
                            len = len+1;
                        }

                        value_size  = UNPACK_UINT8( (tcp_recv_buf + 2 + 2*i + len) );
                        // read in the value name
                        for (j = 0; j < value_size; j++)
                        {
                            ascii_value[j] = UNPACK_UINT8( (tcp_recv_buf + 3 + 2*i + len) );
                            len = len+1;
                        }
                        printf("%s:%s\n", ascii_key, ascii_value);
                    }
                    printf("\n");
                }
                break; // done handling message id 4
            default: // add more message handling operations here
                printf("Message return, unknown message id or unimplemented handling for id: %u\n", id);
                i_result = recv(tcp_socket, (char*) tcp_recv_buf, size, 0);
                break;
            }
            break;
        case 3: //signal emitted
            switch (id)
            {
            case 0: // Status Changed signal emitted
                i_result = recv(tcp_socket, (char*) tcp_recv_buf, size, 0);
                if (i_result == (int) size)
                {
                    uint8_t condition = UNPACK_UINT8(tcp_recv_buf);
                    uint16_t code = UNPACK_UINT16( (tcp_recv_buf + 0x01) );
                    if (handle_status_message(condition, code) == FALSE)
                    {
                        return FALSE;
                    }
                }
                //done with case 0
                break;
            case 6: // global pose signal emitted
                {
                    struct PinPointGlobalPose pose;
					
					tcp_recv_buf[0] = id;
					tcp_recv_buf[1] = 0;
					tcp_recv_buf[2] = (char)(size & 0xFF);
					tcp_recv_buf[3] = (char)((size >> 8) & 0xFF);
					
					char* data_recv_buf = &tcp_recv_buf[4];
                    i_result = recv(tcp_socket, (char*) data_recv_buf, size, 0);
                    if (i_result == (int) size)
                    {
						// Forward onto the MicroAutobox
						sendto(mab_sockfd, tcp_recv_buf, size+4, 0, (struct sockaddr *) &remote_sockaddr, sizeof(struct sockaddr_in));
						
                        // string representation of the time
                        char time_str_buffer[30];

                        //successfully read in all the data from the signal
                        //parse the data
                        pose.time      = UNPACK_UINT64(data_recv_buf);
                        pose.latitude  = 180.0 / pow(2.0,31) * (double)UNPACK_INT32( (data_recv_buf + 0x08) );
                        pose.longitude = 180.0 / pow(2.0,31) * (double)UNPACK_INT32( (data_recv_buf + 0x0C) );
                        pose.altitude  = 1.0/1000.0 * (double)UNPACK_INT32( (data_recv_buf + 0x10) );
                        pose.roll      = 180.0f / powf(2.0f,15) * (float)UNPACK_INT16( (data_recv_buf + 0x14) );
                        pose.pitch     = 180.0f / powf(2.0f,15) * (float)UNPACK_INT16( (data_recv_buf + 0x16) );
                        pose.yaw       = 180.0f / powf(2.0f,15) * (float)UNPACK_INT16( (data_recv_buf + 0x18) );

                        //convert time to standard format ie. YEAR-MON-DAYT HOUR:MIN:SEC.MILLISECZ
                        {
                            time_t unix_time;
                            struct tm * ptm;
                            uint32_t time_milliseconds;
                            unix_time = (time_t) (1/1000000.0 * pose.time);
                            time_milliseconds = (pose.time % 1000000) / 1000;
                            ptm = gmtime ( &unix_time );
                            sprintf(time_str_buffer, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.%.3dZ", (ptm->tm_year+1900), (ptm->tm_mon + 1), ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, time_milliseconds);
                        }

#if DISPLAY_SIGNALS
                        printf("\nGlobal Pose\n");
                        printf("Time        =   %s\n", time_str_buffer);
                        printf("latitude    = %12.7f deg\n",  pose.latitude);
                        printf("longitude   = %12.7f deg\n",  pose.longitude);
                        printf("altitude    = %12.3f m\n",  pose.altitude);
                        printf("roll        =      %7.2f deg\n",   (double) pose.roll);
                        printf("pitch       =      %7.2f deg\n",   (double) pose.pitch);
                        printf("yaw         =      %7.2f deg\n",   (double) pose.yaw);
#endif

                        //write to csv file
                        i_result = fprintf(global_pose_out, "%s, %12.7f, %12.7f, %12.3f, %7.2f, %7.2f, %7.2f\n", time_str_buffer, pose.latitude, pose.longitude, pose.altitude, (double) pose.roll, (double) pose.pitch, (double) pose.yaw);
                        if (i_result < 0)
                        {
                            perror("writing global pose to file failed\n");
                            return FALSE;
                        }
                    }
                    else if ( i_result == 0 )
                    {
                        printf("Connection closed during global pose signal\n");
                        return FALSE;
                    }
                    else
                    {
                        printf("recv failed with error during global pose signal: %d\n", GET_SOCK_ERRNO);
                        return FALSE;
                    }
                }
                //done with case 6
                break;
            case 7: // Local Pose Signal emitted
                {
                    struct PinPointLocalPose pose;
					tcp_recv_buf[0] = id;
					tcp_recv_buf[1] = 0;
					tcp_recv_buf[2] = (char)(size & 0xFF);
					tcp_recv_buf[3] = (char)((size >> 8) & 0xFF);
					
					char* data_recv_buf = &tcp_recv_buf[4];
                    //printf("Received a local pose signal: %d\n", id);
                    i_result = recv(tcp_socket, data_recv_buf, size, 0);
                    if (i_result == (int) size)
                    {
						// Forward onto the MicroAutobox
						sendto(mab_sockfd, tcp_recv_buf, size, 0, (struct sockaddr *) &remote_sockaddr, sizeof(struct sockaddr_in));
						
                        // string representation of the time
                        char time_str_buffer[30];

                        //successfully read in all the data from the signal
                        //parse the data
                        pose.time  = UNPACK_UINT64(data_recv_buf);
                        pose.north = 1.0/1000.0 * (double)UNPACK_INT32( (data_recv_buf + 0x08) );
                        pose.east  = 1.0/1000.0 * (double)UNPACK_INT32( (data_recv_buf + 0x0C) );
                        pose.down  = 1.0/1000.0 * (double)UNPACK_INT32( (data_recv_buf + 0x10) );
                        pose.roll  = 180.0f / powf(2.0f,15) * (float)UNPACK_INT16( (data_recv_buf + 0x14) );
                        pose.pitch = 180.0f / powf(2.0f,15) * (float)UNPACK_INT16( (data_recv_buf + 0x16) );
                        pose.yaw   = 180.0f / powf(2.0f,15) * (float)UNPACK_INT16( (data_recv_buf + 0x18) );

                        //convert time to standard format ie. YEAR-MON-DAYT HOUR:MIN:SEC.MILLISECZ
                        {
                            time_t unix_time;
                            struct tm * ptm;
                            uint32_t time_milliseconds;
                            //convert time to standard format ie. YEAR-MON-DAYT HOUR:MIN:SEC.MILLISECZ
                            unix_time = (time_t) (1/1000000.0 * pose.time);
                            time_milliseconds = (pose.time % 1000000) / 1000;
                            ptm = gmtime ( &unix_time );
                            sprintf(time_str_buffer, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.%.3dZ", (ptm->tm_year+1900), (ptm->tm_mon + 1), ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, time_milliseconds);
                        }
                        
#if DISPLAY_SIGNALS
                        printf("\nLocal Pose\n");
                        printf("Time        =   %s\n", time_str_buffer);
                        printf("north       = %12.3f m\n", pose.north);
                        printf("east        = %12.3f m\n", pose.east);
                        printf("down        = %12.3f m\n", pose.down);
                        printf("roll        =      %7.2f deg\n", (double) pose.roll);
                        printf("pitch       =      %7.2f deg\n", (double) pose.pitch);
                        printf("yaw         =      %7.2f deg\n", (double) pose.yaw);
#endif

                        i_result = fprintf(local_pose_out, "%s, %12.3f, %12.3f, %12.3f, %7.2f, %7.2f, %7.2f\n", time_str_buffer, pose.north, pose.east, pose.down, (double) pose.roll, (double) pose.pitch, (double) pose.yaw);
                        if (i_result < 0)
                        {
                            perror("writing local pose to file failed\n");
                            return FALSE;
                        }
                    }
                    else if ( i_result == 0 )
                    {
                        printf("Connection closed during local pose signal\n");
                        return FALSE;
                    }
                    else
                    {
                        printf("recv failed with error during local pose signal: %d\n", GET_SOCK_ERRNO);
                        return FALSE;
                    }
                }
                //done with case 7
                break;
            case 8: // New velocity signal emitted
                {
                    struct PinPointVelocity velocity;
                    //printf("Received Local velocity signal: \n", id);
					tcp_recv_buf[0] = id;
					tcp_recv_buf[1] = 0;
					tcp_recv_buf[2] = (char)(size & 0xFF);
					tcp_recv_buf[3] = (char)((size >> 8) & 0xFF);
					char* data_recv_buf = &tcp_recv_buf[4];
                    i_result = recv(tcp_socket, data_recv_buf, size, 0);
                    if (i_result == (int) size)
                    {
						// Forward onto the MicroAutobox
						sendto(mab_sockfd, tcp_recv_buf, size, 0, (struct sockaddr *) &remote_sockaddr, sizeof(struct sockaddr_in));
						
                        // string representation of the time
                        char time_str_buffer[30];

                        //parse the data
                        velocity.time        = UNPACK_UINT64(data_recv_buf);
                        velocity.forward_vel = 1.0f/1000.0f * (float)UNPACK_INT24( (data_recv_buf + 0x08) );
                        velocity.right_vel   = 1.0f/1000.0f * (float)UNPACK_INT24( (data_recv_buf + 0x0B) );
                        velocity.down_vel    = 1.0f/1000.0f * (float)UNPACK_INT24( (data_recv_buf + 0x0E) );
                        velocity.roll_rate   = 1.0f/1000.0f * (float)UNPACK_INT24( (data_recv_buf + 0x11) );
                        velocity.pitch_rate  = 1.0f/1000.0f * (float)UNPACK_INT24( (data_recv_buf + 0x14) );
                        velocity.yaw_rate    = 1.0f/1000.0f * (float)UNPACK_INT24( (data_recv_buf + 0x17) );
                        
                        //convert time to standard format ie. YEAR-MON-DAYT HOUR:MIN:SEC.MILLISECZ
                        {
                            time_t unix_time;
                            struct tm * ptm;
                            uint32_t time_milliseconds;
                            unix_time = (time_t) (1/1000000.0 * velocity.time);
                            time_milliseconds = (velocity.time % 1000000) / 1000;
                            ptm = gmtime ( &unix_time );
                            sprintf(time_str_buffer, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.%.3dZ", (ptm->tm_year+1900), (ptm->tm_mon + 1), ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, time_milliseconds);
                        }

                        // Print to screen
#if DISPLAY_SIGNALS
                        printf("\nVelocity State\n");
                        printf("Time        =   %s\n", time_str_buffer);
                        printf("forward_vel =      %7.3f m/s\n", (double)velocity.forward_vel);
                        printf("right_vel   =      %7.3f m/s\n", (double)velocity.right_vel);
                        printf("down_vel    =      %7.3f m/s\n", (double)velocity.down_vel);
                        printf("roll_rate   =      %7.3f rad/s\n", (double)velocity.roll_rate);
                        printf("pitch_rate  =      %7.3f rad/s\n", (double)velocity.pitch_rate);
                        printf("yaw_rate    =      %7.3f rad/s\n", (double)velocity.yaw_rate);
#endif

                        //write to csv file
                        i_result = fprintf(velocity_out, "%s, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f\n", time_str_buffer, (double)velocity.forward_vel, (double)velocity.right_vel, (double)velocity.down_vel, (double)velocity.roll_rate, (double)velocity.pitch_rate, (double)velocity.yaw_rate);
                        if (i_result < 0)
                        {
                            perror("writing velocity to file failed\n");
                            return FALSE;
                        }

                    }
                    else if ( i_result == 0 )
                    {
                        printf("Connection closed during velocity signal\n");
                        return FALSE;
                    }
                    else
                    {
                        if (GET_SOCK_ERRNO != WOULD_BLOCK_ERROR)
                        {
                            printf("recv failed with error during velocity signal: %d\n", GET_SOCK_ERRNO);
                            return FALSE;
                        }
                    }
                }
                //done with case 8
                break;
            case 12: // Body Acceleration Signal emitted
                {
                    struct PinPointAcceleration accel;
                    //printf("Received a body acceleration signal: %d\n", id);
					tcp_recv_buf[0] = id;
					tcp_recv_buf[1] = 0;
					tcp_recv_buf[2] = (char)(size & 0xFF);
					tcp_recv_buf[3] = (char)((size >> 8) & 0xFF);
					data_recv_buf = &tcp_recv_buf[4];
                    i_result = recv(tcp_socket, data_recv_buf, size, 0);
                    if (i_result == (int) size)
                    {
						// Forward onto the MicroAutobox
						sendto(mab_sockfd, tcp_recv_buf, size, 0, (struct sockaddr *) &remote_sockaddr, sizeof(struct sockaddr_in));
						
                        // string representation of the time
                        char time_str_buffer[30];

                        //successfully read in all the data from the signal
                        //parse the data
                        accel.time  = UNPACK_UINT64(data_recv_buf);
                        accel.forward = 1.0/1000.0 * (double)UNPACK_INT16( (data_recv_buf + 0x08) );
                        accel.right  = 1.0/1000.0 * (double)UNPACK_INT16( (data_recv_buf + 0x0A) );
                        accel.down  = 1.0/1000.0 * (double)UNPACK_INT16( (data_recv_buf + 0x0C) );

                        //convert time to standard format ie. YEAR-MON-DAYT HOUR:MIN:SEC.MILLISECZ
                        {
                            time_t unix_time;
                            struct tm * ptm;
                            uint32_t time_milliseconds;
                            //convert time to standard format ie. YEAR-MON-DAYT HOUR:MIN:SEC.MILLISECZ
                            unix_time = (time_t) (1/1000000.0 * accel.time);
                            time_milliseconds = (accel.time % 1000000) / 1000;
                            ptm = gmtime ( &unix_time );
                            sprintf(time_str_buffer, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.%.3dZ", (ptm->tm_year+1900), (ptm->tm_mon + 1), ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, time_milliseconds);
                        }
                        
#if DISPLAY_SIGNALS
                        printf("\nBody Acceleration\n");
                        printf("Time        =   %s\n", time_str_buffer);
                        printf("forward     = %12.3f m\n", (double)accel.forward);
                        printf("right       = %12.3f m\n", (double)accel.right);
                        printf("down        = %12.3f m\n", (double)accel.down);
#endif

                        i_result = fprintf(accel_out, "%s, %7.3f, %7.3f, %7.3f\n", time_str_buffer, (double)accel.forward, (double)accel.right, (double)accel.down);
                        if (i_result < 0)
                        {
                            perror("writing body acceleration to file failed\n");
                            return FALSE;
                        }
                    }
                    else if ( i_result == 0 )
                    {
                        printf("Connection closed during body acceleration signal\n");
                        return FALSE;
                    }
                    else
                    {
                        printf("recv failed with error during body acceleration signal: %d\n", GET_SOCK_ERRNO);
                        return FALSE;
                    }
                }
                //done with case 12
                break;
            default:
                //handle return value
                printf("Signal handler (0x%02X) not yet implemented\n", id);
                i_result = recv(tcp_socket, (char*) tcp_recv_buf, size, 0);
                return FALSE;
            }
            break;
        default:
            printf("Unknown message type received (%02X)\n", message_type);
            i_result = recv(tcp_socket, (char*) tcp_recv_buf, size, 0);
            return FALSE;
        }
    }
    else
    {
        printf("Received a protocol message on TCP\n");
        switch(message_type)
        {
            //handle error message
        case 5:
            if (handle_error(id) == FALSE)
            {
                return FALSE;
            }
            break;
        default:
            printf("Protocol message handler for type (0x%02X) not yet implemented. control = (0x%02X)\n", message_type, control);
            i_result = recv(tcp_socket, (char*) tcp_recv_buf, size, 0);
            return FALSE;
        }
    }
    return TRUE;
}

/**
 * Handles a protocol error message
 * @param  code error code received
 * @return      TRUE if code is valid, FALSE otherwise
 */
BOOL handle_error(const uint8_t code)
{
    //error code received...
    switch(code)
    {
    case 0:
        printf("ERROR received: \"no error\"\n");
        break;
    case 1:
        printf("ERROR received: \"unspecified error\"\n");
        break;
    case 2:
        printf("ERROR received: \"unsupported message type\"\n");
        break;
    case 3:
        printf("ERROR received: \"invalid serialization\"\n");
        break;
    case 4:
        printf("ERROR received: \"invalid method id\"\n");
        break;
    case 5:
        printf("ERROR received: \"invalid signal id\"\n");
        break;
    case 6:
        printf("ERROR received: \"invalid schedule id\"\n");
        break;
    default:
        printf("ERROR: Unknown Error: %d\n", code);
        return FALSE;
    }
    return TRUE;
}

/**
 * Handles a status signal message
 * @param  code error code received
 * @return      TRUE if code is valid, FALSE otherwise
 */
BOOL handle_status_message(const uint8_t condition, const uint16_t code)
{
    printf("Code = %u, Condition = %u\n", code, condition);
    switch(code)
    {
    case 1:
        printf("Aligning has status: %s\n", get_status_condition(condition));
        break;
    case 2:
        printf("NoImuData has status: %s\n", get_status_condition(condition));
        break;
    case 3:
        printf("NoGpsUpdates has status: %s\n", get_status_condition(condition));
        break;
    case 4:
        printf("NoLeftWssUpdates has status: %s\n", get_status_condition(condition));
        break;
    case 5:
        printf("NoRightWssUpdates has status: %s\n", get_status_condition(condition));
        break;
    case 6:
        printf("BadGpsPosAgreement has status: %s\n", get_status_condition(condition));
        break;
    case 7:
        printf("BadGpsVelAgreement has status: %s\n", get_status_condition(condition));
        break;
    case 8:
        printf("BadWssVelAgreement has status: %s\n", get_status_condition(condition));
        break;
    case 9:
        printf("BadGyroBiasEstimate has status: %s\n", get_status_condition(condition));
        break;
    case 10:
        printf("BadAccelBiasEstimate has status: %s\n", get_status_condition(condition));
        break;
    case 11:
        printf("PoseSteadying has status: %s\n", get_status_condition(condition));
        break;
    case 12:
        printf("NoHeadingUpdates has status: %s\n", get_status_condition(condition));
        break;
    case 13:
        printf("BadHeadingAgreement has status: %s\n", get_status_condition(condition));
        break;
    case 14:
        printf("BadMeasurementTime has status: %s\n", get_status_condition(condition));
        break;
    case 15:
        printf("IrregularTimeStep has status: %s\n", get_status_condition(condition));
        break;
    default:
        printf("Unknown code %u, condition = %u\n", code, condition);
        return FALSE;
    }
    return TRUE;
}

