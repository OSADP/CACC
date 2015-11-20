package com.leidos.v2i.state;

import com.leidos.v2i.config.V2IApplicationContext;
import com.leidos.v2i.services.V2IService;
import org.joda.time.DateTime;
import org.joda.time.format.DateTimeFormat;
import org.joda.time.format.DateTimeFormatter;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class V2IParameters {
        private static Logger logger = LoggerFactory.getLogger(V2IParameters.class);

    /**
     * caccState indicates what state we are in based on BSM from MAB
     */
        private AppState caccState = AppState.MANUAL;

    /**
     * caccIntent indicates whether the user DESIRED to enter CACC state.  Simply a user toggle flag that does not
     * mandate specific AppState, that is to be determined by PATH
     */
        private boolean caccIntent = false;
        private double gap = 1.6;
        private double minGap = 0.6;

        private static final byte[] CACC_ON = { 0x01 };
        private static final byte[] CACC_OFF = { 0x00 };

        private V2IService v2iService;

        private V2IParameters() {
        }

        private static class V2IParametersHolder  {
            private static final V2IParameters _instance = new V2IParameters();
        }

        public static V2IParameters getInstance()
        {
            return V2IParametersHolder._instance;
        }

        public AppState getCaccState()   {
            return caccState;
        }

        public void setCaccState(AppState state)   {
            this.caccState = state;
        }

        public boolean getCaccIntent()   {
            return caccIntent;
        }

        public void setCaccIntent(boolean flag)   {
            this.caccIntent = flag;
            if (v2iService != null)   {
                v2iService.sendCommandToMab(getCaccIntentAsArray());
            }
        }

        public double getGap()   {
            return gap;
        }

    /**
     * We are sending 0x01 ON or 0x00 OFF to MAB to indicate user INTENT for CACC state
     *
     * @return
     */
        public byte[] getCaccIntentAsArray()   {
            if (getCaccIntent())   {
                return CACC_ON;
            }

            return CACC_OFF;
        }

        public void setGap(double gap)   {
            this.gap = gap;
        }

        public int getCaccStateOrdinal()   {
            return caccState.ordinal();
        }

        public byte[] getCaccStateAsArray()   {
            return caccState.getAsByteArray();
        }

        public void setService(V2IService service)   {
            this.v2iService = service;
        }
    }
