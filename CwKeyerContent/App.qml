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
    title: "CwKeyer"


    function deviceUpdated(msg) {
        console.log("Device updated app" +msg);
        var obj = JSON.parse(msg)
        mainScreen.deviceUpdatedWindow(obj["device_name"], obj["connected"]);
    }

    Main {
        id: mainScreen
    }

}

