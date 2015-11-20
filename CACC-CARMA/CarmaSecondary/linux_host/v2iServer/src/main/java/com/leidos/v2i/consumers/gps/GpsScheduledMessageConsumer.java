package com.leidos.v2i.consumers.gps;

import com.leidos.v2i.IConsumerInitializer;
import com.leidos.v2i.config.V2IApplicationContext;
import com.leidos.v2i.logger.ILogger;
import com.leidos.v2i.logger.LoggerManager;

import java.io.IOException;
import java.net.*;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.nio.channels.SocketChannel;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * GpsScheduleMessageConsumer
 *
 * This consumer submits pinpoint schedule messages and reads responses in a thread loop until shutdown.
 *
 * User: ferenced
 * Date: 8/9/15
 * Time: 3:47 PM
 */
public class GpsScheduledMessageConsumer implements Runnable {

    private static ILogger logger = LoggerManager.getLogger(GpsScheduledMessageConsumer.class);

    // size of the UDP Ping message. we echo the server packet, replacing the first character 0x91 with 0x81
    private final static int SIZE_UDP_PING = 11;

    private AtomicBoolean bShutdown = new AtomicBoolean(false);
    private SocketChannel socketChannel;
    private DatagramChannel datagramChannel;
    private IConsumerInitializer gpsInitializer;
    private String serverIp;
    private int serverTcpPort;
    private int clientUdpPort;

    // MAB configuration for GPS
    private DatagramSocket mabSocket;
    private String mabIp;
    private int mabUdpPort;
    private InetSocketAddress mabAddress;

    /**
     * Constructor
     */
    public GpsScheduledMessageConsumer()   {
        this.serverIp = V2IApplicationContext.getInstance().getAppConfig().getGpsHost();
        this.serverTcpPort = V2IApplicationContext.getInstance().getAppConfig().getGpsPort();
        this.clientUdpPort = V2IApplicationContext.getInstance().getAppConfig().getGpsUdpPort();

        this.mabUdpPort = V2IApplicationContext.getInstance().getAppConfig().getIntValue("mab.gps.port");
        this.mabIp = V2IApplicationContext.getInstance().getAppConfig().getProperty("mab.host");
        this.mabAddress = new InetSocketAddress(mabIp, mabUdpPort);
    }

    @Override
    public void run() {
        logger.info(ILogger.TAG_GPS, "Gps consumer started...");
        try{
            // A run the server as long as the thread is not interrupted.
            while (!bShutdown.get())  {

                ByteBuffer receiveBuffer = ByteBuffer.allocate(1000);       // control, id, size, payload
                int bytesRead = socketChannel.read(receiveBuffer);

                if (bytesRead > 0)   {
                    receiveBuffer.flip();

                    byte[] gpsPacket = new byte[bytesRead];

                    receiveBuffer.get(gpsPacket);
                    receiveBuffer.clear();

                    logger.info(ILogger.TAG_GPS, "Gps Consumer received result of " + bytesRead + " bytes.");
                    logger.debug(ILogger.TAG_GPS, "GPS Consumer Data: " + javax.xml.bind.DatatypeConverter.printHexBinary(gpsPacket));

                    // send to mab, ignore scheduleMessage responses (start with 0x81, 0x03)
                    if (gpsPacket[0] != 0x81 && gpsPacket[1] != 0x03)   {
                        DatagramPacket sendPacket = new DatagramPacket(gpsPacket, gpsPacket.length, mabAddress);

                        mabSocket.send(sendPacket);
                        logger.info(ILogger.TAG_GPS_MAB, "GPS forwarded to GPS MAB " + gpsPacket.length + " bytes on to: " + mabIp + ":" + mabUdpPort);

                    }
                }

                //keep 2-way handshake going with the Pinpoint or it will time out
                Boolean ping = respondToUdpPing();
                
                //TODO: probably needs to be removed to improve performance once the system is operating reliably
                try   {
                    LoggerManager.writeToDisk();
                }
                catch(IOException ioe)   {
                	logger.info(ILogger.TAG_GPS, "Error writing log to disk; continuing: " + ioe.getMessage());
                	System.err.println("Error writing log to disk: " + ioe.getMessage());
                }
                catch(Exception e) {
                	logger.info(ILogger.TAG_GPS, "Error writing log to disk; continuing: " + e.getMessage());
                	System.err.println("Error writing log to disk: " + e.getMessage());
                }
            }

        } catch (IOException ioe){
            logger.error(ILogger.TAG_GPS, " Error GPS Consumer read loop: " + ioe.getMessage());
            ioe.printStackTrace();
        } catch (Exception e){
            logger.error(ILogger.TAG_GPS, " Error GPS Consumer read loop: " + e.getMessage());
            e.printStackTrace();
        } finally{
            closeConnection();
        }

    }

    /**
     * sendScheduleMessages
     * Wrapper method to send a list of GpsScheduleMessage objects to the Pinpoint to register periodic responses
     *
     * @throws Exception
     */
    public void sendScheduleMessages() throws Exception   {

        new Thread(this).start();

        int iFrequency = V2IApplicationContext.getInstance().getAppConfig().getIntValue("gps.frequency");
        byte frequency = (byte) iFrequency;

        // Looks like each scheduled message has to have it's own offset for them all to get sent
        byte offset = 0x00;

        byte[] messageIds = GpsScheduleMessage.messages;

        List<GpsScheduleMessage> messages = new ArrayList<GpsScheduleMessage>();

        // create the list of messages
        for (byte id : messageIds)   {
            GpsScheduleMessage message = new GpsScheduleMessage(id, frequency, offset);
            messages.add(message);
            offset += 5;
        }
        logger.debug(ILogger.TAG_GPS, "--- sendScheduleMessages - ready to loop on each msg");

        for (GpsScheduleMessage message : messages)   {
            sendScheduleMessage(message);
        }
    }

    /**
     * sendScheduleMessage
     * Sends an individual shedule message to the Pinpoint
     *
     * @param message
     * @throws IOException
     */
    public void sendScheduleMessage(GpsScheduleMessage message) throws IOException  {

        ByteBuffer sendBuffer = ByteBuffer.wrap(message.getBytes());

        int bytesWritten = 0;
        try  {
            bytesWritten = socketChannel.write(sendBuffer);
            logger.debug(ILogger.TAG_GPS, "Client sent scheduleMessage method call: " + message.getMethodId());
        }
        catch(IOException ioe)   {
            logger.error(ILogger.TAG_GPS, "Error sending scheduleMessage:", ioe);
        }

        return;

    }



    /**
     * Start the GPS client.  This opens TCP and UDP channels to the Pinpoint device and constructs an
     * Initializer for the Pinpoint Localization server.
     *
     * @return  true if successful initialization
     */
    public boolean initialize()  {

        Boolean bResult = false;

        try   {
            datagramChannel = DatagramChannel.open();
            datagramChannel.configureBlocking(false);
            socketChannel = SocketChannel.open();
            socketChannel.setOption(StandardSocketOptions.TCP_NODELAY, true);
            this.gpsInitializer = new GpsConsumerInitializer(serverIp, serverTcpPort, clientUdpPort, socketChannel, datagramChannel);

            mabSocket = new DatagramSocket(mabUdpPort);

            bResult = true;
        }
        catch(Exception e)   {
            logger.error(ILogger.TAG_GPS, "Error starting GpsConsumer.", e);
        }
        finally   {
            if (!bResult)   {
                closeConnection();
            }
        }

        return bResult.booleanValue();
    }

    public void terminate() {
        bShutdown.getAndSet(true);
    }


    /**
     * Responds to the UDP Ping from the Pinpoint Device
     * Server will wait up to 30 seconds for a response, so we can just do this a part of our read GPS cycle.
     * 11 byte UDP Ping...server sends 0x91, client responds with 0x81 and echos server data
     *
     * NOTE: When we connected to actual device, it was observed that the Pinpoint sent the UDP Ping approx 1 second
     * after receiving our client's ping response. Since we were blocking on the UDP channel, this resulted in only
     * reading GPS data every second as we waited for the Pinpoint ping.  Updated UDP reading to non-blocking.
     *
     * @return  A Boolean indicating the success of the operation
     * @throws java.io.IOException
     */
    public Boolean respondToUdpPing() throws IOException {
        ByteBuffer udpBuffer = ByteBuffer.allocate(SIZE_UDP_PING);
        byte[] clientPingResponse = new byte[SIZE_UDP_PING];

        SocketAddress client = datagramChannel.receive(udpBuffer);

        if (client != null)   {
            udpBuffer.flip();
            udpBuffer.get(clientPingResponse, 0, SIZE_UDP_PING);

            if (clientPingResponse[0] == (byte) 0x91)   {
                logger.debug(ILogger.TAG_GPS, "Client received UDP Ping.");

                clientPingResponse[0] = (byte) 0x81;

                ByteBuffer udpResponse = ByteBuffer.allocate(11);
                udpResponse.put(clientPingResponse);
                udpResponse.flip();
                datagramChannel.send(udpResponse, client);

                logger.debug(ILogger.TAG_GPS, "Client responded to UDP Ping.");

                return new Boolean(true);
            }
            else   {
                logger.error(ILogger.TAG_GPS, "Error receiving UDP Ping. First value is not 0x91.");
                return new Boolean(false);
            }
        }

        return new Boolean(true);

    }

    /**
     * Closes the associated nio resources
     *
     * @throws Exception
     */
    public void closeConnection()   {

        try   {
            if (socketChannel != null)  {
                socketChannel.socket().close();
                socketChannel.close();
            }
            if (datagramChannel != null)  {
                datagramChannel.socket().close();
                datagramChannel.close();
            }

            if (mabSocket != null)   {
                mabSocket.close();
            }
        }
        catch(IOException ioe)   {
            logger.error(ILogger.TAG_GPS, "Error stopping GpsConsumer: ", ioe);
        }

        logger.info(ILogger.TAG_GPS, "Gps Consumer shutdown.");

    }

    /**
     * getInitializer
     * After this consumer is initialized, you can retrieve the gpsInitializer which does the pinpoint handshaking
     *
     * @return IConsumerInitializer
     */
    public IConsumerInitializer getInitializer()   {
        return gpsInitializer;
    }

}
