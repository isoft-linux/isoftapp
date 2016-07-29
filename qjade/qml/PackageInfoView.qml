/*                                                                                 
 * Copyright (C) 2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

import QtQuick 2.2
import QtQuick.Controls 1.0                                                        
import QtQuick.Controls.Styles 1.0                                                 
import cn.com.isoft.qjade 2.0

Rectangle {
    id: packageInfoView
    width: parent.width; height: parent.height

    property string packageTitle
    property StackView stackView
    property Rectangle rect

    Rectangle {                                                                    
        id: returnRegion
        width: 120;height: 60
        color: "#f5f5f5"
        anchors.top: packageInfoView.top
        anchors.left: packageInfoView.left
        anchors.topMargin: 2
        anchors.leftMargin: 2
        border.width: 1
        border.color: "#c9cbce"

        Image {
            id: navImage
            anchors.top: returnRegion.top
            anchors.left: returnRegion.left
            anchors.leftMargin: 15
            source: "../images/navigation_previous_item.png"
            anchors.verticalCenter: parent.verticalCenter
            

        }
        Text {
            text:qsTr("Return")
            anchors.top: returnRegion.top
            anchors.left: navImage.right
            anchors.leftMargin: 10
            anchors.topMargin: 20
            font.pixelSize: 20
            //font.weight: Font.
        }
        MouseArea {
            anchors.fill: parent
            onClicked: { packageInfoView.stackView.pop() }
        }
                                                                                   
        //Rectangle { y: parent.height - 1; width: parent.width; height: 1; color: "#e4ecd7"}
    }
    Rectangle {
        id: titleRegion
        width: (parent.width - 50)
        height: 60
        color: "#f5f5f5"
        anchors.top: packageInfoView.top
        anchors.left: returnRegion.right
        anchors.right: parent.right
        anchors.leftMargin: 3
        anchors.rightMargin: 3
        anchors.topMargin: 2
        border.width: 1
        border.color: "#c9cbce"

        Image {
            id: iconImage
            sourceSize.width: 48; sourceSize.height: 48
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.topMargin: 20
            anchors.verticalCenter: parent.verticalCenter
        }

        // 名称 版本 更新日期 大小
        Rectangle {
            id: leftRect
            anchors.top: iconImage.top
            anchors.left: iconImage.right
            anchors.leftMargin: 15
            width: (parent.width -60)
            height: parent.height
            color: "transparent"

            Text {
                id: titleText
                anchors.top: leftRect.top
                anchors.left: leftRect.left
                anchors.topMargin: 5
                font.pixelSize: 15
                font.weight: Font.Medium
            }

            Text {
                id: versionText
                anchors.top: titleText.bottom
                anchors.left: titleText.left
                anchors.topMargin: 5
                font.pixelSize: 15
                font.weight: Font.Light
            }
        }
    }

    Rectangle {
        id: descRegion
        width: parent.width;height: 40
        color: "#e9f3f8"
        anchors.top: returnRegion.bottom
        anchors.left: packageInfoView.left
        anchors.right: parent.right
        anchors.rightMargin: 3
        anchors.topMargin: 2
        anchors.leftMargin: 2
        border.width: 1
        border.color: "#c9cbce"

        Text {
            text:qsTr("Desc")
            anchors.top: descRegion.top
            anchors.left: descRegion.left
            anchors.leftMargin: 25
            anchors.topMargin: 10
            font.pixelSize: 18
        }
    }

    PackageInfoModel {
        id: packageInfoModel
        title: packageInfoView.packageTitle

        onTitleChanged: {
            myLoader.isVisible = false
            iconImage.source = packageInfoModel.icon

            descText.text = packageInfoModel.description
            snapshotView.model = packageInfoModel.snapshots

            titleText.text =  packageInfoModel.title
            versionText.text = qsTr("Version:") + " " + packageInfoModel.version +
                    "        " + qsTr("Size:") +  " " + packageInfoModel.size
        }
    }

    // 打开/卸载/升级 下拉框
    ComboBox {
        id: actCombox
        width: 90
        //anchors.top: rightRect.top
        anchors.right: parent.right
        anchors.rightMargin: 30
        //anchors.topMargin: rightRect.height/2 - actCombox.height/2
        visible: false
        model: ["Operation", qsTr("Open"), qsTr("Uninstall"), qsTr("Upgrade") ]
        onActivated: {
            descText.text = "test"+ index; //test
            if (index == 1) {
                descText.text = "open it"
            } else if (index == 2) {
                descText.text = "Uninstall it"
            } else if (index == 3) {
                descText.text = "Upgrade it"
            }
        }
    }

    Rectangle {
        id: listRegion
        width: parent.width;
        height: parent.height
        color: "#feffff"
        anchors.top: descRegion.bottom
        anchors.left: descRegion.left
        anchors.right: parent.right
        anchors.rightMargin: 3
        anchors.topMargin: 1
        //anchors.leftMargin: 1
        border.width: 1
        border.color: "#c9cbce"


    Text {
        id: descText
        anchors.top: listRegion.top
        anchors.topMargin: 10
        anchors.left: listRegion.left
        anchors.leftMargin: 25
        anchors.right: parent.right
        anchors.rightMargin: 30
        wrapMode: Text.WordWrap
        font.weight: Font.Light
    }

    ListView {
        id: snapshotView
        width: parent.width
        height: 400
        anchors.top: listRegion.top
        anchors.topMargin: 100
        anchors.left: parent.left
        anchors.leftMargin: 25
        orientation: Qt.Horizontal
        spacing: 35
        delegate: Item {
            width: 380; height: 280

            Image { 
                source: modelData 
                width: parent.width 
                height: parent.height 
                fillMode: Image.PreserveAspectCrop
                clip: true
            }
        }
    }

    }

    MyLoader { id: myLoader }

    MyResultText { id: myResultText; result: qsTr("No App Available") }
}
