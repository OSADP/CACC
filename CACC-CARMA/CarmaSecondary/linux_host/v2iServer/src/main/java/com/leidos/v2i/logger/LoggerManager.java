package com.leidos.v2i.logger;

import com.leidos.v2i.appcommon.version.VehicleApplicationVersion;

import org.joda.time.DateTime;
import org.joda.time.format.DateTimeFormat;
import org.joda.time.format.DateTimeFormatter;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Created by rushk1 on 10/9/2014.
 */
public class LoggerManager {
    private static HashMap<Class, Logger> loggerInstances = new HashMap<>();
    private static String outputFilename = "log.txt";
    private static ArrayList<LogListener> globalListeners = new ArrayList<>();
    private static LogEntry.Level minOutputToWrite = null;
    private static boolean realTimeOutput = false;
    private static boolean firstWrite = true;
    private static AtomicBoolean bRecordData = new AtomicBoolean(true);        // true, normal log operation...false, gobble and ignore

    /**
     * Create or retrieve an instance of an {@link com.leidos.v2i.logger.ILogger} implementation for class origin.
     * @param origin The class for which the logger is requested
     * @return Either a preexisting logger instance spawned before or a new one
     */
    public static ILogger getLogger(Class origin) {
        // Search to see if we already have an active logger instance for that class, make a new one if not
        if (loggerInstances.containsKey(origin)) {
            return loggerInstances.get(origin);
        } else {
            Logger logger = new Logger(origin);

            // Register all current listeners with the new logger
            for (LogListener listener : globalListeners) {
                logger.registerLogListener(listener);
            }

            logger.setRealTimeOutput(realTimeOutput);

            loggerInstances.put(origin, logger);
            return logger;
        }
    }

    public static void setRealTimeOutput(boolean val) {
        for (Logger log : loggerInstances.values()) {
            log.setRealTimeOutput(val);
        }
        realTimeOutput = val;
    }

    public static void setOutputFile(String filename) {
        outputFilename = filename;
    }

    /**
     * Gather up all the unwritten log entries from all Loggers produced from this factory. Collect the entries from the
     * uwritten buffer, merge them all into a collective buffer and write that buffer to file. Then notify each logger
     * that it's unwritten buffer has been saved to file.
     */
    public synchronized static void writeToDisk() throws IOException {
        // Collect all the LogEntries by non-destructively merging them
        LogBuffer toBeWritten = new LogBuffer();
        for (Logger logger : loggerInstances.values()) {
            toBeWritten = LogBuffer.merge(toBeWritten, logger.getBuffer());
        }

        // Optionally filter the data based on desired output level
        if (minOutputToWrite != null) {
            toBeWritten = toBeWritten.filterByLogLevel(minOutputToWrite);
        }

        // Open the FileOutputStream in append=true mode
        FileOutputStream outstream = new FileOutputStream(new File(outputFilename), true);

        // Write the date to the top of the file
        if (firstWrite) {
            PrintStream out = new PrintStream(outstream);
            DateTime dt = DateTime.now();
            DateTimeFormatter fmt = DateTimeFormat.forPattern("MM-dd-yyyy");
            String version = new VehicleApplicationVersion().getFullId();
            out.println(fmt.print(dt) + "    " + version);

            out.close();
            firstWrite = false;
        }

        toBeWritten.write(outstream);
        outstream.close();

        // Notify each Logger of the write
        for (Logger logger : loggerInstances.values()) {
            logger.notifyWritten();
        }
    }

    /**
     * Registers a callback with all currently instantiated ILoggers from this factory
     * @param listener The {@link com.leidos.v2i.logger.LogListener} object to be registered
     */
    public static void registerGlobalLoggerCallback(LogListener listener) {
        for (Logger logger : loggerInstances.values()) {
            logger.registerLogListener(listener);
        }
    }

    /**
     * Only LogEntries more severe than min will be written to output file. Severity is defined in {@link com.leidos.v2i.logger.LogEntry.Level}.
     * Low ordinal values in the Level enum are considered higher severity.
     *
     * By default no filtering is performed so all log levels will be outputted and no performance cost will be incurred.
     * After this method is invoked however, ever call thereafter to writeToDisk will have to iterate the output
     * {@link com.leidos.v2i.logger.LogBuffer} to filter {@link com.leidos.v2i.logger.LogEntry}s based on
     * their severity level even if the minimum output to write is set to the lowest log level again.
     *
     * @param min The minimum log severity to be written, inclusive.
     */
    public static void setMinOutputToWrite(LogEntry.Level min) {
        minOutputToWrite = min;
    }

    /**
     * Return value of current min setting of output
     *
     * @return  LogeEntry.Level
     */
    public static LogEntry.Level getMinOutputToWrite() {
        return minOutputToWrite;
    }


    /**
     * Provide method to conditionally determine if we should add debug messages
     *
     * @return boolean true if DEBUG set
     */
    public static boolean isDebug() {
        return (minOutputToWrite == LogEntry.Level.DEBUG);
    }


    /**
     * Close the logger for class origin, making it unable to write to file or other forms of output
     * @param origin The class to close the logger for
     */
    public static void closeLogger(Class origin) {
        throw new UnsupportedOperationException();
    }

    /**
     * Set whether we want consumer data logging to begin or stop
     *
     * @param flag
     */
    public static void setRecordData(boolean flag)   {
        bRecordData.getAndSet(flag);
    }

    /**
     * Returns the state of the record data flag
     *
     * @return true to log, false to not log
     */
    public static boolean getRecordData()   {
        return bRecordData.get();
    }

}
