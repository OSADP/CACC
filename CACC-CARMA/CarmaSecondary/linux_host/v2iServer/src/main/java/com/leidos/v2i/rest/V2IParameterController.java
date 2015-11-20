package com.leidos.v2i.rest;

import com.leidos.v2i.config.AppConfig;
import com.leidos.v2i.logger.ILogger;
import com.leidos.v2i.logger.LoggerManager;
import com.leidos.v2i.rest.response.AjaxResponse;
import com.leidos.v2i.rest.response.ParametersResponse;
import com.leidos.v2i.state.V2IParameters;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;


@RestController
public class V2IParameterController {

    @Autowired
    AppConfig appConfig;

    private static ILogger logger = LoggerManager.getLogger(V2IParameterController.class);

    @RequestMapping("/setParameters")
    public AjaxResponse setParameters(@RequestParam(value="caccIntent", required=true) boolean caccIntent)   {

        boolean result = true;

        String statusMessage =  "Setting caccIntent: " + caccIntent;
        logger.info("REST", statusMessage);

        V2IParameters parameters = V2IParameters.getInstance();

        try   {
            parameters.setCaccIntent(caccIntent);
        }
        catch(Exception e)   {
            result = false;
            statusMessage = "Error setting V2I Parameters: " + e.getMessage();
        }

        return new AjaxResponse(result, statusMessage);

    }


    @RequestMapping("/getParameters")
    public AjaxResponse getParameters()   {

        boolean result = true;
        boolean caccIntent = false;

        V2IParameters parameters = V2IParameters.getInstance();

        caccIntent = parameters.getCaccIntent();
        String statusMessage =  "Getting caccIntent: " + caccIntent;
        logger.info("REST", statusMessage);

        return new ParametersResponse(result, statusMessage, caccIntent);

    }

}