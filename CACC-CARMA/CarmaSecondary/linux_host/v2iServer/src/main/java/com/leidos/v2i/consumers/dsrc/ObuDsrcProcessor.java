package com.leidos.v2i.consumers.dsrc;

import com.google.common.primitives.Bytes;
import com.leidos.v2i.config.V2IApplicationContext;
import com.leidos.v2i.logger.ILogger;
import com.leidos.v2i.logger.LoggerManager;

import java.io.IOException;
import java.net.*;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicBoolean;


/**
 * ObuDsrcProcessor
 *
 * Listens for incoming DSRC on asd.inport
 * Forwards incoming messages to MAB mab.host, mab.dsrc.outport
 */
public class ObuDsrcProcessor implements Runnable {

    private static ILogger logger = LoggerManager.getLogger(ObuDsrcProcessor.class);

    private AtomicBoolean bShutdown = new AtomicBoolean(false);
    private int maxPacketSize;
    private int udpTimeout;

    // DSRC receiver configuration
    private DatagramSocket obuInSocket;
    private int obuUdpInPort;

    // MAB configuration for DSRC
    private DatagramSocket mabOutSocket;
    private String mabIp;
    private int mabUdpOutPort;
    private InetSocketAddress mabAddress;

    //private DateTime lastTimeSent;

    public ObuDsrcProcessor(){
        this.maxPacketSize = V2IApplicationContext.getInstance().getAppConfig().getIntValue("asd.maxpacketsize");
        this.udpTimeout = V2IApplicationContext.getInstance().getAppConfig().getIntValue("udp.timeout");

        this.obuUdpInPort = V2IApplicationContext.getInstance().getAppConfig().getIntValue("asd.inport");

        this.mabUdpOutPort = V2IApplicationContext.getInstance().getAppConfig().getIntValue("mab.dsrc.outport");
        this.mabIp = V2IApplicationContext.getInstance().getAppConfig().getProperty("mab.host");
        this.mabAddress = new InetSocketAddress(mabIp, mabUdpOutPort);

    }

    /**
     * initialize
     */
    public void initialize(DatagramSocket mabSocket){
        logger.info(ILogger.TAG_DSRC_OBU, "Initializing DSRC OBU Receive Interface for port: " + obuUdpInPort);

        //establish the UDP sockets
        try   {
            obuInSocket = new DatagramSocket(obuUdpInPort);
            obuInSocket.setSoTimeout(udpTimeout);
            
            mabOutSocket = mabSocket;
        }
        catch(Exception e)   {
            logger.error(ILogger.TAG_DSRC_OBU, "Exception initializing DSRC sockets: " + e.getMessage());
        }

        new Thread(this).start();
    }


    @Override
    public void run() {
        logger.info(ILogger.TAG_DSRC_OBU, "Starting DSRC OBU consumer thread for port: " + obuUdpInPort);

        while (!bShutdown.get())  {
            byte[] buf = new byte[maxPacketSize];
            DatagramPacket p = new DatagramPacket(buf, maxPacketSize);

            try {
            	obuInSocket.receive(p);

                int bytesRead = p.getLength();

                if (bytesRead > 0)   {
                	logger.debugf(ILogger.TAG_DSRC_OBU, "bytesRead = %d", bytesRead);
                	
                    byte[] dsrcPacket = new byte[bytesRead];
                    dsrcPacket = Arrays.copyOf(p.getData(), bytesRead);

                    byte[] wsmpMessage = extractWsmp(dsrcPacket);
                    logger.info(ILogger.TAG_DSRC_OBU, "Received " + bytesRead + " bytes on port: " + obuUdpInPort);
                    logger.info(ILogger.TAG_DSRC_OBU, "wsmpMessage: " + javax.xml.bind.DatatypeConverter.printHexBinary(wsmpMessage));

                    // forward to DSRC MAB
                    DatagramPacket sendPacket = new DatagramPacket(wsmpMessage, wsmpMessage.length, mabAddress);
                    mabOutSocket.send(sendPacket);
                    logger.info(ILogger.TAG_DSRC_MAB, "DSRC packet forwarded to MAB " + wsmpMessage.length + " bytes on to: " + mabIp + ":" + mabUdpOutPort);
                }

            } catch (SocketTimeoutException ste)   {
                // expected
            } catch (SocketException e) {
                logger.info(ILogger.TAG_DSRC_OBU, "read: socket exception: " + e.toString());
            } catch (IOException e) {
                logger.info(ILogger.TAG_DSRC_OBU, "read: IO exception: " + e.toString());
            }
        }

        closeConnection();
    }

    protected int wsmpLength; //number of bytes in the full WSMP message

    /**
     * extractWsmp
     * finds the core wsmp message within the data buffer
     *
     * @param data - the raw input data stream
     * @return the wsmp message
     */
    public byte[] extractWsmp(byte[] data) {
        ByteBuffer stream;
        stream = ByteBuffer.wrap(data);

        //look for the sequence that indicates the beginning of the raw BSM
        byte[] bsmStartIndicator = {(byte)0x80, 0x01, 0x02};

        int index = Bytes.indexOf(data, bsmStartIndicator);

        if (index > 0)   {
            int wsmpStart = findWsmp(stream, index); //sets wsmpLength also
            int usableLength = Math.min(wsmpLength, data.length - wsmpStart); //in case the length decoding is bogus, prevents buffer overflow problems
            logger.debugf(ILogger.TAG_DSRC_OBU, "extractWsmp found wsmpStart = %d, wsmpLength = %d, usableLength = %d", 
            				wsmpStart, wsmpLength, usableLength);

            //throw away first wsmpStart bytes in stream to set its position so the next get() will work
            byte[] trash = new byte[wsmpStart];
            stream.rewind();
            stream.get(trash, 0, wsmpStart); 

            byte[] wsmp = new byte[usableLength];
            stream.get(wsmp, 0, usableLength);
            return wsmp;
        } else {
            logger.warn(ILogger.TAG_DSRC_OBU, "extractWsmp failed to find beginning of BSM");
            return data;
        }
    }

    /**
     * findWsmp
     * Backs up from the start of the BSM message to find the beginning of the wrapping WSMP and decodes its length
     *
     * @param stream - the raw data stream
     * @param bsmStart - first byte of the BSM in the stream
     * @return index of starting byte of the WSMP message; sets member variable wsmpLength also
     */
    protected int findWsmp(ByteBuffer stream, int bsmStart) {

        //if there isn't enough data prior to start of BSM just return 0 (it will represent a bogus message)
        if (bsmStart < 2) return 0;

        //back up over one or more length bytes and look for the beginning of the WSMP element indicator
        int wsmpStart = bsmStart - 1; //skip the first length byte as it doesn't tell us how many length bytes there are
        boolean found = false;
        do {
            if (stream.get(--wsmpStart) == 0x30) {
                found = true;
            }
        } while (!found  &&  wsmpStart > 0);

        wsmpLength = 0;
        if (found) {
            //move forward again and decode the length bytes
            int lengthLength = bsmStart - wsmpStart - 1;
            int index = wsmpStart + 1;
            logger.debugf(ILogger.TAG_DSRC_OBU, "lengthLength = %d, index = %d", lengthLength, index);
            logger.debugf(ILogger.TAG_DSRC_OBU, "   Data at index = %02x %02x %02x %02x %02x %02x %02x %02x",
            				stream.get(index), stream.get(index+1), stream.get(index+2), stream.get(index+3),
            				stream.get(index+4), stream.get(index+5), stream.get(index+6), stream.get(index+7));

            if (lengthLength >= 1) { //if there's no valid length info just allow a bogus message to be returned
            	
            	int bsmLength = 0;
                byte it = stream.get(index);
            	//if length encoding is definite short form then
                if ((it & 0x80) == 0) {
                	//just this one byte is the length
                	bsmLength = (int)it;
                	logger.debugf(ILogger.TAG_DSRC_OBU, "Length encoded definite short form = %d", bsmLength);
                	
                //else if length encoding is definite long form then
                }else if ((it & 0x7f) > 0) {
                	int lengthBytesRemaining = (int)(it & 0x7f);
                	logger.debugf(ILogger.TAG_DSRC_OBU, "lengthBytesRemaining = %d", lengthBytesRemaining);
                	while (lengthBytesRemaining > 0) {
                		it = stream.get(++index);
                		int itVal = (int)it & 0x000000ff;
                		bsmLength |= itVal << 8*(lengthBytesRemaining - 1);
                		--lengthBytesRemaining;
                	}
                	logger.debugf(ILogger.TAG_DSRC_OBU, "Length encoded definite long form = %d", bsmLength);
                	
                //else length has been encoded in the indefinite form then
                }else {
                	logger.warn(ILogger.TAG_DSRC_OBU, "Length encoded as indefinite form!");
                }
                
                wsmpLength = bsmLength + lengthLength + 1; //the 1 represents the 0x30 indicator byte
            }
        }

        return wsmpStart;
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
        logger.info(ILogger.TAG_DSRC_OBU, "Stopping DSRC OBU thread for ports: " + obuUdpInPort + ":" + mabUdpOutPort);
        try {
            obuInSocket.close();
            if (!(mabOutSocket.isClosed())) {
            	mabOutSocket.close();
            }
        } catch (Exception e) {
            logger.error(ILogger.TAG_DSRC_OBU, "Error closing DSRC for ports: " + obuUdpInPort + ":" + mabUdpOutPort + ", " + e.getMessage());
        }
    }
}