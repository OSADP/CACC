package com.leidos.v2i.state;

import com.google.common.primitives.Bytes;
import com.leidos.v2i.logger.ILogger;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.nio.ByteBuffer;
import java.util.Arrays;

import static org.junit.Assert.*;

/**
 * Created with IntelliJ IDEA.
 * User: ferenced
 * Date: 8/16/15
 * Time: 1:53 PM
 */
public class AppStateTester {

    //@Test
    public void testState()   {

        byte[] validateArray = { 0x04 };

        AppState state = AppState.PATH_CACC;

        byte[] arrayState = state.getAsByteArray();
        assertFalse(arrayState == validateArray );
    }

}
