/**
 * Created with IntelliJ IDEA.
 * User: ferenced
 * Date: 8/5/15
 * Time: 9:18 AM
 */

// used by android audio fix that requires some type of 'user gesture' to enable sound from client
var audioElements,
    sounds = {};

function displayUiMessage(uiMessage)   {
    "use strict"

    var img = document.getElementById("caccState");
    var caccState = uiMessage.caccState;

    var brakeIndicator = uiMessage.brakeIndicator;
    var brakeIndicatorImage = document.getElementById("brakeIndicatorId");

    if (brakeIndicator > 0)   {
        brakeIndicatorImage.src = "images/Brake.png";
        playAndroidSound();
    }
    else  {
        brakeIndicatorImage.src = "images/BrakeBackground.png";
    }

    if (caccState === 4) {
        img.src = "images/PathCACCState.png";
    }
    else if (caccState === 3)   {
        img.src = "images/PathACCState.png";
    }
    else if (caccState === 2)   {
        img.src = "images/PathCCState.png";
    }
    else if (caccState === 1)   {
        img.src = "images/FactoryAccState.png";
    }
    else   {
        img.src = "images/ManualState.png";
    }
}


function setOnOffButton(flag)    {
    "use strict"

    var img = document.getElementById("onOffButton");


    if (flag === true) {
        img.src = "images/ButtonOn.png";
    }
    else  {
        img.src = "images/ButtonOff.png";
    }
}


function playAndroidSound() {
    // doesn't work on many android devices
    //document.getElementById("audioId").play();

    playSound('audio-fix');
}


function setDviStatusMessage(statusMessage)   {
    "use strict"

    setDviData("statusMessageId", statusMessage);

}


function setDviData(theId, theData)   {
    var id = document.getElementById(theId);
    id.innerHTML = theData;

}



$(window).load(function() {
    audioElements = document.getElementsByTagName('audio');

    for (var i = 0; i < audioElements.length; i++) {
        sounds[audioElements[i].className] = audioElements[i];
    }

// Solves chrome for andriod issue 178297 Require user gesture
// https://code.google.com/p/chromium/issues/detail?id=178297
// Fix based on code from http://blog.foolip.org/2014/02/10/media-playback-restrictions-in-blink/
    if (mediaPlaybackRequiresUserGesture()) {
        window.addEventListener('keydown', removeBehaviorsRestrictions);
        window.addEventListener('mousedown', removeBehaviorsRestrictions);
        window.addEventListener('touchstart', removeBehaviorsRestrictions);
    }

    V2IAjax.ajaxGetParameters();
    connect();
});


$(window).unload(function() {
    disconnect();
});
