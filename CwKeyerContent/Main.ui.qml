
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
    signal textCwDecoderUpdated(string text)

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

    Connections {
        target: rectangle
        function onTextCwDecoderUpdated(text) {
            text_cw_decoder.insert(text_cw_decoder.length, text)
            text_cw_decoder.cursorPosition = text_cw_decoder.length
            scroll_text_cw_decoder.ScrollBar.vertical.position = 1.0
                    - scroll_text_cw_decoder.ScrollBar.vertical.size
        }
    }

    Button {
        id: btn_detect_device
        x: 550
        y: 8
        width: 82
        height: 70
        text: qsTr("Search\nZadig device")
        font.pointSize: 9

        Connections {
            target: btn_detect_device

            function onClicked() {
                btn_detect_device.enabled = false
                guiConnector.detectDevice()
                btn_detect_device.enabled = true
            }
        }
    }

    Button {
        id: btn_connect
        x: 11
        y: 8
        width: 107
        height: 70
        text: qsTr("Connect")

        Connections {
            target: btn_connect
            function onClicked() {
                if (txt_device.connected) {
                    guiConnector.disconnectDevice()
                } else {
                    guiConnector.connectDevice()
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
        width: 344
        height: 70
        color: "#00ffffff"
        border.color: "#bbbbbb"
        border.width: 2

        Text {
            id: txt_device
            x: 3
            y: 3
            width: 341
            height: 65
            text: qsTr("")
            font.pixelSize: 16
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
            width: 83
            height: 32
            editable: true
            to: 50
            from: 10
            padding: 3
            font.pointSize: 10
            value: guiConnector.wpm
            onValueChanged: guiConnector.wpm = value
        }

        Label {
            id: lbl_wpm
            x: 8
            y: 14
            text: qsTr("WPM")
        }

        RadioButton {
            id: radio_ultimate
            x: 326
            y: 8
            text: qsTr("Ultimate")
            checked: guiConnector.mode === 1 // ULTIMATIC = 0x1
            onCheckedChanged: if (checked)
                                  guiConnector.mode = 1
        }

        RadioButton {
            id: radio_iambic_a
            x: 419
            y: 8
            text: qsTr("Iambic A")
            checked: guiConnector.mode === 2 // IAMBIC_A = 0x2
            onCheckedChanged: if (checked)
                                  guiConnector.mode = 2
        }

        RadioButton {
            id: radio_iambic_b
            x: 518
            y: 8
            text: qsTr("Iambic B")
            checked: guiConnector.mode === 3 // IAMBIC_B = 0x3
            onCheckedChanged: if (checked)
                                  guiConnector.mode = 3
        }

        Label {
            id: lbl_wordspace
            x: 144
            y: 14
            text: qsTr("Farnsworth")
        }

        SpinBox {
            id: input_wordspace
            x: 222
            y: 8
            width: 83
            height: 32
            padding: 3
            font.pointSize: 10
            editable: true
            to: 50
            from: 10
            value: guiConnector.farnsWorth
            onValueChanged: guiConnector.farnsWorth = value
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

            Connections {
                target: btn_sound
                function onClicked() {
                    if (guiConnector.enabledSound) {
                        guiConnector.setEnabledSound(false)
                    } else {
                        guiConnector.setEnabledSound(true)
                    }
                }
            }
        }

        Slider {
            id: amplitude
            x: 416
            y: 46
            value: guiConnector.amplitude
            onValueChanged: guiConnector.amplitude = value
        }

        ComboBox {
            id: sel_device
            x: 122
            y: 8
            width: 215
            height: 32
            model: guiConnector.audioDevices // ← lista desde C++
            currentIndex: guiConnector.selectedAudioDevice
            onCurrentIndexChanged: guiConnector.selectedAudioDevice = currentIndex
        }

        Rectangle {
            id: sound_connected
            x: 72
            y: 10
            width: 30
            height: 30
            color: guiConnector.enabledSound ? "#0fad00" : "#b40202"
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
            value: guiConnector.frequency
            onValueChanged: guiConnector.frequency = value
        }

        Label {
            id: lbl_frecuency
            x: 8
            y: 54
            text: qsTr("Frecuency")
        }

        CheckBox {
            id: checkbox_send_keyboard
            x: 359
            y: 8
            enabled: guiConnector.enabledZadig
            text: qsTr("Send keyboard active")
            checked: guiConnector.enabledKeyboard
            onToggled: guiConnector.enabledKeyboard = checked
        }
    }

    Rectangle {
        id: form_com_out
        x: 8
        y: 250
        width: 624
        height: 50
        color: "#00ffffff"
        border.color: "#bbbbbb"
        border.width: 2

        Button {
            id: btn_comm_out
            x: 8
            y: 8
            text: qsTr("Comm CW Out")
            Connections {
                target: btn_comm_out
                function onClicked() {
                    if (guiConnector.enabledCommOut) {
                        guiConnector.setEnabledCommOut(false)
                    } else {
                        guiConnector.setEnabledCommOut(true)
                    }
                }
            }
        }

        ComboBox {
            id: select_comm_out
            x: 165
            y: 8
            width: 120
            height: 32
            model: guiConnector.commPorts
            currentIndex: guiConnector.selectedCommPort
            onCurrentIndexChanged: guiConnector.selectedCommPort = currentIndex
        }

        Rectangle {
            id: comm_out_connected
            x: 129
            y: 10
            width: 30
            height: 30
            color: guiConnector.enabledCommOut ? "#0fad00" : "#b40202"
        }

        Button {
            id: btn_comm_in
            x: 334
            y: 10
            text: qsTr("Comm CW In")
            Connections {
                target: btn_comm_in
                function onClicked() {
                    if (guiConnector.enabledCommIn) {
                        guiConnector.setEnabledCommIn(false)
                    } else {
                        guiConnector.setEnabledCommIn(true)
                    }
                }
            }
        }

        Rectangle {
            id: comm_in_connected
            x: 460
            y: 10
            width: 30
            height: 30
            color: guiConnector.enabledCommIn ? "#0fad00" : "#b40202"
        }
        ComboBox {
            id: select_comm_in
            x: 496
            y: 8
            width: 120
            height: 32
            model: guiConnector.commPorts
            currentIndex: guiConnector.selectedCommPortIn
            onCurrentIndexChanged: guiConnector.selectedCommPortIn = currentIndex
        }
    }

    CheckBox {
        id: cw_decoder_active
        x: 11
        y: 306
        text: qsTr("CW Decoder")
        checked: guiConnector.enabledCwDecoder
        onToggled: guiConnector.enabledCwDecoder = checked
    }

    Rectangle {
        id: rectangle_text_cw_decoder
        x: 8
        y: 334
        width: 624
        height: 138
        color: "#00ffffff"
        border.color: "#bbbbbb"
        border.width: 2

        ScrollView {
            id: scroll_text_cw_decoder
            x: 0
            y: 0
            width: 624
            height: 138
            ScrollBar.horizontal.interactive: false
            ScrollBar.vertical.interactive: true
            clip: true

            TextEdit {
                id: text_cw_decoder
                x: 0
                y: 0
                width: 624
                height: 138
                text: qsTr("")
                font.pixelSize: 16
                wrapMode: Text.Wrap
                overwriteMode: true
                readOnly: true
                font.family: "Segoe UI"
            }
        }
    }

    Button {
        id: btn_clear
        x: 582
        y: 310
        width: 50
        height: 24
        text: qsTr("Clear")

        Connections {
            target: btn_clear
            function onClicked() {
                text_cw_decoder.clear()
            }
        }
    }

    states: [
        State {
            name: "clicked"
        }
    ]
}
