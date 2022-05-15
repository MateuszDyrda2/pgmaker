import QtQuick 2.9
import QtQuick.Templates as T
//must be template

T.Button {
    id: myButton
    text: qsTr("TastBarButton")
    implicitHeight: 44
    implicitWidth: 200


    background: Rectangle{
        id: myButtonBackground

   //   radius: 5
        color: parent.down ? "#4F4B4F" :
                               (parent.hovered ? "#444044" : "#3f3b3f")

      /*  states:[
         State {
             name: "normal"
             when: !taskBarButton.down
             PropertyChanges {
                 target: buttonBackground
             }
         },
         State {
             name: "down"
             when: taskBarButton.down
             PropertyChanges {
                 target: buttonBackground
                 color: "red"
             }
         }
     ]*/
    }

    contentItem: Item{
        id: item1
        Text{
            id: myButtonText
            text: myButton.text
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            color: "white"
        }
    }
}
