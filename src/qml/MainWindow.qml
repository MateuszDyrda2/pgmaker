import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.5
import QtQuick.Dialogs

Window {
    id: window
    width: 1280
    height: 720
    visible: true
    color: "#555555"
    title: qsTr("pgmaker")

    Rectangle{
        height: 44
        color: "#3f3b3f"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.rightMargin: 0
        anchors.leftMargin: 0

    }


    FileDialog {
        id: openFileDialog
        title: "Please choose a file"
        onAccepted: {
            console.log("You chose: " + openFileDialog.selectedFile)
            choosenFileText.text = openFileDialog.selectedFile

        }
        onRejected: {
            console.log("Canceled")
        }
    }


    Button {
        id: importVideoButton
        x: 5
        y: 7
        width: 88
        height: 37
        text: qsTr("Import")
        onClicked: {
            openFileDialog.open();
        }

    }

    Slider {
        id: slider1
        x: 241
        y: 566
        width: 798
        height: 18
        value: 0.5
    }

    Button {
        id: playVideoButton
        x: 550
        y: 503
        width: 88
        height: 37
        text: qsTr("Play")

    }

    Text {
        id: choosenFileText
        x: 533
        y: 285
        width: 142
        height: 16
        text: qsTr("Selected file")
        font.pixelSize: 12
    }

    Button {
        id: pauseVideoButton
        x: 638
        y: 503
        width: 88
        height: 37
        text: qsTr("Pause")
    }

    Button {
        id: blockEditorButton
        x: 988
        y: 7
        width: 88
        height: 37
        text: qsTr("Block editor")
    }

    Rectangle {
        id: rectangle
        width: 235
        color: "#484747"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.topMargin: 44
        anchors.leftMargin: 0
    }

    Rectangle {
        id: rectangle1
        x: 1045
        width: 235
        color: "#484747"
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: 44
        anchors.bottomMargin: 0
        anchors.rightMargin: 0
    }




}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.66}D{i:1}D{i:2}D{i:3}D{i:4}D{i:5}D{i:6}D{i:7}D{i:8}D{i:9}D{i:10}
}
##^##*/
