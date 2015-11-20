/**
 * Created with IntelliJ IDEA.
 * User: ferenced
 * Date: 8/20/15
 * Time: 2:35 PM
 * To change this template use File | Settings | File Templates.
 */

// also, see load at bottom of v2i.js

function mediaPlaybackRequiresUserGesture() {
    // test if play() is ignored when not called from an input event handler
    var video = document.createElement('video');
    video.play();
    return video.paused;
}

function removeBehaviorsRestrictions() {
    for (var i = 0; i < audioElements.length; i++) {
        audioElements[i].load();
    }

    window.removeEventListener('keydown', removeBehaviorsRestrictions);
    window.removeEventListener('mousedown', removeBehaviorsRestrictions);
    window.removeEventListener('touchstart', removeBehaviorsRestrictions);
}

function playSound(sound) {
    for (var key in sounds) {
        sounds[key].pause();
    }

    sounds[sound].load();
    sounds[sound].play();
}

//module.exports = {
//    playSound: playSound
//};

