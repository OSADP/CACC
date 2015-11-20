package com.leidos.v2i.command;

import com.leidos.v2i.config.V2IApplicationContext;
import com.leidos.v2i.logger.ILogger;
import com.leidos.v2i.logger.LoggerManager;
import com.leidos.v2i.services.V2IService;
import com.leidos.v2i.ui.UiMessage;
import org.joda.time.DateTime;
import org.joda.time.Duration;

import java.io.IOException;
import java.net.*;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicBoolean;


/**
 * CommandStatusProcessor
 *
 * Listens for incoming status messages on mab.command.port
 * Forwards messages to MAB mab.host, mab.command.port
 *
 *
 */
public class CommandStatusProcessor implements Runnable {

    private static ILogger logger = LoggerManager.getLogger(CommandStatusProcessor.class);

    private AtomicBoolean bShutdown = new AtomicBoolean(false);
    private int maxPacketSize;
    private int udpTimeout;
    private int minUiDuration;

    // MAB configuration for Commands
    private DatagramSocket mabSocket;
    private String mabIp;
    private int mabUdpPort;
    private InetSocketAddress mabAddress;

    private DateTime lastTimeSent;

    public CommandStatusProcessor(){
        this.maxPacketSize = V2IApplicationContext.getInstance().getAppConfig().getIntValue("asd.maxpacketsize");
        this.udpTimeout = V2IApplicationContext.getInstance().getAppConfig().getIntValue("udp.timeout");
        this.lastTimeSent = new DateTime();
        this.minUiDuration = V2IApplicationContext.getInstance().getAppConfig().getIntValue("ui.duration");

        this.mabUdpPort = V2IApplicationContext.getInstance().getAppConfig().getIntValue("mab.command.port");
        this.mabIp = V2IApplicationContext.getInstance().getAppConfig().getProperty("mab.host");
        this.mabAddress = new InetSocketAddress(mabIp, mabUdpPort);
    }

    /**
     * initialize
     */
    public void initialize(){
        logger.info(ILogger.TAG_COMMAND, "Initializing MAB Commmand/Status Interface on port: " + mabUdpPort);

        //establish the UDP socket
        try   {
            mabSocket = new DatagramSocket(mabUdpPort);
            mabSocket.setSoTimeout(udpTimeout);
        }
        catch(Exception e)   {
            logger.error(ILogger.TAG_COMMAND, "Exception initializing mabSocket: " + e.getMessage());
        }

        new Thread(this).start();

    }


    @Override
    public void run() {
        logger.info(ILogger.TAG_COMMAND, "Starting MAB Command/Status thread for port: " + mabUdpPort);

        while (!bShutdown.get())  {
            byte[] buf = new byte[maxPacketSize];
            DatagramPacket p = new DatagramPacket(buf, maxPacketSize);

            try {
                // **** RECEIVE STATUS FROM MAB AND FORWARD TO UI
            	//      data content is:  byte 0 = state (see AppState class), byte 1 = emergency brake indicator (boolean)
            	
                mabSocket.receive(p);

                int bytesRead = p.getLength();

                if (bytesRead == 2)   {

                    byte[] statusData = new byte[bytesRead];

                    statusData = Arrays.copyOf(p.getData(), bytesRead);

                    logger.info(ILogger.TAG_COMMAND, "MAB status received, result of " + bytesRead + " bytes.");
                    logger.debug(ILogger.TAG_COMMAND, "MAB status data: " + javax.xml.bind.DatatypeConverter.printHexBinary(statusData));

                    // send TO UI, but only if we have equaled or exceed min threshold
                    //   if brakeIndicator is not zero, we are sending immediately to UI
                    V2IService service = V2IApplicationContext.getInstance().getService();
                    if (service != null)   {
                        Duration duration = new Duration(lastTimeSent, new DateTime());
                        if ( ( minUiDuration <= duration.getMillis() ) || ( statusData[1] != (byte) 0x00 ) )    {
                            UiMessage uiMessage = new UiMessage(statusData[0], statusData[1]);
                            service.sendUiMessage(uiMessage);
                            lastTimeSent = new DateTime();
                        }
                    }
                }
            } catch (SocketTimeoutException ste)   {
                // expected
            } catch (SocketException e) {
                logger.warn(ILogger.TAG_COMMAND, "read: socket exception ");
            } catch (IOException e) {
                logger.warn(ILogger.TAG_COMMAND, "read: IO exception ");
            }

        }

        closeConnection();
    }

    /**
     * Send UI based caccIntent to MAB
     *
     * @param command
     */
    public void writeCommandToMab(byte[] command)   {

        try   {
            // Send command to MAB
            DatagramPacket sendPacket = new DatagramPacket(command, command.length, mabAddress);

            mabSocket.send(sendPacket);

            logger.debug(ILogger.TAG_COMMAND, this.getClass().getSimpleName() + " sent " + command.length + " bytes to MAB Command: " + mabIp + ":" + mabUdpPort);
        } catch (SocketException e) {
            logger.warn(ILogger.TAG_COMMAND, "read: socket exception ");
        } catch (IOException e) {
            logger.warn(ILogger.TAG_COMMAND, "read: IO exception ");
        }
    }

    /**
     * terminate
     * Set the atomic boolean shutdown flag
     */
    public void terminate() {
        bShutdown.getAndSet(true);
    }


    /**
     * closeConnection
     */
    private void closeConnection(){
        logger.info(ILogger.TAG_COMMAND, "Stopping MAB Command thread for port: " + mabUdpPort);
        try {
            mabSocket.close();
        } catch (Exception e) {
            logger.error(ILogger.TAG_COMMAND, "Error closing MAB Command for ports: " + mabUdpPort + ", " + e.getMessage());
        }
    }

}