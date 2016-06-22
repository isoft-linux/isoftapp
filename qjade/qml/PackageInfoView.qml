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
        id: titleRegion                                                            
        width: parent.width; height: 40                                       
        color: "#f5f5f5"                                                           
        
        Image {
            id: navImage
            source: "../images/navigation_previous_item.png"
            anchors.verticalCenter: parent.verticalCenter
            
            MouseArea {
                anchors.fill: parent
                onClicked: { packageInfoView.stackView.pop() }
            }
        }
                                                                                   
        Rectangle { y: parent.height - 1; width: parent.width; height: 1; color: "#e4ecd7"}
    }

    PackageInfoModel {
        id: packageInfoModel
        title: packageInfoView.packageTitle

        onTitleChanged: {
            myLoader.isVisible = false
            iconImage.source = packageInfoModel.icon

            descText.text = qsTr("Desc:") + packageInfoModel.description
            snapshotView.model = packageInfoModel.snapshots

            titleText.text = qsTr("Name:") + packageInfoModel.title
            versionText.text = qsTr("Version:") + packageInfoModel.version
            uptDAteText.text = qsTr("UptDate:") + packageInfoModel.datetime
            sizeText.text = qsTr("Size:") + packageInfoModel.size

            developerText.text = qsTr("Developer:") + packageInfoModel.title
            langText.text = qsTr("Language:") + packageInfoModel.title
            cateText.text = qsTr("Category:") + packageInfoModel.category
            dlCountText.text = qsTr("DownloadCount:") + packageInfoModel.title
        }
    }

    Image {
        id: iconImage
        sourceSize.width: 128; sourceSize.height: 128
        anchors.top: titleRegion.bottom
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.topMargin: 20
    }
/*
    Text {
        id: titleText
        anchors.top: iconImage.top
        anchors.left: iconImage.right
        anchors.leftMargin: 20
        font.pixelSize: 18
    }
*/
    // 名称 版本 更新日期 大小
    Rectangle {
        id: leftRect
        anchors.top: iconImage.top
        anchors.left: iconImage.right
        anchors.leftMargin: 15
        width: iconImage.height >100 ? (iconImage.height*2) : (128*2)
        height: iconImage.height>100 ? (iconImage.height - 10):118
        //color: "red"

        Text {
            id: titleText
            anchors.top: leftRect.top
            anchors.left: leftRect.left
            anchors.topMargin: 5
            font.pixelSize: 15
        }

        Text {
            id: versionText
            anchors.top: titleText.bottom
            anchors.left: titleText.left
            anchors.topMargin: 5
            font.pixelSize: 15
        }
        Text {
            id: uptDAteText
            anchors.top: versionText.bottom
            anchors.left: versionText.left
            anchors.topMargin: 5
            font.pixelSize: 15
        }
        Text {
            id: sizeText
            anchors.top: uptDAteText.bottom
            anchors.left: uptDAteText.left
            anchors.topMargin: 5
            font.pixelSize: 15
        }

    }

    // 开发者 语言 类别 下载量
    Rectangle {
        id: rightRect
        anchors.top: iconImage.top
        anchors.left: leftRect.right
        anchors.leftMargin: 15
        width: iconImage.height > 100 ? (iconImage.height*2) : (128*2)
        height: iconImage.height >100 ? (iconImage.height - 10):118
        //color: "green"

        Text {
            id: developerText
            anchors.top: rightRect.top
            anchors.left: rightRect.left
            anchors.topMargin: 5
            font.pixelSize: 15
        }

        Text {
            id: langText
            anchors.top: developerText.bottom
            anchors.left: developerText.left
            anchors.topMargin: 5
            font.pixelSize: 15
        }
        Text {
            id: cateText
            anchors.top: langText.bottom
            anchors.left: langText.left
            anchors.topMargin: 5
            font.pixelSize: 15
        }
        Text {
            id: dlCountText
            anchors.top: cateText.bottom
            anchors.left: cateText.left
            anchors.topMargin: 5
            font.pixelSize: 15
        }

    }

    // 打开/卸载/升级 下拉框
    ComboBox {
        id: actCombox
        width: 90
        anchors.top: rightRect.top
        anchors.right: parent.right
        anchors.rightMargin: 30
        anchors.topMargin: rightRect.height/2 - actCombox.height/2
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



    Text {
        id: descText
        anchors.top: iconImage.bottom //titleText.bottom
        anchors.topMargin: iconImage.height >100 ? 2:(128 - iconImage.height)
        anchors.left: iconImage.left // titleText.left
        anchors.right: parent.right
        anchors.rightMargin: 30
        wrapMode: Text.WordWrap
    }

    ListView {
        id: snapshotView
        width: parent.width
        height: 400
        anchors.top: iconImage.bottom
        anchors.topMargin: 100
        anchors.left: parent.left
        anchors.leftMargin: 20
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

    MyLoader { id: myLoader }

    MyResultText { id: myResultText; result: qsTr("No App Available") }
}
