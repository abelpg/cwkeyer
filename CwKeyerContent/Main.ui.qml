

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

    Connections {
        target: rectangle

        function onDeviceUpdatedWindow(name, connected) {
            txt_device.text = name
            txt_device.connected = connected
            if (connected) {
                device_connected.color = "#0fad00"
                btn_connect.text = "Disconnect"
            } else {
                device_connected.color = "#b40202"
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

    Rectangle {
        id: line1
        x: 8
        y: 84
        width: 624
        height: 5
        color: "#bbbbbb"
    }

    Rectangle {
        id: device_connected
        x: 124
        y: 8
        width: 70
        height: 70
        color: Constants.backgroundColor
    }

    Rectangle {
        id: rectangle_text
        x: 200
        y: 8
        width: 432
        height: 70
        color: "#00ffffff"
        border.color: "#bbbbbb"
        border.width: 2

        Text {
            id: txt_device
            x: 3
            y: 3
            width: 425
            height: 65
            text: qsTr("")
            font.pixelSize: 20
            property bool connected: true
        }
    }

    Rectangle {
        id: form_keyer
        x: 8
        y: 95
        width: 624
        height: 53
        color: "#00ffffff"
        border.color: "#bbbbbb"
        border.width: 2

        SpinBox {
            id: input_wpm
            x: 49
            y: 8
            value: 20
            editable: true
            to: 50
            from: 10
            onValueChanged: guiConnector.wpm = value
        }

        Label {
            id: lbl_wpm
            x: 8
            y: 14
            text: qsTr("WPM")
        }

        Label {
            id: lbl_mode
            x: 168
            y: 15
            text: qsTr("Mode")
        }

        RadioButton {
            id: radio_ultimate
            x: 225
            y: 9
            text: qsTr("Ultimate")
        }

        RadioButton {
            id: radio_iambic_a
            x: 328
            y: 9
            text: qsTr("Iambic A")
        }

        RadioButton {
            id: radio_iambic_b
            x: 433
            y: 9
            text: qsTr("Iambic B")
        }
    }

    Rectangle {
        id: form_sound
        x: 8
        y: 154
        width: 624
        height: 90
        color: "#00ffffff"
        border.color: "#bbbbbb"
        border.width: 2

        Button {
            id: btn_sound
            x: 8
            y: 8
            text: qsTr("Sound")
        }

        Slider {
            id: amplitude
            x: 416
            y: 46
            value: 0.5
            onValueChanged: guiConnector.amplitude = value
        }

        ComboBox {
            id: sel_device
            x: 122
            y: 8
            width: 494
            height: 32
            model: guiConnector.audioDevices          // ← lista desde C++
            currentIndex: guiConnector.selectedAudioDevice
            onCurrentIndexChanged: guiConnector.selectedAudioDevice = currentIndex
        }

        Rectangle {
            id: sound_connected
            x: 72
            y: 10
            width: 30
            height: 30
            color: Constants.backgroundColor
        }

        Label {
            id: lbl_amplitude
            x: 359
            y: 52
            text: qsTr("Volume")
        }

        SpinBox {
            id: frecuency
            x: 122
            y: 46
            stepSize: 10
            to: 1000
            from: 300
            onValueChanged: guiConnector.frequency = value
        }

        Label {
            id: lbl_frecuency
            x: 8
            y: 54
            text: qsTr("Frecuency")
        }
    }

    states: [
        State {
            name: "clicked"
        }
    ]
}
