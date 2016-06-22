/*                                                                                 
 * Copyright (C) 2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>   
 * Copyleft 2014 Jeff Bai <jeffbaichina@members.fsf.org>                    
 */

import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Controls 1.0
import cn.com.isoft.qjade 2.0

MyWindow {
    id: aboutView
    title: qsTr("About")
    modality: Qt.ApplicationModal

    Brand { id: brand }

    Text {
        id: nameText
        text: brand.name
        anchors.top: parent.top; anchors.left: parent.left
        anchors { topMargin: 74; leftMargin: 22 }
        font.pixelSize: 17
        color: "#4b5b2a"
    }

    Text {
        id: descText
        text: brand.slogan
        anchors.top: nameText.bottom; anchors.left: nameText.left
        anchors.topMargin: 7
        font.pixelSize: 12
        color: "#4a5b2f"
    }

    AppInfo { id: appInfo }

    Text {
        id: versionText
        text: qsTr("Version: %1").arg(appInfo.version)
        anchors.top: descText.bottom; anchors.left: nameText.left
        anchors.topMargin: 60
        font.pixelSize: 10
        color: "#999999"
    }

    Text {
        text: qsTr("Originally designed for 2014 iSOFT Infrastructure Software co., Ltd, and developed by AOSC in advance.")
        width: 400
        wrapMode: Text.WordWrap
        anchors.top: versionText.bottom; anchors.left: versionText.left
        anchors.topMargin: 8
        font.pixelSize: 10
        color: "#999999"
    }

    Image {
        source: brand.logo
        sourceSize.width: 128; sourceSize.height: 128
        anchors.top: parent.top; anchors.right: parent.right
        anchors { topMargin: 60; rightMargin: 40 }
    }

    Button { 
        text: qsTr("OK")
        anchors.right: parent.right; anchors.bottom: parent.bottom
        anchors.margins: 30
        onClicked: close()
    }
}
