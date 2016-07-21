/*                                                                                 
 * Copyright (C) 2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 */

import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Controls 1.0

Window {
    id: myWindow
    width: 442; height: 266
    flags: Qt.FramelessWindowHint 
    x: (Screen.width - width) / 2; y: (Screen.height - height) / 2
    color: "transparent"

    property string title: "Title"

    BorderImage {                                                                  
        anchors.fill: parent 
        border { left: 10; top: 10; right: 10; bottom: 10 }                        
        source: "../images/shadow.png"                                                       
    }

    Rectangle {
        width: parent.width - 19; height: parent.height - 19
        anchors.top: parent.top
        anchors.left: parent.left
        anchors { topMargin: 10; leftMargin: 10 }
        color: "white"
    }

    Rectangle {                                                                
        id: myWindowTitle
        width: parent.width - 19; height: 40
        anchors.top: parent.top 
        anchors.left: parent.left
        anchors { topMargin: 10; leftMargin: 10 }
        color: "#3296e4"

        Text { 
            text: myWindow.title
            anchors.left: parent.left
            anchors.leftMargin: 22
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 15
            color: "#fdfffc"
        }
        
        MyWindowButton {                                                           
            name: "close"
            isCloseWindow: true
            sourceHeight: 7          
            anchors.top: parent.top; anchors.right: parent.right
            anchors { topMargin: 5; rightMargin: 5 }
        }                                          
    }
}
