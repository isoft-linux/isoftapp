/*                                                                                 
 * Copyright (C) 2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 *               2016 fujiang <fujiang.zhu@i-soft.com.cn>
 */

import QtQuick 2.2

Rectangle {
    id: myToolButton
    property string title
    property string icon
    property string normal_color: "transparent"
    property string hover_color: "#0b79ce"
    property bool hover:false
    property bool click:false
    property int  upgrade_number:0
    property bool  upgrade_button:false

    width: 90
    height: 69
    color: click ? "white" : (hover ? "#5eb7fa"  : "transparent")

    Rectangle {
        id: buttonRect
        width: parent.width
        height: 7
        anchors.bottom: parent.bottom
        color: myToolButton.click ? "#0b79ce" : (myToolButton.hover ? "#5eb7fa"  : "#3296e4")
    }

    Text {
        text: title
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: 22
        color: click ? hover_color : (hover ? hover_color : "#e3f0fc")
    }

    /* 有升级包时，在按钮上显示数量 */
    Rectangle {
        id: uptRect
        width: 20
        height: 20
        color: "transparent"
        //radius: width/2
        visible: upgrade_button ? (upgrade_number > 0 ? true : false) : false
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 10
        anchors.leftMargin: 20 + parent.width/2

        Text {
            text: upgrade_number
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 15
            font.weight: Font.Bold
            color: "red"
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: hover = true
        onExited: hover = false
    }
}
