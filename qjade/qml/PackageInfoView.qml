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

    property string packageName
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
        }
        MouseArea {
            anchors.fill: parent
            onClicked: { packageInfoView.stackView.pop() }
        }
                                                                                   
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

        Rectangle {
            id: leftRect
            anchors.top: iconImage.top
            anchors.left: iconImage.right
            anchors.leftMargin: 15
            width: parent.width*2/3
            height: parent.height - 10
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

        JadedBus {
            id: jadedBus

            property bool isError: false
            property string info: "UnknownInfo"

            Component.onCompleted: {
                jadedBus.info = jadedBus.getInfo(packageName)

                if (jadedBus.info == "UnknownInfo") {
                    funcButton.text = "unknown"
                } else if (jadedBus.info == "InfoRunning") {
                    actCombox.visible = false
                    funcButton.visible = false;
                    infoText.visible = true;
                    infoText.text = qsTr("Running");
                } else if (jadedBus.info == "InfoWaiting") {
                    funcButton.visible = false
                    actCombox.visible = false
                    infoText.visible = true
                    infoText.text = qsTr("Waiting")
                } else if (jadedBus.info == "InfoInstalled") {
                    funcButton.visible = false
                    infoText.visible = false
                    actCombox.visible = true
                } else if (jadedBus.info == "InfoUpdatable") {
                    funcButton.text = qsTr("Update")
                }
            }
            onErrored: {
                if (name == packageName) {
                    if (detail == "lastest")
                        titleText.text = packageInfoModel.title + " (" + qsTr("Is lastest") + ")"
                    else
                    titleText.text = packageInfoModel.title + " (" + qsTr("Error") + ")"
                    funcButton.visible = true;
                    infoText.visible = false;
                }
            }

            onPerChanged: {
                if (name == packageInfoModel.name) {
                    progressInfo.value = perCent;
                    if (perCent != 100) {
                        actCombox.visible = false
                        funcButton.visible = false
                        infoText.visible = true
                        infoText.text = qsTr("Waiting")
                        progressInfo.visible = true

                    }

                }
            }

            onTaskChanged: {
                if (name == packageInfoModel.name) {
                    jadedBus.info = jadedBus.getInfo(name)
                    if (jadedBus.info == "InfoInstalled") {
                        funcButton.visible = false
                        infoText.visible = false
                        actCombox.visible = true
                        progressInfo.visible = false;

                    } else {
                        funcButton.visible = true
                        infoText.visible = false
                        actCombox.visible = false
                        progressInfo.visible = false;
                    }
                }
            }
        }

        ProgressBar {
            id: progressInfo
            width:  iconImage.width
            anchors.left: iconImage.left
            y: iconImage.y + iconImage.height - 15
            maximumValue:  100
            value : 0
            visible: false
        }

        PercentageButton {
            id: funcButton
            text: qsTr("Install")
            anchors.right: parent.right
            anchors.rightMargin: 17
            anchors.verticalCenter: parent.verticalCenter
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    if (funcButton.text == qsTr("Install")) {
                        jadedBus.install(packageInfoModel.name)
                    } else if (funcButton.text == qsTr("Update")) {
                        jadedBus.update(packageInfoModel.name)
                    }
                    funcButton.visible = false
                    infoText.visible = true
                    infoText.text = qsTr("Waiting")
                }
            }
        }

        Text {
            id: infoText
            text: qsTr("Installed")
            anchors.top: funcButton.top
            anchors.left: funcButton.left
            font.pixelSize: 11
            color: "#979797"
            visible: false
        }

        ComboBox {
            id: actCombox
            width: funcButton.width
            anchors.top: funcButton.top
            anchors.left: funcButton.left
            model: [qsTr("Open"), qsTr("Uninstall"), qsTr("Upgrade"),qsTr("SelectOp") ]
            visible: false
            currentIndex: 3

            onPressedChanged:     {
                currentIndex = 3
            }
            onActivated: {
                if (index == 0) {
                    jadedBus.runCmd(packageInfoModel.name)
                } else if (index == 1) {
                    jadedBus.uninstall(packageInfoModel.name)
                } else if (index == 2) {
                    jadedBus.update(packageInfoModel.name)
                }
                currentIndex = 3

                if (index != 0) {
                    actCombox.visible = false
                    infoText.visible = true
                    infoText.text = qsTr("Waiting")
                }
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

            jadedBus.info = jadedBus.getInfo(packageInfoModel.name)
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
        anchors.right: parent.right
        anchors.rightMargin: 25

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

        property int curIdx: 0
        property int maxIdx: snapshotView.count
        property int gridWidth: 200
        function advance(steps) {
             var nextIdx = curIdx + steps
             if (nextIdx < 0 || nextIdx > maxIdx)
                return;
             snapshotView.contentX += gridWidth * steps;
             curIdx += steps;
        }

        Image {
            source: "../images/new_array.png"
            MouseArea{
                anchors.fill: parent
                onClicked: {snapshotView.advance(-1) }
            }
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            opacity: snapshotView.curIdx == 0 ? 0.2 : 1.0
            Behavior on opacity {NumberAnimation{}}
            visible: snapshotView.count > 2 ? true : false
        }
        Image {
            source: "../images/new_array.png"
            mirror: true
            MouseArea{
                anchors.fill: parent
                onClicked: {snapshotView.advance(1)}
            }
            opacity: snapshotView.curIdx == snapshotView.maxIdx ? 0.2 : 1.0
            Behavior on opacity {NumberAnimation{}}
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            visible: snapshotView.count > 2 ? true : false
        }
    }

    }

    MyLoader { id: myLoader }

    MyResultText { id: myResultText; result: qsTr("No App Available") }
}
