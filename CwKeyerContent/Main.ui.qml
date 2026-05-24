

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
        anchors.verticalCenterOffset: -216
        anchors.horizontalCenterOffset: -263
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
        x: 165
        y: 8
        value: 20
        editable: true
        to: 50
        from: 10
    }

    Label {
        id: lbl_wpm
        x: 124
        y: 14
        text: qsTr("WPM")
    }
    states: [
        State {
            name: "clicked"
            when: btn_keyer.checked
        }
    ]
}
