package com.leidos.v2i.state;

/**
 * Created with IntelliJ IDEA.
 * User: ferenced
 * Date: 8/16/15
 * Time: 11:57 AM
 * To change this template use File | Settings | File Templates.
 */
public enum AppState {
    // EMAIL: There is actually a 5-state enumeration “flag” that indicates which state the vehicle is in:  manual, factory ACC, PATH CC, PATH ACC, PATH CACC.
    MANUAL,
    FACTORY_ACC,
    PATH_CC,
    PATH_ACC,
    PATH_CACC;

    public byte[] getAsByteArray()   {
        byte[] result = new byte[1];
        result[0] = (byte) this.ordinal();
        return result;
    }
}
