/*                                                                                 
 * Copyright (C) 2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 */

import QtQuick 2.2
import QtQuick.Controls.Styles 1.0

ScrollViewStyle {                                                   
    transientScrollBars: true                                              
    
    handle: Rectangle {                                                    
        implicitWidth: 7                                                   
        color: "#c1c1c1"
        anchors.fill: parent                                               
        anchors.topMargin: 6                                               
        anchors.bottomMargin: 6                                            
        radius: 6                                                          
    }                                                                      
    
    scrollBarBackground: Rectangle {                                       
        implicitWidth: 7                                                   
        border.width: 1                                                    
        border.color: "#e4e4e4"
        color: "#fdfdfd"                                                   
        radius: 6                                                          
    }                                                                      
}
