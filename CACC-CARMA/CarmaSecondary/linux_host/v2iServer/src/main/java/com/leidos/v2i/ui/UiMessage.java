package com.leidos.v2i.ui;

import com.leidos.v2i.logger.ILogger;
import com.leidos.v2i.logger.LoggerManager;

/**
 * Created with IntelliJ IDEA.
 * User: ferenced
 * Date: 8/18/15
 * Time: 12:01 AM
 *
 * This class represents a UI message sent to the client via WebSockets.  It is converted to JSON.
 *
 */
public class UiMessage {

    private static ILogger logger = LoggerManager.getLogger(UiMessage.class);

    private int caccState;
    private int brakeIndicator;

    /**
     *
     * @param caccState         enum ordinal of AppState
     * @param brakeIndicator    > 0, make noise
     */
    public UiMessage(int caccState, int brakeIndicator)   {
        this.caccState = caccState;
        this.brakeIndicator = brakeIndicator;
    }

    public int getCaccState()   {
        return caccState;
    }

    public int getBrakeIndicator()   {
        return brakeIndicator;
    }

    @Override
    public String toString()   {
        return "UIMessage [ caccState=" + caccState +
                ", brakeIndicator=" + brakeIndicator +
                " ]";
    }

}
