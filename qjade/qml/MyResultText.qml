/*                                                                                 
 * Copyright (C) 2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 */

import QtQuick 2.2

Text {
    visible: isVisible     
    text: result                           
    anchors.verticalCenter: parent.verticalCenter                              
    anchors.horizontalCenter: parent.horizontalCenter
    color: "#989898"
    font.pixelSize: 40 

    property bool isVisible: false
    property string result: "Oops..." 
}
