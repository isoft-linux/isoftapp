/*                                                                                 
 * Copyright (C) 2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 */

import QtQuick 2.2

MouseArea {
    cursorShape: Qt.ArrowCursor
    
    property variant clickPos: "1, 1"
    
    onPressed: {
        cursorShape = Qt.SizeAllCursor
        clickPos = Qt.point(mouse.x, mouse.y)
    }

    onPositionChanged: {
        var delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y)
        rootWindow.x += delta.x
        rootWindow.y += delta.y
    }

    onExited: cursorShape = Qt.ArrowCursor
}
