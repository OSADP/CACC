package com.leidos.v2i.consumers.gps;

import com.leidos.v2i.appcommon.utils.UnpackUtils;
import com.leidos.v2i.logger.ILogger;
import com.leidos.v2i.logger.LoggerManager;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.SocketChannel;

/**
 * NIO utilities to send/receive data on a SocketChannel
 */
public class NioUtils {

    private SocketChannel socketChannel;

    private static ILogger logger = LoggerManager.getLogger(NioUtils.class);

    /**
     * initialize using the socketChannel
     * @param socketChannel
     */
    public NioUtils(SocketChannel socketChannel)  {
        this.socketChannel = socketChannel;
    }


    /**
     * Write a byte array to the channel
     *
     * @param theByte
     * @throws Exception
     */
    public void sendBytes(byte[] theByte) throws Exception  {

        ByteBuffer buf = ByteBuffer.wrap(theByte);

        while (buf.hasRemaining())   {
            socketChannel.write(buf);
            logger.debug(ILogger.TAG_GPS, "Sent " + theByte.length + " bytes: " + buf.toString());
        }
    }

    /**
     * Read size bytes from the channel
     *
     * @param size
     * @return
     * @throws Exception
     */
    public byte[] receiveBytes(int size) throws Exception   {

        byte[] result = new byte[size];

        ByteBuffer inBuffer = ByteBuffer.allocate(size);

        int bytesRead = socketChannel.read(inBuffer);

        if (bytesRead == size)   {
            inBuffer.flip();
            inBuffer.get(result);
            logger.debug(ILogger.TAG_GPS, "Read " + size + " bytes: ");
        }

        return result;
    }

    /**
     * Read a single byte from the channel
     *
     * @return
     * @throws Exception
     */
    public byte receiveByte() throws Exception  {
        byte result = 0x00;

        ByteBuffer inBuffer = ByteBuffer.allocate(1);

        int bytesRead = socketChannel.read(inBuffer);

        if (bytesRead == 1)   {
            inBuffer.flip();
            result = inBuffer.get();
            logger.debug(ILogger.TAG_GPS, "Read 1 byte: " + String.format("0x%02x", result));
        }

        return result;

    }

    /**
     * Return the payload size.
     *
     * sizeLength = 0, 1, 2....message size = sizeLength
     * sizeLength = 3...message size = 4
     *
     * @param sizeLength
     * @return
     * @throws Exception
     */
    public long getMessageSize(byte sizeLength) throws Exception   {

        UnpackUtils utils = UnpackUtils.getInstance();

        int size = utils.unpackU8(sizeLength);
        long longSize = 0L;
        byte[] result = null;

        switch (size)   {

            case 0:
                longSize = 0L;
                break;

            case 1:
                result = receiveBytes((int) size);
                longSize = utils.unpackU8(result[0]);
                break;

            case 2:
                result = receiveBytes((int) size);
                longSize = utils.unpackU16(result);
                break;

            case 3:
                result = receiveBytes((int) size + 1);
                longSize = utils.unpackU32(result);
                break;

            default:
                logger.error(ILogger.TAG_GPS, "Error: Invalid packet size requested: " + size);
        }

        return longSize;

    }

    private void debugByteArray(byte[] bytes)   {
        logger.debug(ILogger.TAG_GPS, "********* bytes ***********");
        for (int i=0; i<bytes.length; i++)    {
            System.out.println("     [" + i + "] : " + bytes[i]);
        }
        logger.debug(ILogger.TAG_GPS, "***************************");
    }

    /**
     * Encode lat/long into a 4 byte int
     *
     * Intended to simulate what the Pinpoint sends across the line for lat/long
     *
     * @param latLong      The double Latitude or Longitude
     * @param globalPose   A byte array containing the 4 bytes of value
     * @param startIndex   The index into the byte array to copy the 4 bytes
     */
    public void encodeLatLong(double latLong, byte[] globalPose, int startIndex)   {

        double result = ( (latLong * Math.pow(2.0, 31)) / 180.0);

        int resultInt = (int) result;

        byte[] intArray = new byte[4];
        ByteBuffer.wrap(intArray).order(ByteOrder.LITTLE_ENDIAN).asIntBuffer().put(resultInt);

        globalPose[startIndex] = intArray[0];
        globalPose[startIndex+1] = intArray[1];
        globalPose[startIndex+2] = intArray[2];
        globalPose[startIndex+3] = intArray[3];
    }


    /**
     * Decode lat/long into from a 4 byte int
     *
     * Our GpsConsumer decodes this value from the getGlobalPose response
     *
     * @param globalPose   A byte array containing the 4 bytes of value
     * @param startIndex   The index into the byte array to obtain the value
     */
    public double decodeLatLong(byte[] globalPose, int startIndex)    {

        byte[] latLongArray = new byte[4];
        latLongArray[0] = globalPose[startIndex];
        latLongArray[1] = globalPose[startIndex+1];
        latLongArray[2] = globalPose[startIndex+2];
        latLongArray[3] = globalPose[startIndex+3];

        int latitude = ByteBuffer.wrap(latLongArray).order(ByteOrder.LITTLE_ENDIAN).getInt();

        double result = 180.0 / Math.pow(2.0, 31) * (double) latitude;

        return result;

    }

    /**
     * Decode velocity from 3 byte array
     *
     * The GPS can decode this value from the getVelocityState response
     *
     * @param velocityState   A byte array containing the 3 bytes of value
     * @param startIndex      The index into the byte array to obtain the value
     * @return
     */
    public double decodeVelocity(byte[] velocityState, int startIndex)    {

        byte[] velocityArray = new byte[3];
        velocityArray[0] = velocityState[startIndex];
        velocityArray[1] = velocityState[startIndex+1];
        velocityArray[2] = velocityState[startIndex+2];

        int intVelocity = UnpackUtils.getInstance().unpack24(velocityArray);

        //  C code => velocity.forward_vel = 1.0f/1000.0f * (float)UNPACK_INT24( (tcp_recv_buf + 0x08) );
        // data provided in mm/s...converting to m/s
        double velocity = 1.0 / 1000.0 * intVelocity;

        return velocity;

    }

}
