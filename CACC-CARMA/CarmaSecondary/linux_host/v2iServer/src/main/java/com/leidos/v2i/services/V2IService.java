package com.leidos.v2i.services;

import com.leidos.v2i.IConsumerInitializer;
import com.leidos.v2i.command.CommandStatusProcessor;
import com.leidos.v2i.config.AppConfig;
import com.leidos.v2i.config.V2IApplicationContext;
import com.leidos.v2i.consumers.dsrc.MabDsrcProcessor;
import com.leidos.v2i.consumers.dsrc.ObuDsrcProcessor;
import com.leidos.v2i.consumers.gps.GpsScheduledMessageConsumer;
import com.leidos.v2i.logger.ILogger;
import com.leidos.v2i.logger.LogEntry;
import com.leidos.v2i.logger.LoggerManager;
import com.leidos.v2i.state.V2IParameters;
import com.leidos.v2i.ui.UiMessage;

import org.springframework.beans.factory.DisposableBean;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.net.DatagramSocket;
import java.net.SocketException;

@Service("v2iService")
public class V2IService implements DisposableBean {
    private static ILogger logger = LoggerManager.getLogger(V2IService.class);

    @Autowired
    private AppConfig appConfig;

    @Autowired
    private SimpMessagingTemplate template;

    ObuDsrcProcessor obuDsrcProcessor;
    MabDsrcProcessor mabDsrcProcessor;

    GpsScheduledMessageConsumer gpsConsumer;

    CommandStatusProcessor commandProcessor;

    public void start()   {
        setLogLevel();
        LoggerManager.setRecordData(true);

        // allow running of components via configuration
        if ( V2IApplicationContext.getInstance().getAppConfig().getIntValue("dsrc.enable") == 1 )   {
            initDsrcProcessor();
        }

        if ( V2IApplicationContext.getInstance().getAppConfig().getIntValue("gps.enable") == 1 )   {
            initGpsConsumer();
        }

        if ( V2IApplicationContext.getInstance().getAppConfig().getIntValue("command.enable") == 1 )   {
            initCommandProcessor();
        }
    }


    protected void initGpsConsumer()   {
        gpsConsumer = new GpsScheduledMessageConsumer();
        gpsConsumer.initialize();
        IConsumerInitializer gpsInitializer = gpsConsumer.getInitializer();

        Boolean bInit;
        try   {
            bInit = gpsInitializer.call();
            logger.debug(ILogger.TAG_GPS, "--- initGpsConsumer: ready to schedule messages.");
            //try { LoggerManager.writeToDisk(); } catch(Exception e)   { System.err.println("Error writing to log file:" + e.getMessage()); }
            if (bInit)   {
                gpsConsumer.sendScheduleMessages();
                //try { LoggerManager.writeToDisk(); } catch(Exception e)   { System.err.println("Error writing to log file:" + e.getMessage()); }
            }
            else   {
                // a consumer failed to initialize, stop the app
                logger.error("GPS", "GPS failed to initialize, please review the log");
                //try { LoggerManager.writeToDisk(); } catch(Exception e)   { System.err.println("Error writing to log file:" + e.getMessage()); }
            }
        }
        catch(Exception e)   {
            logger.error("GPS", "Error initializing GPS handshaking: " + e.getMessage());
            try { LoggerManager.writeToDisk(); } catch(Exception e2)   { logger.info(ILogger.TAG_GPS, "Error writing to log file:" + e2.getMessage()); }
        }
        logger.debug(ILogger.TAG_GPS, "--- bottom of initGpsConsumer.");
        try { LoggerManager.writeToDisk(); } catch(Exception e2)   { logger.info(ILogger.TAG_GPS, "Error writing to log file:" + e2.getMessage()); }

    }

    protected void initDsrcProcessor() {
    	
    	int udpTimeout = V2IApplicationContext.getInstance().getAppConfig().getIntValue("udp.timeout");
    	int mabDsrcInport = V2IApplicationContext.getInstance().getAppConfig().getIntValue("mab.dsrc.inport");
    	int mabDsrcOutport = V2IApplicationContext.getInstance().getAppConfig().getIntValue("mab.dsrc.outport");

    	//due to port budget limitations on the MAB, we may need to consolidate the in & out ports there;
    	// no such problem talking to the OBU.
    	try {
			DatagramSocket mabInSocket = new DatagramSocket(mabDsrcInport);  //will be closed by MabDsrcProcessor or ObuDsrcProcessor
			mabInSocket.setSoTimeout(udpTimeout);
			DatagramSocket mabOutSocket = null;
			if (mabDsrcInport == mabDsrcOutport) {
				mabOutSocket = mabInSocket;
			}else {
				mabOutSocket = new DatagramSocket(mabDsrcOutport);  //will be closed by MabDsrcProcessor or ObuDsrcProcessor
				mabOutSocket.setSoTimeout(udpTimeout);
			}

	    	mabDsrcProcessor = new MabDsrcProcessor();
	        mabDsrcProcessor.initialize(mabInSocket);
	    	
	        obuDsrcProcessor = new ObuDsrcProcessor();
	        obuDsrcProcessor.initialize(mabOutSocket);
	        logger.infof(ILogger.TAG_EXECUTOR, "Established MAB sockets on ports %d (in), %d (out)", mabDsrcInport, mabDsrcOutport);

    	} catch (SocketException e) {
			logger.error(ILogger.TAG_EXECUTOR, "Unable to open MAB socket(s) for DSRC: " + e.toString());
		}
    }

    protected void initCommandProcessor()   {
        V2IParameters.getInstance().setService(this);

        commandProcessor = new CommandStatusProcessor();
        commandProcessor.initialize();


    }


    public void sendCommandToMab(byte[] command)   {
        try   {
            commandProcessor.writeCommandToMab(command);
        }
        catch(Exception ioe)   {
            logger.error(ILogger.TAG_COMMAND, "Error writing CACC state to MAB: " + ioe.getMessage());
        }
    }


    public synchronized void sendUiMessage(UiMessage uiMessage)   {
        logger.info(ILogger.TAG_DVI,  "Send to client: " + uiMessage.toString());
        template.convertAndSend("/topic/dvitopic", uiMessage);

        try   {
            LoggerManager.writeToDisk();
        }
        catch(IOException ioe)   {
            System.err.println("Error writing log to disk: " + ioe.getMessage());
        }

    }


    @Override
    public void destroy()   {

        if (obuDsrcProcessor != null)  obuDsrcProcessor.terminate();
        if (mabDsrcProcessor != null)  mabDsrcProcessor.terminate();
        if (gpsConsumer != null)  gpsConsumer.terminate();
        if (commandProcessor != null) commandProcessor.terminate();

        try   {
            Thread.sleep(100);
        }
        catch(Exception e) {};
        logger.info(ILogger.TAG_EXECUTOR,  "Destroying bean V2IService via lifecycle destroy().");
    }

    /**
     * Set min log level
     *
     * If not configured or configured incorrectly, uses DEBUG
     */
    public void setLogLevel()   {
        String logLevel = appConfig.getProperty("log.level");

        LogEntry.Level enumLevel = null;

        try   {
            enumLevel = LogEntry.Level.valueOf(logLevel.toUpperCase());
        }
        catch(Exception e)   {
            logger.warn("EXEC", "log.level value improperly configured: " + logLevel);
            enumLevel = LogEntry.Level.DEBUG;
        }

        LoggerManager.setMinOutputToWrite(enumLevel);
    }

}
