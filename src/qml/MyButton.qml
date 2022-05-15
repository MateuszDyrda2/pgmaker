import QtQuick 2.9
import QtQuick.Templates as T
//nie było napisane że trzbea jako templates

T.Button {
    id: taskBarButton
  //  highlighted: true
   flat: true
    property color defaultColor: "#555555"
    property color hoverColor: "#5e5d5d"
    property color clickedColor: "#696868"
   // property bool highlight: false
    //opacity: 0.1


    text: qsTr("TastBarButton")
    implicitHeight: 44
    implicitWidth: 200

    onClicked: {
        console.log(internal.statusColor)
    }

    background: Rectangle{
        id: buttonBackground
       // opacity: enabled ? 1 : 0.3
   //     radius: 5
        color: parent.down ? "red" :
                               (parent.hovered ? "blue" : "black")
       // color: taskBarButton.checked || taskBarButton.highlighted ? "red" :
               //       taskBarButton.flat && !taskBarButton.down ? (taskBarButton.visualFocus ? "yellow" : "black") : "blue"
      //  color: Color.blend(taskBarButton.checked || taskBarButton.highlighted ? "red" : "blue",
        //                                                                    "yellow", taskBarButton.down ? 0.5 : 0.0)
        //border.color: "black"
                //border.width: 0
        // color: parent.down ? "red" : "blue"
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
            id: taskBarButtonText
            text: taskBarButton.text
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            color: "white"
        }
    }



}
