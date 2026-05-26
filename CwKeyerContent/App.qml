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

    function deviceConnected() {
        console.log("Device connected");

    }

    function deviceDisconnected() {
        console.log("Device disconnected" );

    }

    function deviceInitiated(msg) {
        console.log("Device initiated app" +msg);
        mainScreen.deviceUpdatedWindow(msg);
    }

    function deviceUpdated(msg) {
        console.log("Device updated app" +msg);
        mainScreen.deviceUpdatedWindow(msg);
    }

    Main {
        id: mainScreen
    }

}

