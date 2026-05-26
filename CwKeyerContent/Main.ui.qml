

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import CwKeyer

Rectangle {
    id: rectangle
    width: Constants.width
    height: Constants.height
    color: Constants.backgroundColor
    signal deviceUpdatedWindow(string name, bool connected)

    Button {
        id: btn_keyer
        text: qsTr("Keyer")
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -140
        anchors.horizontalCenterOffset: -285
        checkable: true
        anchors.horizontalCenter: parent.horizontalCenter

        Connections {
            target: btn_keyer
            function onClicked() {
                console.log("clicked")
            }
        }
    }

    SpinBox {
        id: input_wpm
        x: 121
        y: 84
        value: 20
        editable: true
        to: 50
        from: 10
    }

    Label {
        id: lbl_wpm
        x: 80
        y: 90
        text: qsTr("WPM")
    }

    Button {
        id: btn_detect_device
        x: 8
        y: 8
        text: qsTr("Detect device")

        Connections {
            target: btn_detect_device

            function onClicked() {
                btn_detect_device.enabled = false
                bsy_indicator.opacity = 1.0
                guiConnector.detect_device()
                bsy_indicator.opacity = 0.0
                btn_detect_device.enabled = true
            }
        }
    }

    BusyIndicator {
        id: bsy_indicator
        x: 115
        y: 8
        width: 35
        height: 32
        opacity: 0
        visible: true
    }

    Text {
        id: txt_device
        x: 121
        y: 8
        width: 201
        height: 70
        text: qsTr("")
        font.pixelSize: 20
        property bool connected: true
    }

    Connections {
        target: rectangle

        function onDeviceUpdatedWindow(name, connected) {
            txt_device.text = name
            txt_device.connected = connected
            if (connected) {
                btn_connect.text = "Disconnect"
            } else {
                btn_connect.text = "Connect"
            }
        }
    }

    Button {
        id: btn_connect
        x: 8
        y: 46
        width: 107
        height: 32
        text: qsTr("Connect")

        Connections {
            target: btn_connect
            function onClicked() {
                if (txt_device.connected) {
                    guiConnector.disconnect_device()
                } else {
                    guiConnector.connect_device()
                }
            }
        }
    }

    states: [
        State {
            name: "clicked"
            when: btn_keyer.checked
        }
    ]
}
