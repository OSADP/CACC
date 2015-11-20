/** CONFIDENTIAL - do not disclose, distribute, or alter without express permission by TORC Robotics.
All use subject to express agreement only. No implied use or license.

Software copyrights by TORC Robotics, LLC as of initial publish date.

Unless required by applicable law or agreed in writing, use of software is on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef PINPOINT_HANDLERS_H
#define PINPOINT_HANDLERS_H

#include "socket_includes.h"
#include <stdio.h>
#include <stdint.h>

// Function Prototypes
extern BOOL handle_udp_message(uint8_t* const udp_recv_buf, const SOCKET udp_socket, struct sockaddr_in const udp_server);
extern BOOL handle_tcp_message(uint8_t* const tcp_recv_buf, const SOCKET tcp_socket, FILE * const global_pose_out, FILE * const local_pose_out, FILE * const velocity_out, FILE * const accel_out, const int mab_sockfd, const struct sockaddr_in remote_sockaddr);
extern BOOL handle_error(const uint8_t code);
extern BOOL handle_status_message(const uint8_t condition, const uint16_t code);

/**
 * Structure to hold a pinpoint local pose message
 */
struct PinPointLocalPose {
    uint64_t time;       // Microseconds since 1970
    double north;        // Northing (m)
    double east;         // Easting (m)
    double down;         // Downing (m)
    float roll;          // Roll (deg)
    float pitch;         // Pitch (deg)
    float yaw;           // Yaw (deg)
};

/**
 * Structure to hold a pinpoint global pose message
 */
struct PinPointGlobalPose {
    uint64_t time;       // Microseconds since 1970
    double latitude;     // Latitude (deg)
    double longitude;    // Longitude (deg)
    double altitude;     // altitude (m)
    float roll;          // Roll (deg)
    float pitch;         // Pitch (deg)
    float yaw;           // Yaw (deg)
};

/**
 * Structure to hold a pinpoint velocity message
 */
struct PinPointVelocity {
    uint64_t time;       // Microseconds since 1970
    float forward_vel;  // Forward (m/s)
    float right_vel;    // Right (m/s)
    float down_vel;     // Down (m/s)
    float roll_rate;    // Roll (deg/s)
    float pitch_rate;   // Pitch (deg/s)
    float yaw_rate;     // Yaw (deg/s)
};

struct PinPointAcceleration {
    uint64_t time;       // Microseconds since 1970
    float forward;  // Forward (m/s/s)
    float right;    // Right (m/s/s)
    float down;     // Down (m/s/s)
};
#endif
