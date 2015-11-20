package com.leidos.v2i.consumers.gps;

// See docs 8.2.6 scheduleMessage

import java.util.Arrays;

/**
 * GpsScheduleMessage
 *
 * A request to have the pinpoint provide output for the various method calls on a scheduled basis.
 *
 */
public class GpsScheduleMessage {

    public static final byte ID_GET_GLOBAL_POSE = 0x07;
    public static final byte ID_GET_FILTER_ACCURACY = 0x06;
    public static final byte ID_GET_LOCAL_POSE = 0x09;
    public static final byte ID_GET_VELOCITY_STATE = 0x0B;
    public static final byte ID_GET_BODY_ACCELERATION = 0x12;

    // the list of messages/method ids to send with a scheduleMessage
    //   we are using this value for both methodId and scheduleId
    public static final byte[] messages = { ID_GET_FILTER_ACCURACY, ID_GET_GLOBAL_POSE, ID_GET_LOCAL_POSE,
                                   ID_GET_VELOCITY_STATE, ID_GET_BODY_ACCELERATION };

    private static final int INDEX_SCHEDULE_ID = 3;
    private static final int INDEX_METHOD_ID = 12;
    private static final int INDEX_MS = 4;
    private static final int INDEX_OFFSET = 8;

    // client packet sent to server to request global pose, used as template to modify method id.  We will use
    //  this same value for schedule id to ensure unique
    private final static byte[] GPS_SCHEDULE_MESSAGE = {
            (byte) 0x91,        // invoke protocol message with return, one-bye size field
            0x03,               // schedule message message id
            0x0F,               // payload length - 15 bytes
            0x01,               // giving it a schedule id
            (byte) 0x32,        // 50 ms
            0x00,
            0x00,
            0x00,
            0x00,               // schedule offset, none???
            0x00,
            0x00,
            0x00,
            0x07,               // getGlobalPose method ID
            0x00,               // method parameter size, none????
            0x00,
            0x00,
            0x00,
            0x01                // get data as TCP, 0 = UDP
            // parameters, if they exist
    };

    private byte[] message;

    GpsScheduleMessage(byte methodId, byte ms, byte offset)   {
        this.message = Arrays.copyOf(GPS_SCHEDULE_MESSAGE, GPS_SCHEDULE_MESSAGE.length);
        message[INDEX_METHOD_ID] = methodId;
        message[INDEX_SCHEDULE_ID] = methodId;
        message[INDEX_MS] = ms;
        message[INDEX_OFFSET] = offset;
    }

    public int getMethodId()   {
        return (int) message[INDEX_METHOD_ID];
    }

    public byte[] getBytes()   {
        return message;
    }
}
