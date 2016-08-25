/*
 * Copyright (C) 2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

import QtQuick 2.2
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0
import cn.com.isoft.qjade 2.0
import QtQml.Models 2.2

Rectangle {
    id: packageByCategoryView
    width: parent.width
    height: parent.height

    property string category
    property string title
    property bool loading

    property var selectedItemList:[]

    PackageByCategoryModel {
        id: pkModel
        category: packageByCategoryView.category
        onPackageChanged: myLoader.visible = false;
        onError: myLoader.visible = false;
    }

    // 全部软件：共有xx款软件
    Rectangle {
        id: titleRegion
        width: parent.width; height: 43
        color: "#f5f5f5"

        Text {
            id: countText
            text: packageByCategoryView.title + ": " + qsTr("%1 total").arg(pkModel.packages.length)
            font.pixelSize: 14
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
        }

        // 分割线
        Rectangle { y: parent.height - 1; width: parent.width; height: 1; color: "#e4ecd7"}
    }

    ScrollView {
        id: viewScroll
        width: parent.width
        height: parent.height - titleRegion.height - 60 //bottonAct.height
        anchors.top: titleRegion.bottom
        flickableItem.interactive: true

        ListView {
            id: pkListView
            model: pkModel.packages
            anchors.fill: parent

            delegate: Rectangle {
                id: pkRect
                objectName: "rectItem"
                width: parent.width
                height: 60
                color: index % 2 == 0 ? "white" : "#f5f5f5" // 每条记录颜色区别;index 不准确

                // 批处理单选按钮 allChecked 按下时发送信号
                signal checkItem(bool checked)

                onCheckItem: {
                    //nameText.text = "abc" + index
                    if(appCheckBox.enabled)
                    appCheckBox.checked = checked
                }

                JadedBus {
                    id: jadedBus

                    property bool isError: false
                    property string info: "UnknownInfo"

                    Component.onCompleted: {
                        jadedBus.info = jadedBus.getInfo(modelData.name)
                        //allChecked.text = index

                        var item = pkListView.contentItem.children[index]
                        item.state = "checked"
                        pkRect.objectName = "rectItem"
                        if (jadedBus.info == "UnknownInfo") {
                            // 过滤
                            item.state = "" // for allChecked

                            pkRect.visible = false; // 不显示此软件包的信息
                            pkRect.height = 0;
                            pkModel.removeAt(index);
                        } else if (jadedBus.info == "InfoRunning") {
                            actCombox.visible = false
                            funcButton.visible = false;
                            infoText.visible = true;
                            infoText.text = qsTr("Running"); // 运行中
                        } else if (jadedBus.info == "InfoWaiting") {
                            funcButton.visible = false
                            actCombox.visible = false
                            infoText.visible = true
                            infoText.text = qsTr("Waiting") // 等待
                        } else if (jadedBus.info == "InfoInstalled") {
                            funcButton.visible = false
                            infoText.visible = false // 已安装最新版本
                            actCombox.visible = true // 已经安装，则显示下拉框
                            appCheckBox.enabled = false

                            // 已经安装的，不再次安装
                            item.state = "" // for allChecked
                        } else if (jadedBus.info == "InfoUpdatable") {
                            funcButton.text = qsTr("Update") // 软件升级
                        }
                    }
                    onErrored: {
                        if (name == modelData.name) {
                            if (detail == "lastest")
                                nameText.text = modelData.title + " (" + qsTr("Is lastest") + ")"
                            else
                            nameText.text = modelData.title + " (" + qsTr("Error") + ")"
                            funcButton.visible = true;
                            infoText.visible = false;
                        }
                    }

                    // 安装/卸载时 接收到的进度
                    onPerChanged: {
                        if (name == modelData.name) {
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
                        if (name == modelData.name) {
                            jadedBus.info = jadedBus.getInfo(name)
                            if (jadedBus.info == "InfoInstalled") {
                                funcButton.visible = false
                                infoText.visible = false // 已安装最新版本
                                actCombox.visible = true // 已经安装，则显示下拉框
                                appCheckBox.enabled = false

                                // 已经安装的，不再次安装
                                var item = pkListView.contentItem.children[index]
                                item.state = "" // for allChecked

                                progressInfo.visible = false;

                                if (appCheckBox.checked) {
                                    appCheckBox.checked = false
                                }

                            } else {
                                funcButton.visible = true
                                infoText.visible = false
                                actCombox.visible = false

                                appCheckBox.enabled = true
                                var item2 = pkListView.contentItem.children[index]
                                item2.state = "checked" // for allChecked

                                progressInfo.visible = false;
                            }
                        }
                    }
                }
                // 图标前面的单选按钮
                CheckBox {
                    id: appCheckBox
                    anchors.left: parent.left
                    anchors.leftMargin: 18
                    anchors.verticalCenter: parent.verticalCenter
                    scale: 0.9

                    checked: false
                    onCheckedChanged: {
                        // 软件包前面的单选按钮被按下的处理
                        var tmpItem = modelData.name
                        var findit = false
                        if (appCheckBox.checked) {
                            for (var i = 0; i < selectedItemList.length; i++) {
                                if (selectedItemList[i] == tmpItem) {
                                    nameText.text = modelData.title
                                    findit = true
                                }
                            }
                            if (findit == false) {
                                selectedItemList.push(modelData.name)
                                nameText.text = modelData.title
                            }

                        } else {
                            for (var j = 0; j < selectedItemList.length; j++) {
                                if (selectedItemList[j] == tmpItem) {
                                    var nullVar = ""
                                    selectedItemList[j] = nullVar
                                }
                            }
                        }
                    }

                    }

                // 软件图标
                Image {
                    id: appIcon
                    source: modelData.icon
                    asynchronous: true
                    sourceSize.width: 48; sourceSize.height: 48
                    anchors.left: appCheckBox.right //parent.left
                    anchors.leftMargin: 1 //7
                    anchors.verticalCenter: parent.verticalCenter

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            stackView.push({item: Qt.resolvedUrl("PackageInfoView.qml"),
                            properties: {packageName:modelData.name,packageTitle: modelData.title,
                                         stackView: stackView}})
                        }
                    }
                }

                // 安装/卸载时，显示此进度条
                ProgressBar {
                    id: progressInfo
                    width:  appIcon.width
                    anchors.left: appIcon.left
                    //anchors.top:  appIcon.bottom
                    y: appIcon.y + appIcon.height - 15 // 进度条位置
                    maximumValue:  100
                    value : 0
                    visible: false
                }

                // 软件名称
                Text {
                    id: nameText
                    text: modelData.title
                    anchors.left: appIcon.right
                    anchors.leftMargin: 8
                    anchors.top: parent.top
                    anchors.topMargin: 6
                    font.pixelSize: 14

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            stackView.push({item: Qt.resolvedUrl("PackageInfoView.qml"),
                            properties: {packageName:modelData.name,packageTitle: modelData.title,
                                         stackView: stackView}});
                        }
                    }
                }

                // 软件简单描述
                Text {
                    text: modelData.description
                    width: parent.width/2
                    height: appIcon.height - nameText.height
                    anchors.left: nameText.left
                    anchors.top: nameText.bottom
                    anchors.topMargin: 5
                    wrapMode: Text.WordWrap
                    font.pixelSize: 11
                    color: "#b7b7b7"
                }

                // 软件包大小 xxM
                Text {
                    id: sizeText
                    text: modelData.size
                    font.pixelSize: 11
                    color: "#979797"
                    anchors.right: funcButton.left
                    anchors.rightMargin: 67
                    anchors.verticalCenter: parent.verticalCenter
                    visible: true
                }

                // 安装按钮
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
                                jadedBus.install(modelData.name)
                            } else if (funcButton.text == qsTr("Update")) {
                                jadedBus.update(modelData.name)
                            }
                            funcButton.visible = false
                            infoText.visible = true
                            infoText.text = qsTr("Waiting")
                        }
                    }

                }

                // 安装提示信息
                Text {
                    id: infoText
                    text: qsTr("Installed")
                    anchors.top: funcButton.top
                    anchors.left: funcButton.left
                    font.pixelSize: 11
                    color: "#979797"
                    visible: false
                }

                // 安装完成后，显示下拉框
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
                            jadedBus.runCmd(modelData.name)
                        } else if (index == 1) {
                            nameText.text = modelData.title
                            jadedBus.uninstall(modelData.name)
                        } else if (index == 2) {
                            jadedBus.update(modelData.name)
                        }
                        currentIndex = 3

                        if (index != 0) {
                            actCombox.visible = false
                            infoText.visible = true
                            infoText.text = qsTr("Waiting")
                        }
                    }
                }

                // 分割线
                Rectangle {
                    width: parent.width; height: 1
                    anchors.top: pkRect.bottom
                    anchors.left: parent.left
                    color: "#e4ecd7"
                }


            }
        }

        style: MyScrollViewStyle {}
    }

    // 分割线
    Rectangle {
        id: beforeBottonAct
        anchors.top: viewScroll.bottom
        width: parent.width
        height: 1
        color: "#e4ecd7"
    }

    // 批处理单选按钮
    CheckBox {
        id: allChecked
        anchors.left: parent.left
        anchors.leftMargin: 18
        anchors.top: beforeBottonAct.bottom
        anchors.topMargin: 10
        checked: false
        text: qsTr("install all selected items") // + pkListView.count
        onClicked: { // todo...
            delete selectedItemList
            for (var i = 0; i < pkListView.contentItem.children.length; ++i) {
                var item = pkListView.contentItem.children[i]
                if (typeof(item) == 'undefined' ) {
                    continue;
                }
                else if(item.objectName != "rectItem") {
                    continue;
                }
                //if (item.state == "checked")
                    item.checkItem(checked)

            }
            bottonAct.enabled = true
        }

    }


    // 多选，一键安装/卸载/升级
    PercentageButton {
        id: bottonAct
        text: qsTr("InstallAll")
        anchors.right: parent.right
        anchors.rightMargin: 17
        anchors.top: beforeBottonAct.bottom
        anchors.topMargin: 10
        // 批处理单选按钮 被按下了，此按钮才会可用
        enabled: allChecked.checked? true:false

        onClicked: {
            // 遍历每个被选中的软件包，作相同的动作
            for (var i = 0; i < selectedItemList.length; i++) {
                if (selectedItemList[i] != "") {
                    jadedBus.install(selectedItemList[i])
                }
            }

            allChecked.checked = false
            bottonAct.enabled = false
        }
    }



    MyLoader { id: myLoader; isVisible: packageByCategoryView.loading }
}
