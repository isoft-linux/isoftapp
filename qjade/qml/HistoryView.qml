/*
 * Copyright (C) 2016 fj <fujiang@i-soft.com.cn>
 */

import QtQuick 2.2
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0
import cn.com.isoft.qjade 2.0
import QtQml.Models 2.2
import "global.js" as Global
Rectangle {
    id: historyView
    width: parent.width
    height: parent.height

    property string category
    property string title
    property bool loading: true

    MypkgModel {
        id: pkModel
        category: historyView.category
        onPackageChanged: myLoader.visible = false;
        onError: {
            if (Global.isNetworkAvailable == false) {
                //myLoader.visible = true;
                historyView.enabled = false
                networkErrorText.isVisible = true;
            } else {
                myLoader.visible = false;
                myResultText.isVisible = true;
            }
        }
    }

    Rectangle {
        id: titleRegion
        width: parent.width; height: 43
        color: "#f5f5f5"

        Text {
            id: countText
            text: qsTr("%1 total").arg(pkModel.packages.length)
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
        height: parent.height - titleRegion.height - 30
        anchors.top: titleRegion.bottom
        flickableItem.interactive: true

        ListView {
            id: pkListView
            model: pkModel.packages
            anchors.fill: parent

            delegate: Rectangle {
                id: pkRect
                width: parent.width
                height: 60
                color: index % 2 == 0 ? "white" : "#f5f5f5"

                JadedBus {
                    id: jadedBus

                    property bool isError: false
                    property string info: "UnknownInfo"

                    Component.onCompleted: {
                        jadedBus.info = jadedBus.getInfo(modelData.name)

                        var item = pkListView.contentItem.children[index]
                        if (typeof(item) != 'undefined' ) {
                            item.state = "checked"
                        }
                        if (jadedBus.info == "UnknownInfo") {
                            pkRect.visible = false;
                            pkRect.height = 0;
                            pkModel.removeAt(index);

                        } else if (jadedBus.info == "InfoRunning") {
                            actCombox.visible = false;
                            funcButton.visible = false;
                            infoText.visible = true;
                            infoText.text = qsTr("Running");
                        } else if (jadedBus.info == "InfoWaiting") {
                            actCombox.visible = false
                            funcButton.visible = false
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
                        if (name == modelData.name) {
                            if (detail == "lastest")
                                nameText.text = modelData.title + " (" + qsTr("Is lastest") + ")"
                            else
                            nameText.text = modelData.title + " (" + qsTr("Error") + ")"
                            funcButton.visible = true;
                            infoText.visible = false;
                        }

                        if (detail == "offline") {
                            myLoader.visible = true;
                            Global.isNetworkAvailable = false
                            historyView.enabled = false
                        } else if (detail == "online") {
                            myLoader.visible = false
                            Global.isNetworkAvailable = true
                            historyView.enabled = true
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
                                statusText.text = qsTr("Installed")

                                progressInfo.visible = false;
                            } else {
                                funcButton.visible = true
                                infoText.visible = false
                                actCombox.visible = false
                                statusText.text = qsTr("Uninstalled")

                                progressInfo.visible = false;
                            }
                        }
                    }
                }

                Image {
                    id: appIcon
                    source: modelData.icon
                    asynchronous: true
                    sourceSize.width: 48; sourceSize.height: 48
                    anchors.left: parent.left
                    anchors.leftMargin: 17
                    anchors.verticalCenter: parent.verticalCenter

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            stackView.push({item: Qt.resolvedUrl("PackageInfoView.qml"),
                            properties: {packageName:modelData.name,packageTitle: modelData.title,
                                         stackView: stackView}})
                        }
                        cursorShape: Qt.PointingHandCursor
                    }
                }

                ProgressBar {
                    id: progressInfo
                    width:  appIcon.width
                    anchors.left: appIcon.left
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
                        cursorShape: Qt.PointingHandCursor
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
                    id: statusText
                    text: modelData.size == "1" ? qsTr("Installed") :qsTr("Uninstalled")
                    font.pixelSize: 11
                    color: "#979797"
                    anchors.right: funcButton.left
                    anchors.rightMargin: parent.width/10*3
                    anchors.verticalCenter: parent.verticalCenter
                    visible: true
                }

                PercentageButton {
                    id: funcButton
                    text: qsTr("Install")
                    anchors.right: parent.right
                    anchors.rightMargin: 17
                    anchors.verticalCenter: parent.verticalCenter
                    //MouseArea {
                    //    anchors.fill: parent
                    //    cursorShape: Qt.PointingHandCursor
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
                    //}
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
                    model: ListModel {
                             id: listmodel
                             ListElement { text: qsTr("Open")}
                             ListElement { text: qsTr("Uninstall")}
                             ListElement { text: qsTr("Upgrade")}
                             ListElement { text: qsTr("SelectOp")}
                    }
                    visible: false
                    currentIndex: 3

                    onPressedChanged:     {
                        if(listmodel.count == 4) {
                            listmodel.remove(3)
                        }
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
                        //currentIndex = 3
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

    MyLoader { id: myLoader; isVisible: false }

    MyResultText { id: myResultText; result: qsTr("No Installed Available") }
    MyResultText { id: networkErrorText; result: qsTr("Network is Unavailable") }
}
