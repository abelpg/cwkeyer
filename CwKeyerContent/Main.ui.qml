

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

    Button {
        id: btn_keyer
        text: qsTr("Keyer")
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -146
        anchors.horizontalCenterOffset: -250
        checkable: true
        anchors.horizontalCenter: parent.horizontalCenter

        Connections {
            target: btn_keyer

            function onClicked() {
                usbDevice.list_devices()
            }
        }
    }

    SpinBox {
        id: input_wpm
        x: 156
        y: 78
        value: 20
        editable: true
        to: 50
        from: 10
    }

    Label {
        id: lbl_wpm
        x: 115
        y: 84
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
        x: 156
        y: 8
        width: 161
        height: 32
        text: qsTr("")
        font.pixelSize: 12
    }
    states: [
        State {
            name: "clicked"
            when: btn_keyer.checked
        }
    ]
}
