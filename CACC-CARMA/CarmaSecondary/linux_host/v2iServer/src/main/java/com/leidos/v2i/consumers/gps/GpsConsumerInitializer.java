package com.leidos.v2i.consumers.gps;

import com.leidos.v2i.IConsumerInitializer;
import com.leidos.v2i.logger.ILogger;
import com.leidos.v2i.logger.LoggerManager;

import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.DatagramChannel;
import java.nio.channels.SocketChannel;

public class GpsConsumerInitializer implements IConsumerInitializer {

    private static ILogger logger = LoggerManager.getLogger(GpsConsumerInitializer.class);

    private SocketChannel socketChannel;
    private DatagramChannel datagramChannel;
    private NioUtils nioUtils;
    private String serverIp;
    private int serverTcpPort;
    private int clientUdpPort;

    // first step of the protocol, both sides exchange protocol version byte
    private final static byte[] PROTOCOL_VERSION = { 0x42 };

    // second exchange, both sides set a setApi message
    // we can discard payload from server
    private final static byte[] SET_API = { (byte) 0x89, 0x01, 0x03, 0, 0, 0};

    // third exchange, getUdpPort
    private final static byte[] GET_UDP_PORT = { (byte) 0x90, 0x05 };

    // we fill in the last two bytes of this UDP Port response with our port #
    private final static byte[] GET_UDP_PORT_RESPONSE = { (byte) 0x81, 0x05, 0x02, 0, 0};

    /**
     * Constructor for GpsInitializer which performs client/Pinpoint message protocol on startup
     *
     * @param host
     * @param tcpPort
     * @param clientUdpPort
     * @param socketChannel
     * @param datagramChannel
     */
    public GpsConsumerInitializer(String host, int tcpPort, int clientUdpPort, SocketChannel socketChannel, DatagramChannel datagramChannel)   {
        this.socketChannel = socketChannel;
        this.datagramChannel = datagramChannel;
        this.serverIp = host;
        this.serverTcpPort = tcpPort;
        this.clientUdpPort = clientUdpPort;
        nioUtils = new NioUtils(socketChannel);
    }

    /**
     * The Callable run by the GpsConsumer that performs initialization between client and Pinpoint
     * @return
     * @throws Exception
     */
    public Boolean call() throws Exception   {
        socketChannel.connect(new InetSocketAddress(serverIp, serverTcpPort));

        //***********************************
        // 1) Send/Receive protocol message from PinPoint
        //***********************************
        byte serverProtocolVersion = nioUtils.receiveByte();

        nioUtils.sendBytes(PROTOCOL_VERSION);

        if ((0xF0 & serverProtocolVersion) != (0xF0 & PROTOCOL_VERSION[0]))   {
            logger.info(ILogger.TAG_GPS, "Received invalid protocol version from server.");
        }
        else {
            logger.info(ILogger.TAG_GPS, "Received proper protocol version from server.");
        }

        //***********************************
        // 2) Send/Receive API
        //***********************************

        // Receive setAPI message. This message contains text describing the
        //      messages and signals that the PinPoint accepts
        // first, get 2 bytes of header...
        byte[] controlId = nioUtils.receiveBytes(2);

        byte control = controlId[0];
        byte id = controlId[1];

        byte protocolFlag = (byte) ((0x80 & control) >> 7);
        byte messageType = (byte) ((0x78 & control) >> 3);
        byte sizeLength = (byte) (0x03 & control);


        if (protocolFlag == 1 && messageType == 0x01 && id == 0x01)  {
            logger.info(ILogger.TAG_GPS, "Received setApiMessage:" +
                    " protocolFlag=" + protocolFlag +
                    " messageType=" + messageType +
                    " id=" + id +
                    " sizeLength=" + sizeLength);
        }
        else   {
            logger.info(ILogger.TAG_GPS, "Did not receive setApiMessage:" +
                    " protocolFlag=" + protocolFlag +
                    " messageType=" + messageType +
                    " id=" + id +
                    " sizeLength=" + sizeLength);
        }

        // get size of api payload
        long size = nioUtils.getMessageSize(sizeLength);
        logger.info(ILogger.TAG_GPS, "getMessageSize computed: " + size);

        // read in setApi payload
        byte[] apiPayload = nioUtils.receiveBytes((int) size);
        logger.info(ILogger.TAG_GPS, "Received " + apiPayload.length + " api payload bytes.");


        // send our empty payload
        nioUtils.sendBytes(SET_API);

        //***********************************
        // 3) Setup UDP port communication with the PinPoint
        //***********************************

        //***********************************
        // Create a UDP port/socket that the pinpoint can connect to
        //***********************************
        // DOING IN CONSTRUCTOR NOW AND PROVIDING TO INITIALIZER
        // datagramChannel = DatagramChannel.open();

        InetSocketAddress isa = new InetSocketAddress(clientUdpPort);

        int dynamicPort = isa.getPort();
        datagramChannel.socket().bind(isa);
        logger.info(ILogger.TAG_GPS, "Client setup UDP server on port: " + clientUdpPort);

        //***********************************
        // The PinPoint sends a GetUdpPort message to us, so it knows which port to send
        // the UDP messages on (such as the UDP port ping message)
        //***********************************
        byte[] serverUdpPortRequest = nioUtils.receiveBytes(2);

        if (serverUdpPortRequest[0] == (byte) 0x90 && serverUdpPortRequest[1] == 0x05)   {
            logger.info(ILogger.TAG_GPS, "Received GetUdpPort request from server.");

            byte[] localUdpPortArray = ByteBuffer.allocate(2).putShort((short) clientUdpPort).order(ByteOrder.LITTLE_ENDIAN).array();

            // NOTE: had to switch order, once switched, connected to actual Pinpoint device and retrieved lat/long
            GET_UDP_PORT_RESPONSE[3] = localUdpPortArray[1];
            GET_UDP_PORT_RESPONSE[4] = localUdpPortArray[0];

            nioUtils.sendBytes(GET_UDP_PORT_RESPONSE);
        }

        //***********************************
        //Send getUdpPort message to PinPoint
        //  This message queries the PinPoint to request a response that tells us which UDP port
        //  to connect to on the device
        //***********************************
        nioUtils.sendBytes(GET_UDP_PORT);
        logger.info(ILogger.TAG_GPS, "Client sent getUdpPort to server to request port #.");

        byte[] clientUdpPortResponse = nioUtils.receiveBytes(5);

        // client port is indices 3 and 4
        short clientUdpPort = ByteBuffer.wrap(clientUdpPortResponse).order(ByteOrder.BIG_ENDIAN).getShort(3);
        logger.info(ILogger.TAG_GPS, "Server UDP port: " + clientUdpPort);



        return new Boolean(true);
    }

}
