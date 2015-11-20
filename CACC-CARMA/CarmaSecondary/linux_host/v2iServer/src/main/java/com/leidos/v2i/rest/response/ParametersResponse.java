package com.leidos.v2i.rest.response;

public class ParametersResponse extends AjaxResponse {

    private boolean caccIntent;

    public ParametersResponse(boolean result, String serverMessage, boolean caccIntent)   {
        super(result, serverMessage);
        this.caccIntent = caccIntent;
    }

    public boolean getCaccIntent()   {
        return caccIntent;
    }
}
