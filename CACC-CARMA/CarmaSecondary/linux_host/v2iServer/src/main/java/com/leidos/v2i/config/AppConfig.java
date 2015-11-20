package com.leidos.v2i.config;

import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.PropertySource;
import org.springframework.core.env.Environment;

import javax.inject.Inject;
import java.util.Map;

/**
 * AppConfig using v2i.properties instead of application.properties
 *
 * Provide a mechanism for external configuration via a v2i.properties outside the jar in the current directory.
 *
 * Using a v2i.properties file in the same directory as the jar overrides the default properties set inside the jar.
 */
@Configuration
@PropertySource( value = { "classpath:/v2i.properties", "file:v2i.properties" }, ignoreResourceNotFound = true )
public class AppConfig {
    @Inject
    Environment env;

    Map<String, String> properties;

    public int getIntValue(String property)   {
        String value = env.getProperty(property);
        return Integer.parseInt(value);
    }

    public String getProperty(String name) {
        return env.getProperty(name);
    }

    /**
     * Returns provided default value if property is not in properties file
     *
     * @param property
     * @param defaultValue
     * @return int
     */
    public int getDefaultIntValue(String property, int defaultValue)   {
        String value = env.getProperty(property);
        if (value == null)   {
            return defaultValue;
        }

        int result = 0;
        try   {
            result = Integer.parseInt(value);
        }
        catch(Exception e)   {}

        return result;
    }


    public int getPeriodicDelay() {
        return getIntValue("periodicDelay");
    }

    public int getUiRefresh() {
        return getIntValue("uiRefresh");
    }

    public String getGpsHost() {
        return env.getProperty("gps.host");
    }

    public int getGpsPort() {
        return getIntValue("gps.port");
    }

    public int getGpsUdpPort() {
        return getIntValue("gps.udpport");
    }

    public boolean getLogRealTimeOutput() { return getBooleanValue("log.stdout"); }


    public boolean getBooleanValue(String property)   {
        boolean bValue = false;

        String value = getProperty(property);
        if (value == null || value.isEmpty())  {
            bValue = false;
        }
        else   {
            bValue = Boolean.parseBoolean(value);
        }

        return bValue;
    }

    @Override
    public String toString() {
        return "AppConfig [ gps.host:" + getGpsHost() +
                " gps.port:" + getGpsPort() +
                " periodicDelay:" + getPeriodicDelay() +
                " ]";
    }
}