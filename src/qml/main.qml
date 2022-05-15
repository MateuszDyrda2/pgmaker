import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.5
import QtQuick.Dialogs
import QtQuick.Layouts 6.0


Window {
    id: window
    width: 1280
    height: 720
    visible: true
    color: "#555555"
    title: qsTr("pgmaker")

    GridLayout {
        id: gridLayout
        anchors.fill: parent
        columnSpacing: 5
        flow: GridLayout.TopToBottom
        columns: 5
        rows: 5

        Rectangle{
            id: center
            Layout.column: 1
            Layout.columnSpan: 3
            Layout.row: 1
            Layout.rowSpan: 3
            color: white
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillWidth: true
            //Layout.minimumHeight: 500
           // Layout.minimumWidth: 500
            //Layout.maximumWidth: 8000000



        }
        Rectangle {
            id: leftBar
            Layout.column: 0
            Layout.columnSpan: 1
            Layout.row: 1
            Layout.rowSpan: 3
            color: "#484747"
            Layout.fillHeight: true
            Layout.minimumWidth: 200
            Layout.maximumWidth: 200

            GridLayout {
                id: gridLayout12
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.leftMargin: 19
                anchors.rightMargin: 19
                anchors.topMargin: 19
                flow: GridLayout.LeftToRight
                columns: 2
                rows: 2
                rowSpacing: 2
                columnSpacing: 2





                    MyButton {
                        id: importButton55
                        text: "Import"
                        Layout.preferredWidth: 80
                    }
                    MyButton {
                        id: importButt2on55
                        text: "Import"
                        Layout.preferredWidth: 80
                    }
                    MyButton {
                        id: importB5utt2on55
                        text: "Import"
                        Layout.preferredWidth: 80
                       // Layout.minimumWidth: 40
                      //  Layout.maximumWidth: 80
                    }


            }

        }

        Rectangle {
            id: rightBar
            Layout.column: 4
            Layout.columnSpan: 1
            Layout.row: 1
            Layout.rowSpan: 3
            color: "#484747"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.fillHeight: true
            Layout.minimumWidth: 200
            Layout.maximumWidth: 200
        }

        Rectangle{
            id: topBar
            Layout.column: 0
            Layout.columnSpan: 5
            Layout.row: 0
            Layout.rowSpan: 1
            color: "#3f3b3f"
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            Layout.minimumHeight: 44

            RowLayout {
                id: rowLayout
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.leftMargin: 0
                anchors.bottomMargin: 4
                anchors.topMargin: 0
                spacing: 0

                MyButton {
                    id: importButton
                    text: "Import"
                    Layout.minimumWidth: 40
                    Layout.maximumWidth: 80
                }
                MyButton {
                    id: blockEditorButton
                    text: "Block Editor"
                    Layout.minimumWidth: 40
                    Layout.maximumWidth: 80
                }
                MyButton {
                    id: scriptingButton
                    text: "Scripting"
                    Layout.minimumWidth: 40
                    Layout.maximumWidth: 80
                }
            }

        }
        Rectangle{
            id: bottonBar
            Layout.column: 0
            Layout.columnSpan: 5
            Layout.row: 4
            Layout.rowSpan: 1
            color: "#3f3b3f"
            Layout.alignment: Qt.AlignBottom
            Layout.fillWidth: true
            Layout.minimumHeight: 100

            GridLayout {
                id: gridLayout1
                anchors.fill: parent
                columns: 3
                rows: 2
                Button {
                    id: scriptingButssston


                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.minimumWidth: 40
                    Layout.maximumWidth: 80
                    Layout.column: 2
                 //   Layout.columnSpan: 1
                    Layout.row: 0
                   // icon.name: "play"
                  //  icon.source:
                    icon.color: transparent
                    icon.width: 20
                    icon.height: 20



                }
            }
        }

    }

}


/*
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
    */







/*##^##
Designer {
    D{i:0;formeditorZoom:0.5}D{i:2}D{i:5}D{i:6}D{i:7}D{i:4}D{i:3}D{i:8}D{i:11}D{i:12}
D{i:13}D{i:10}D{i:9}D{i:16}D{i:15}D{i:14}D{i:1}
}
##^##*/
