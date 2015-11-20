package com.leidos.v2i.consumers.dsrc;

import com.leidos.v2i.config.V2IApplicationContext;
import com.leidos.v2i.logger.ILogger;
import com.leidos.v2i.logger.LoggerManager;

import java.io.IOException;
import java.net.*;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.xml.bind.DatatypeConverter;


/**
 * DsrcProcessor
 *
 * Listens for incoming DSRC from MAB on mab.dsrc.inport
 * Forwards and incoming MAB DSRC packets to OBU on asd.host, asd.outport
 */
public class MabDsrcProcessor implements Runnable {

    private static ILogger logger = LoggerManager.getLogger(MabDsrcProcessor.class);

    private AtomicBoolean bShutdown = new AtomicBoolean(false);
    private int maxPacketSize;
    private int udpTimeout;

    // DSRC receiver configuration
    private DatagramSocket obuOutSocket;
    private String obuIp;
    private int obuUdpOutPort;
    private InetSocketAddress obuAddress;

    // MAB configuration for DSRC
    private DatagramSocket mabInSocket;
    private int mabUdpInPort;

    //private DateTime lastTimeSent;

    public MabDsrcProcessor(){
        this.maxPacketSize = V2IApplicationContext.getInstance().getAppConfig().getIntValue("asd.maxpacketsize");
        this.udpTimeout = V2IApplicationContext.getInstance().getAppConfig().getIntValue("udp.timeout");

        this.obuIp = V2IApplicationContext.getInstance().getAppConfig().getProperty("asd.host");
        this.obuUdpOutPort = V2IApplicationContext.getInstance().getAppConfig().getIntValue("asd.outport");
        this.obuAddress = new InetSocketAddress(obuIp, obuUdpOutPort);

        this.mabUdpInPort = V2IApplicationContext.getInstance().getAppConfig().getIntValue("mab.dsrc.inport");

    }

    /**
     * initialize
     */
    public void initialize(DatagramSocket mabSocket){
        logger.info(ILogger.TAG_DSRC_MAB, "Initializing DSRC MAB Receive Interface for port: " + mabUdpInPort);

        //establish the UDP sockets
        try   {
            obuOutSocket = new DatagramSocket(obuUdpOutPort);
            obuOutSocket.setSoTimeout(udpTimeout);

            mabInSocket = mabSocket;
        }
        catch(Exception e)   {
            logger.error(ILogger.TAG_DSRC_MAB, "Exception initializing DSRC sockets: " + e.getMessage());
        }

        new Thread(this).start();
    }


    @Override
    public void run() {
        logger.info(ILogger.TAG_DSRC_MAB, "Starting DSRC MAB consumer thread for port: " + mabUdpInPort);

        while (!bShutdown.get())  {
            byte[] buf = new byte[maxPacketSize];
            DatagramPacket p = new DatagramPacket(buf, maxPacketSize);

            try {
                mabInSocket.receive(p);

                int bytesRead = p.getLength();

                if (bytesRead > 0)   {
                    byte[] dsrcPacket = new byte[bytesRead];

                    dsrcPacket = Arrays.copyOf(buf, bytesRead);

                    logger.info(ILogger.TAG_DSRC_MAB, this.getClass().getSimpleName() + " received result of " + bytesRead + " bytes on port: " + mabUdpInPort);
                    logger.debug(ILogger.TAG_DSRC_MAB, this.getClass().getSimpleName() +" Data: " + javax.xml.bind.DatatypeConverter.printHexBinary(dsrcPacket));

                    // forward to DSRC RADIO - need to translate raw data to ascii first
            		String asciiString = DatatypeConverter.printHexBinary(dsrcPacket);
            		byte[] asciiBytes = asciiString.getBytes();

                    DatagramPacket sendPacket = new DatagramPacket(asciiBytes, asciiBytes.length, obuAddress);

                    obuOutSocket.send(sendPacket);

                    logger.info(ILogger.TAG_DSRC_MAB, "DSRC packet forwarded to OBU " + dsrcPacket.length + " bytes on to: " + obuIp + ":" + obuUdpOutPort);
                }
            } catch (SocketTimeoutException ste)   {
                // expected
            } catch (SocketException e) {
                logger.info(ILogger.TAG_DSRC_MAB, "read: socket exception ");
            } catch (IOException e) {
                logger.info(ILogger.TAG_DSRC_MAB, "read: IO exception ");
            }

            //TODO: probably needs to be removed to improve performance once the system is operating reliably
            try   {
                LoggerManager.writeToDisk();
            }
            catch(IOException ioe)   {
            	logger.info(ILogger.TAG_DSRC_MAB, "Error writing log to disk; continuing: " + ioe.getMessage());
                System.err.println("Error writing log to disk: " + ioe.getMessage());
            }
            catch(Exception e) {
            	logger.info(ILogger.TAG_DSRC_MAB, "Error writing log to disk; continuing: " + e.getMessage());
            	System.err.println("Error writing log to disk: " + e.getMessage());
            }
        }

        closeConnection();
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
        logger.info(ILogger.TAG_DSRC_MAB, "Stopping DSRC MAB thread for ports: " + obuUdpOutPort + ":" + mabUdpInPort);
        try {
            obuOutSocket.close();
            if (!(mabInSocket.isClosed())) {
            	mabInSocket.close();
            }
        } catch (Exception e) {
            logger.error(ILogger.TAG_DSRC_MAB, "Error closing DSRC for ports: " + obuUdpOutPort + ":" + mabUdpInPort + ", " + e.getMessage());
        }
    }

}