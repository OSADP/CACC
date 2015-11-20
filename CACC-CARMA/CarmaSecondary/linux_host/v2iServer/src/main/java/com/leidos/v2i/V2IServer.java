package com.leidos.v2i;

import com.leidos.v2i.config.V2IApplicationContext;
import com.leidos.v2i.config.AppConfig;
import com.leidos.v2i.logger.ILogger;
import com.leidos.v2i.logger.LoggerManager;
import com.leidos.v2i.services.V2IService;

import org.joda.time.DateTime;
import org.joda.time.format.DateTimeFormat;
import org.joda.time.format.DateTimeFormatter;
import org.springframework.boot.autoconfigure.EnableAutoConfiguration;
import org.springframework.boot.SpringApplication;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.context.annotation.ComponentScan;

import java.io.IOException;

@ComponentScan
@EnableAutoConfiguration
public class V2IServer {

    public static void main(String[] args) {
        ConfigurableApplicationContext context = SpringApplication.run(V2IServer.class, args);

        // set our context for non spring managed classes
        V2IApplicationContext.getInstance().setApplicationContext(context);
        AppConfig config = V2IApplicationContext.getInstance().getAppConfig();

        //SimulatedDviExecutorService service = context.getBean(SimulatedDviExecutorService.class);
        V2IService service = context.getBean(V2IService.class);
        V2IApplicationContext.getInstance().setService(service);

        DateTime now = new DateTime();
        DateTimeFormatter fmt = DateTimeFormat.forPattern("YYYYMMddHHmmss");
        String logName = fmt.print(now);

        LoggerManager.setOutputFile(config.getProperty("log.path") + logName + ".log");
        LoggerManager.setRealTimeOutput(config.getLogRealTimeOutput());
        ILogger logger = LoggerManager.getLogger(V2IServer.class);

        logger.infof("TAG", "####### V2I server started ########");
        try   {
            LoggerManager.writeToDisk();
        }
        catch(IOException ioe)   {
            System.err.println("Error writing log to disk: " + ioe.getMessage());
        }

        service.start();

    }
}