/*                                                                                 
 * Copyright (C) 2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>  
 * Copyleft 2014 Jeff Bai <jeffbaichina@members.fsf.org>                     
 */

import QtQuick 2.2
import QtQuick.Window 2.1

Rectangle {
    id: myWindowButton
    width: 20
    height: 20
    color: "transparent"

    property string name: "minimize"
    property int sourceWidth: 8
    property int sourceHeight: 8
    property bool hover: false
    property bool maximize: false
    property bool isCloseWindow: false

    Image {
        source: "../images/window_" + name + (myWindowButton.hover ? "" : "_hover") + ".png"
        sourceSize.width: myWindowButton.sourceWidth
        sourceSize.height: myWindowButton.sourceHeight
        anchors.verticalCenter: parent.verticalCenter 
        anchors.horizontalCenter: parent.horizontalCenter
    }
    
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: myWindowButton.hover = true
        onExited: myWindowButton.hover = false
        onClicked: {
            if (myWindowButton.name == "minimize") {
                rootWindow.visibility = Window.Minimized
            } else if (myWindowButton.name == "close") {
                if (myWindowButton.isCloseWindow) 
                    close() 
                else 
                    Qt.quit()
            }
        }
    }
}
