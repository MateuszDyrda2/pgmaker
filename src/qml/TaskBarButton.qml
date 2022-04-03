import QtQuick 2.9
import QtQuick.Controls 2.5
import "qrc:/qml/TaskBarButton"

Button {
    id: taskBarButton

    property color defaultColor: "red"
    property color hoverColor: "yellow"
    property color clickedColor: "white"

    QtObject{
        id: internal

        property var statusColor: if(taskBarButton.down){
                                      taskBarButton.down ? clickedColor : defaultColor
                                  } else{
                                      taskBarButton.hovered ? hoverColor : defaultColor

                                  }
    }

    text: qsTr("TastBarButton")
    implicitHeight: 40
    implicitWidth: 200

    background: Rectangle{
        color : internal.statusColor
    }
    contentItem: Item{
        id: item1
        Text{
            id: taskBarButtonText
            text: taskBarButton.text
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            color: "white"
        }
    }

}
