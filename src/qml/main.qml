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

    FileDialog {
            id: openFileDialog
            title: "Please choose a file"
            onAccepted: {
                console.log(openFileDialog.selectedFile)
            }
            onRejected: {
                console.log("Canceled")
            }
        }

    GridLayout {
        id: mainGridLayout
        anchors.fill: parent
        columnSpacing: 5
        flow: GridLayout.TopToBottom
        columns: 5
        rows: 5

        Rectangle{
            id: centerRectangle
            Layout.column: 1
            Layout.columnSpan: 3
            Layout.row: 1
            Layout.rowSpan: 3
            color: white
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillWidth: true
        }
        Rectangle {
            id: leftBarRectangle
            Layout.column: 0
            Layout.columnSpan: 1
            Layout.row: 1
            Layout.rowSpan: 3
            color: "#484747"
            Layout.fillHeight: true
            Layout.minimumWidth: 200
            Layout.maximumWidth: 200

            GridLayout {
                id: leftBarGridLayout
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
                        id: placeHolder1
                        text: "Placeholder"
                        Layout.preferredWidth: 80
                    }
                    MyButton {
                        id: placeHolder2
                        text: "Placeholder"
                        Layout.preferredWidth: 80
                    }
                    MyButton {
                        id: placeHolder3
                        text: "Placeholder"
                        Layout.preferredWidth: 80
                    }
            }
        }

        Rectangle {
            id: rightBarRectagle
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
            id: topBarRectangle
            Layout.column: 0
            Layout.columnSpan: 5
            Layout.row: 0
            Layout.rowSpan: 1
            color: "#3f3b3f"
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            Layout.minimumHeight: 44

            RowLayout {
                id: topBarRowLayout
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
                    onClicked: {
                        openFileDialog.open();
                    }
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
            id: bottonBarRectangle
            Layout.column: 0
            Layout.columnSpan: 5
            Layout.row: 4
            Layout.rowSpan: 1
            color: "#3f3b3f"
            Layout.alignment: Qt.AlignBottom
            Layout.fillWidth: true
            Layout.minimumHeight: 100

            GridLayout {
                id: bottonBarGridLayout
                anchors.fill: parent
                columns: 3
                rows: 2
                Button {
                    id: playPauseButton
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




/*##^##
Designer {
    D{i:0;formeditorZoom:0.5}D{i:2}D{i:5}D{i:6}D{i:7}D{i:4}D{i:3}D{i:8}D{i:11}D{i:12}
D{i:13}D{i:10}D{i:9}D{i:16}D{i:15}D{i:14}D{i:1}
}
##^##*/
