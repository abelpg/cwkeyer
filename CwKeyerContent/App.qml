import QtQuick
import CwKeyer

Window {
    id: window
    width: mainScreen.width
    height: mainScreen.height
    visible: true
    minimumHeight: 480
    minimumWidth: 640
    maximumHeight: 480
    maximumWidth: 640
    title: "CwKeyer - EA1FXG"


    function deviceUpdated(msg) {        
        var obj = JSON.parse(msg)
        mainScreen.deviceUpdatedWindow(obj["device_name"], obj["connected"]);
    }

    function textCwDecoderUpdated(msg) {
        mainScreen.textCwDecoderUpdated(msg);
    }

    Main {
        id: mainScreen
    }

}

