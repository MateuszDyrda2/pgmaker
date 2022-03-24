import QtQuick 2.6
import QtQuick.Controls 2.5

Window {
    id: window
    width: 720
    height: 480
    visible: true
    color: "#555555"
    title: qsTr("pgmaker")

    Button {
        id: button
        x: 58
        y: 20
        width: 88
        height: 37
        text: qsTr("Import")
    }
}


