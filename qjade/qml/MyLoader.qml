/*                                                                                 
 * Copyright (C) 2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 */

import QtQuick 2.2

AnimatedImage {                                                                
    source: "../images/ajax-loader.gif"                                        
    visible: isVisible                                     
    anchors.verticalCenter: parent.verticalCenter                              
    anchors.horizontalCenter: parent.horizontalCenter                         

    property bool isVisible: true 
}
