/*                                                                                 
 * Copyright (C) 2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 */

import QtQuick 2.2

Rectangle {
    property string title
    property string icon
    property string normal_color: "transparent"
    property string hover_color: "#1dd7fc"
    property bool hover:false
    property bool click:false
    property int  upgrade_number:0
    property bool  upgrade_button:false

    width: 70
    height: 60
    color: "transparent"

/*
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
*/
    Text {
        text: title
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: 22 //13
        color: click ? hover_color : (hover ? hover_color : "#e3f0fc")
    }

    /* 有升级包时，在按钮上显示数量 */
    Rectangle {
        id: uptRect
        width: 20
        height: 20
        color: "red"
        radius: width/2
        visible: upgrade_button ? (upgrade_number > 0 ? true : false) : false
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 5
        anchors.leftMargin: 10

        Text {
            text: upgrade_number
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 8
            color: "black"
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: hover = true
        onExited: hover = false
    }
}
