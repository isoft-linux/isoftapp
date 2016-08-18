/*                                                                                 
 * Copyright (C) 2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

import QtQuick 2.2
import QtQuick.Controls 1.0                                                        
import QtQuick.Controls.Styles 1.0                                                 
import cn.com.isoft.qjade 2.0

Rectangle {
    id: searchView
    width: parent.width; height: parent.height

    property string keyword
    property StackView stackView
    property Rectangle rect

    SearchModel {
        id: searchModel
        Component.onCompleted: {
            searchModel.search(searchView.keyword);
        }
        onError: {
            myResultText.visible = searchListModel.count == 0 ? true : false;
        }
        onSearchResultChanged: {
            for (var i = 0; i < searchModel.searchResult.length; i++) {
                searchListModel.append({"icon": searchModel.searchResult[i].icon,
                "pkName": searchModel.searchResult[i].name, 
                "description": searchModel.searchResult[i].description,
                "size": searchModel.searchResult[i].size,
                "title": searchModel.searchResult[i].title});
            }
            searchListView.visible = searchListModel.count == 0 ? false : true;
            myResultText.visible = searchListModel.count == 0 ? true : false;
        }
    }

    JadedBus {                                                                     
        id: searchJadedBus
        Component.onCompleted: {
            searchJadedBus.search(searchView.keyword)
        }
        onSearchError: {
            myLoader.visible = false;
            myResultText.visible = searchListModel.count == 0 ? true : false;
        }
        onSearchChanged: {                                                       
            myLoader.visible = false;                                               
            for (var i = 0; i < searchJadedBus.searchs.length; i++) {
                searchListModel.append({"icon": searchJadedBus.searchs[i].icon, 
                "pkName": searchJadedBus.searchs[i].name, 
                "description": searchJadedBus.searchs[i].description,
                "size": searchJadedBus.searchs[i].size});
            }
            searchListView.visible = searchListModel.count == 0 ? false : true;
            myResultText.visible = searchListModel.count == 0 ? true : false;
        }                                                                          
    }

    Rectangle {                                                                    
        id: titleRegion                                                            
        width: parent.width; height: 40
        color: "#f5f5f5"
        anchors.top: searchView.top
        anchors.topMargin: 2
        anchors.left: parent.left

        Rectangle {
            id: returnRegion
            width: 80;height: 40
            color: "#f5f5f5"
            anchors.left: titleRegion.left
            anchors.leftMargin: 2
            border.width: 1
            border.color: "#c9cbce"

            Image {
                id: navImage
                anchors.top: returnRegion.top
                anchors.left: returnRegion.left
                anchors.leftMargin: 2
                source: "../images/navigation_previous_item.png"
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                text:qsTr("Return")
                anchors.top: returnRegion.top
                anchors.left: navImage.right
                anchors.leftMargin: -2
                anchors.topMargin: 10
                font.pixelSize: 20
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    searchView.rect.name = "search_btn"
                    searchView.stackView.pop()
                }
            }
        }

        Rectangle {
            id: sizeRegion
            width: (parent.width - 50)
            height: 40
            color: "#f5f5f5"
            anchors.left: returnRegion.right
            anchors.right: parent.right
            anchors.leftMargin: 3
            anchors.rightMargin: 3
            border.width: 1
            border.color: "#c9cbce"

            Rectangle {
                id: leftRect
                anchors.left: parent.left
                anchors.topMargin: 20
                anchors.verticalCenter: parent.verticalCenter
                width: (parent.width -60)
                height: parent.height
                color: "transparent"

                Text {
                    id: countText
                    text: qsTr("%1 total searched").arg(searchListView.count)
                    font.pixelSize: 15
                    anchors.left: leftRect.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    id: sizeText
                    text: qsTr("Size")
                    font.pixelSize: 15
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width/2 - 10
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    ListModel {
        id: searchListModel
        Component.onCompleted: {
            searchListModel.clear();
        }

        ListElement { icon: ""; pkName: ""; description: ""; size: "" ;title: ""}
    }

    ScrollView {
        width: parent.width
        height: parent.height - titleRegion.height // - secondTitleRegion.height
        anchors.top: titleRegion.bottom
        anchors.left: titleRegion.left
        flickableItem.interactive: true

        ListView {
            id: searchListView
            model: searchListModel
            anchors.fill: parent

            delegate: Rectangle {
                id: itemRect
                width: parent.width
                height: 60
                color: index % 2 == 0 ? "white" : "#f5f5f5"

                Image {
                    id: appIcon
                    source: icon
                    sourceSize.width: 48; sourceSize.height: 48
                    anchors.left: parent.left
                    anchors.leftMargin: 7
                    anchors.verticalCenter: parent.verticalCenter

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            stackView.push({item: Qt.resolvedUrl("PackageInfoView.qml"),
                            properties: {packageTitle: title,
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

                Text {
                    id: nameText
                    text: pkName
                    anchors.left: appIcon.right
                    anchors.leftMargin: 8
                    anchors.top: parent.top
                    anchors.topMargin: 6
                    font.pixelSize: 14
                }

                Text {
                    text: description
                    width: parent.width/3 //parent.width - appIcon.width - nameText.width - funcButton.width
                    anchors.left: nameText.left
                    anchors.top: nameText.bottom
                    anchors.topMargin: 8                                           
                    anchors.rightMargin: 8
                    wrapMode: Text.WordWrap                                        
                    font.pixelSize: 11                                             
                    color: "#b7b7b7"
                }

                // 软件包大小 xxM
                Text {
                    //id: sizeText
                    text: size //modelData.size
                    font.pixelSize: 11
                    color: "#979797"
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width/2
                    anchors.verticalCenter: parent.verticalCenter
                    visible: true
                }

                JadedBus {
                    id: jadedBus

                    property bool isError: false
                    property string info: "UnknownInfo"

                    Component.onCompleted: {
                        jadedBus.info = jadedBus.getInfo(pkName)
                        if (jadedBus.info == "UnknownInfo") {
                            itemRect.visible = false; // 不显示此软件包的信息
                            itemRect.height = 0;
                        } else if (jadedBus.info == "InfoRunning") {
                            funcButton.visible = false;
                            infoText.visible = true;
                            infoText.text = qsTr("Running"); // 运行中
                        } else if (jadedBus.info == "InfoWaiting") {
                            funcButton.visible = false
                            infoText.visible = true
                            infoText.text = qsTr("Waiting") // 等待
                        } else if (jadedBus.info == "InfoInstalled") {
                            funcButton.visible = false
                            infoText.visible = false // 已安装最新版本
                            actCombox.visible = true // 已经安装，则显示下拉框
                        } else if (jadedBus.info == "InfoUpdatable") {
                            funcButton.text = qsTr("Update") // 软件升级
                        }
                    }

                    onErrored: {
                        if (name == pkName) {
                            if (detail == "lastest")
                                nameText.text = name + " (" + qsTr("Is lastest") + ")"
                            else
                            nameText.text = name + " (" + qsTr("Error") + ")"
                            funcButton.visible = true;
                            infoText.visible = false;
                        }
                    }

                    onTaskChanged: {
                        if (name == pkName) {
                            jadedBus.info = jadedBus.getInfo(name)
                            if (jadedBus.info == "InfoInstalled") {
                                funcButton.visible = false
                                infoText.visible = false // 已安装最新版本
                                actCombox.visible = true // 已经安装，则显示下拉框
                                progressInfo.visible = false;
                            } else {
                                funcButton.visible = true
                                infoText.visible = false
                                actCombox.visible = false
                                progressInfo.visible = false;
                            }
                        }
                    }

                    // 安装/卸载时 接收到的进度
                    onPerChanged: {
                        if (name == pkName) {
                            progressInfo.value = perCent;
                            if (perCent != 100) {
                                funcButton.visible = false
                                infoText.visible = true
                                infoText.text = qsTr("Waiting")
                                progressInfo.visible = true

                            }

                        }
                    }
                }

                PercentageButton {
                    id: funcButton
                    text: qsTr("Install")
                    anchors.right: parent.right
                    anchors.rightMargin: 17
                    anchors.verticalCenter: parent.verticalCenter
                    onClicked: {
                        if (funcButton.text == qsTr("Install")) {                  
                            jadedBus.install(pkName)                       
                        } else if (funcButton.text == qsTr("Update")) {            
                            jadedBus.update(pkName)                        
                        }                                                          
                        funcButton.visible = false                                 
                        infoText.visible = true                                    
                        infoText.text = qsTr("Waiting")
                    }
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
                            jadedBus.runCmd(pkName)
                        } else if (index == 1) {
                            nameText.text = pkName
                            jadedBus.uninstall(pkName)
                        } else if (index == 2) {
                            jadedBus.update(pkName)
                        }
                        currentIndex = 3
                    }
                }

                Text {
                    id: infoText
                    text: qsTr("Installed")
                    anchors.top: funcButton.top
                    anchors.left: funcButton.left
                    visible: false
                }

            }
        }

        style: MyScrollViewStyle {}
    }

    MyLoader { id: myLoader }

    MyResultText { id: myResultText; result: qsTr("No App Available") }
}
