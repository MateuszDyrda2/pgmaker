import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.5
import QtQuick.Dialogs

Window {
    id: window
    width: 720
    height: 480
    visible: true
    color: "#555555"
    title: qsTr("pgmaker")



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
        x: 58
        y: 20
        width: 88
        height: 37
        text: qsTr("Import")
        onClicked: {
            openFileDialog.open();
        }

    }

    Slider {
        id: slider1
        x: 208
        y: 432
        width: 309
        height: 18
        value: 0.5
    }

    Button {
        id: playVideoButton
        x: 58
        y: 86
        width: 88
        height: 37
        text: qsTr("Play")

    }

    Text {
        id: choosenFileText
        x: 303
        y: 46
        width: 142
        height: 16
        text: qsTr("Selected file")
        font.pixelSize: 12
    }


}
