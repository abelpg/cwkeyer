import QtQuick
import CwKeyer

Window {
    id: window
    width: mainScreen.width
    height: mainScreen.height

    visible: true
    title: "CwKeyer"

    function onDeviceUpdated(msg) {
        console.log("onChangeScreenQml\n")
    }

    Main {
        id: mainScreen
        anchors.centerIn: parent
    }

}

