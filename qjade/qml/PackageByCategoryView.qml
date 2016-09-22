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
    property var notInstallItemList:[]

    PackageByCategoryModel {
        id: pkModel
        category: packageByCategoryView.category
        onPackageChanged: myLoader.visible = false;
        onError: myLoader.visible = false;
    }

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

        Rectangle { y: parent.height - 1; width: parent.width; height: 1; color: "#e4ecd7"}
    }

    ScrollView {
        id: viewScroll
        width: parent.width
        height: parent.height - titleRegion.height - 60
        anchors.top: titleRegion.bottom
        flickableItem.interactive: true

        ListView {
            id: pkListView
            model: pkModel.packages
            anchors.fill: parent
            property int pre_index:-1

            delegate: Rectangle {
                id: pkRect
                objectName: "rectItem"
                width: parent.width
                height: 60
                color: index % 2 == 0 ? "white" : "#f5f5f5"

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

                        if (pkListView.pre_index != pkListView.index) {
                            if (allChecked.checked) {
                                allChecked.checked = false
                                allChecked.pressed = false

                                delete selectedItemList
                                delete notInstallItemList
                                for (var i = 0; i < pkListView.contentItem.children.length; ++i) {
                                    var item = pkListView.contentItem.children[i]
                                    if (typeof(item) == 'undefined' ) {
                                        continue;
                                    }
                                    else if(item.objectName != "rectItem") {
                                        continue;
                                    }
                                    item.checkItem(false)
                                }
                            }
                            bottonAct.enabled = false
                        }

                        var item = pkListView.contentItem.children[index]
                        if (typeof(item) != 'undefined' ) {
                        item.state = "checked"
                        }
                        pkRect.objectName = "rectItem"
                        if (jadedBus.info == "UnknownInfo") {
                            item.state = ""
                            pkRect.visible = false;
                            pkRect.height = 0;
                            pkModel.removeAt(index);
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
                            appCheckBox.enabled = false

                            if (typeof(item) != 'undefined' ) {
                            item.state = "" // for allChecked
                            }
                        } else if (jadedBus.info == "InfoUpdatable") {
                            funcButton.text = qsTr("Update")
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
                                infoText.visible = false
                                actCombox.visible = true
                                appCheckBox.enabled = false

                                var item = pkListView.contentItem.children[index]
                                if (typeof(item) != 'undefined' ) {
                                item.state = "" // for allChecked
                                }

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

                CheckBox {
                    id: appCheckBox
                    anchors.left: parent.left
                    anchors.leftMargin: 18
                    anchors.verticalCenter: parent.verticalCenter
                    scale: 0.9

                    checked: false
                    onCheckedChanged: {
                        var tmpItem = modelData.name
                        var findit = false
                        var k = 0
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

                            for (k = 0; k < notInstallItemList.length; k++) {
                                if (notInstallItemList[k] == tmpItem) {
                                    var nullItem = ""
                                    notInstallItemList[k] = nullItem
                                }
                            }


                        } else {
                            for (var j = 0; j < selectedItemList.length; j++) {
                                if (selectedItemList[j] == tmpItem) {
                                    var nullVar = ""
                                    selectedItemList[j] = nullVar
                                }
                            }

                            findit = false
                            for (k = 0; k < notInstallItemList.length; k++) {
                                if (notInstallItemList[k] == tmpItem) {
                                    findit = true
                                }
                            }
                            if (findit == false) {
                                notInstallItemList.push(modelData.name)
                            }
                        }
                    }

                    }

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

                ProgressBar {
                    id: progressInfo
                    width:  appIcon.width
                    anchors.left: appIcon.left
                    //anchors.top:  appIcon.bottom
                    y: appIcon.y + appIcon.height - 15
                    maximumValue:  100
                    value : 0
                    visible: false
                }

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

                                for(var i = 0 ; i < pkModel.packages.length;i++) {
                                    if (pkModel.packages[i].name == modelData.name) {
                                        pkModel.packages[i].needInstall = "9" ;
                                        break;
                                    }
                                }

                            } else if (funcButton.text == qsTr("Update")) {
                                jadedBus.update(modelData.name)
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
                            jadedBus.runCmd(modelData.name)
                        } else if (index == 1) {
                            nameText.text = modelData.title
                            jadedBus.uninstall(modelData.name)

                            for(var i = 0 ; i < pkModel.packages.length;i++) {
                                if (pkModel.packages[i].name == modelData.name) {
                                    pkModel.packages[i].needInstall = "2" ;
                                    break;
                                }
                            }
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

    Rectangle {
        id: beforeBottonAct
        anchors.top: viewScroll.bottom
        width: parent.width
        height: 1
        color: "#e4ecd7"
    }

    CheckBox {
        id: allChecked
        anchors.left: parent.left
        anchors.leftMargin: 18
        anchors.top: beforeBottonAct.bottom
        anchors.topMargin: 10
        checked: false
        text: qsTr("install all selected items")
        onClicked: {
            selectedItemList = []
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
            if (checked) {
                bottonAct.enabled = true
            } else {
                bottonAct.enabled = false
            }
        }

    }

    PercentageButton {
        id: bottonAct
        text: qsTr("InstallAll")
        anchors.right: parent.right
        anchors.rightMargin: 17
        anchors.top: beforeBottonAct.bottom
        anchors.topMargin: 10
        enabled: allChecked.checked? true:false

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor

        onClicked: {
            var i =0
            for (i = 0; i < selectedItemList.length; i++) {
                if (selectedItemList[i] != "") {
                    //jadedBus.install(selectedItemList[i])
                }

                selectedItemList[i] = ""
            }

            for(i = 0 ; i < pkModel.packages.length;i++) {
                if (pkModel.packages[i].needInstall == "2") {
                    var find = false
                    for (var j = 0; j < notInstallItemList.length; j++) {
                        if (notInstallItemList[j] == pkModel.packages[i].name) {
                            find = true;
                            notInstallItemList[j] = "";
                            break;
                        }
                    }
                    if (find) {
                        continue;
                    }
                    jadedBus.install(pkModel.packages[i].name)
                    pkModel.packages[i].needInstall = "9"
                }
            }

            selectedItemList = []
            notInstallItemList = []

            allChecked.checked = false
            bottonAct.enabled = false

            if(typeof(pkListView.index) == 'undefined' ) {
                pkListView.pre_index = -2
            } else {
                pkListView.pre_index = pkListView.index
            }
        }
        }
    }

    MyLoader { id: myLoader; isVisible: packageByCategoryView.loading }
}
