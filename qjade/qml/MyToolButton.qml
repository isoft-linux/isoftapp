/*                                                                                 
 * Copyright (C) 2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 */

import QtQuick 2.2

Rectangle {
    property string title
    property string icon
    property string normal_color: "#a4b97b"
    property string hover_color: "#9bb26c"
    property bool hover
    property bool click

    width: 70 //75
    height: 60 //72
    color: click ? hover_color : (hover ? hover_color : normal_color)

    Rectangle {
        width: 0 //5
        height: parent.height
        color: click ? "#4b5b2a" : (hover ? "#4b5b2a" : "transparent")
    }

    Rectangle {
        width: parent.width
        height: 1
        color: "#e3ebd4"
    }

    Image {
        id: myToolButtonIcon
        source: icon
        y: 12 // 14
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        text: title
        y: myToolButtonIcon.y + myToolButtonIcon.height + 7
        anchors.horizontalCenter: myToolButtonIcon.horizontalCenter
        font.pixelSize: 13
        color: "#e6ecde"
    }

    MouseArea {
        anchors.fill: parent                                               
        hoverEnabled: true
        onEntered: hover = true
        onExited: hover = false
    }
}
