/**
 * Created with IntelliJ IDEA.
 * User: ferenced
 * Date: 8/4/15
 * Time: 6:50 PM
 * To change this template use File | Settings | File Templates.
 */


function _V2IAjax() {}

var V2IAjax = new _V2IAjax();

_V2IAjax.prototype.caccIntent=false;
_V2IAjax.prototype.caccState = 0;
_V2IAjax.prototype.gap = .6;

/**
 * setParameters ajax
 */
_V2IAjax.prototype.ajaxSetParameters = function () {
    "use strict";

    //var operatingSpeed = $( "#operatingSpeedId" ).val();
    var caccIntent = !_V2IAjax.prototype.caccIntent;
    var gap = _V2IAjax.prototype.gap

    var request = $.ajax({
        url : "setParameters",
        dataType : "json",
        type : "post",
        data : {
            caccIntent : caccIntent
        }
    });


    request.done(function(response, textStatus, jqXHR) {
        if ( response.result )   {
            //setDviStatusMessage(response.serverMessage);
            _V2IAjax.prototype.caccIntent = caccIntent;
            setOnOffButton(_V2IAjax.prototype.caccIntent);
        }

    });

    request.fail(function(jqXHR, textStatus, errorThrown) {
        var prefix = "An error occurred setting v2i parameters: ";
        var statusMessage = prefix.concat(textStatus);
        alert(statusMessage);
    });


}

/**
 * getParameters ajax
 */
_V2IAjax.prototype.ajaxGetParameters = function () {
    "use strict";

    var request = $.ajax({
        url : "getParameters",
        dataType : "json",
        type : "post",
        data : {
        }
    });


    request.done(function(response, textStatus, jqXHR) {
        if ( response.result )   {
            setOnOffButton(response.caccIntent);
            _V2IAjax.prototype.caccIntent = response.caccIntent;
            //setDviStatusMessage(response.serverMessage);
        }

    });

    request.fail(function(jqXHR, textStatus, errorThrown) {
        var prefix = "An error occurred setting v2i parameters: ";
        var statusMessage = prefix.concat(textStatus);
        alert(statusMessage);
    });


}
