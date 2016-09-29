/*                                                                                 
 * Copyright (C) 2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

import QtQuick 2.2
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0
import cn.com.isoft.qjade 2.0
import "global.js" as Global

Rectangle {
    id: updateView
    width: parent.width
    height: parent.height

    property int count: updateListView.count

    property var selectedItemList:[]

    Component.onCompleted: {
        if (Global.isNetworkAvailable == false) {
            myResultText.result = qsTr("Network is Unavailable");
            myResultText.visible = true;
            myLoader.visible = false;
        }
    }

    JadedBus {
        id: updateJadedBus
        Component.onCompleted: {
            updateJadedBus.getUpdate()
        }
        onGetUpdateError: {
            myLoader.visible = false;
            myResultText.isVisible = true;
        }
        onUpdateChanged: {
            if (Global.isNetworkAvailable) {
                myLoader.visible = false;
                updateListView.model = updateJadedBus.updates;
                updateListView.visible = count == 0 ? false : true;                     
                myResultText.visible = count == 0 ? true : false;
                updateText.text = qsTr("Update") + ": " + qsTr("%1 total").arg(count);
            }
        }
    }

    Rectangle {                                                                    
        id: titleRegion                                                            
        width: parent.width; height: 65
        color: "#f5f5f5"                                                                                                                         
        Text {                                                      
            id: updateText               
            text: qsTr("Update") + ": " + qsTr("%1 total").arg(updateView.count)
            font.pixelSize: 14                                                     
            anchors.left: parent.left
            anchors.leftMargin: 70
            anchors.top: parent.top
            anchors.topMargin:10
        }                                                                          

        Rectangle {
            id: secondTitleRegion
            anchors.top: updateText.bottom
            anchors.topMargin: 10
            width: parent.width; height: 30
            color: "lightgray"

            Text {
                id: nameText
                text: qsTr("Name")
                font.pixelSize: 14
                anchors.left: parent.left
                anchors.leftMargin: parent.width/9 - 35
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                id: sizeText
                text: qsTr("Size")
                font.pixelSize: 14
                anchors.left: parent.left
                anchors.leftMargin: parent.width/2
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                id: versionText
                text: qsTr("Version")
                font.pixelSize: 14
                anchors.left: parent.left
                anchors.leftMargin: parent.width/10*7
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                id: actionText
                text: qsTr("Action")
                font.pixelSize: 14
                anchors.left: parent.left
                anchors.leftMargin: parent.width/10*9 + 20
                anchors.verticalCenter: parent.verticalCenter
            }
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
            id: updateListView
            anchors.fill: parent

            delegate: Rectangle {
                id: updateRect
                objectName: "rectItem"
                width: parent.width
                height: 60
                color: index % 2 == 0 ? "white" : "#f5f5f5"
                signal checkItem(bool checked)

                onCheckItem: {
                    appCheckBox.checked = checked
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
                        if (appCheckBox.checked) {
                            for (var i = 0; i < selectedItemList.length; i++) {
                                if (selectedItemList[i] == tmpItem) {
                                    nameText.text = modelData.name
                                    findit = true
                                }
                            }
                            if (findit == false) {
                                selectedItemList.push(modelData.name)
                                nameText.text = modelData.name
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

                Image {
                    id: appIcon
                    source: modelData.icon
                    width: 48; height: 48
                    anchors.left: appCheckBox.right
                    anchors.verticalCenter: parent.verticalCenter
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
                    text: modelData.name
                    anchors.left: appIcon.right
                    anchors.leftMargin: 8
                    anchors.top: parent.top
                    anchors.topMargin: 6
                    font.pixelSize: 14
                }

                Text {
                    text: modelData.description
                    width: parent.width/2
                    anchors.left: nameText.left
                    anchors.top: nameText.bottom
                    anchors.topMargin: 8
                    wrapMode: Text.WordWrap
                    font.pixelSize: 11
                    color: "#b7b7b7"
                }

                Text {
                    id: sizeContentText
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width/2 - sizeContentText.width/6
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData.size
                }

                Text {
                    id: curVersionText
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width/10*7 - versionText.width/2
                    anchors.top: parent.top
                    anchors.topMargin: 6
                    font.pixelSize: 14

                    text: qsTr("Current version:") + modelData.version
                }
                Text {
                    id: newVersionText
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width/10*7 - versionText.width/2
                    anchors.top: curVersionText.bottom
                    anchors.topMargin: 8
                    font.pixelSize: 14

                    text: qsTr("Latest version:") + modelData.datetime
                }

                JadedBus {
                    id: jadedBus
                
                    property bool isError: false
                    property string info: "UnknownInfo"

                    Component.onCompleted: {
                        jadedBus.info = jadedBus.getInfo(modelData.name)
                        if (jadedBus.info == "InfoWaiting") {
                            updateButton.visible = false;
                            infoText.visible = true
                            infoText.text = qsTr("Waiting")
                        }
                    }
                    onErrored: {
                        if (name == modelData.name) {
                            nameText.text = modelData.name + " (" + qsTr("Error") + ")";
                            updateButton.visible = true;
                            infoText.visible = false;
                            isError = true
                        }
                    }

                    onPerChanged: {
                        if (name == modelData.name) {
                            updateButton.visible = false
                            progressInfo.value = perCent;
                            if (perCent != 100) {
                                infoText.visible = true
                                infoText.text = qsTr("Waiting")
                                progressInfo.visible = true
                            }

                        }
                    }
                    onTaskChanged: {                                               
                        if (name == modelData.name) {
                            if (isError) {
                                isError = false
                                appCheckBox.checked = false
                            } else {

                            updateButton.visible = false
                            infoText.visible = true
                            infoText.text = qsTr("Updated")
                            updateRect.visible = false
                            updateRect.height = 0
                            updateView.count--
                            updateText.text = qsTr("Update") + ": " + qsTr("%1 total").arg(updateView.count)
                            }
                        }
                    }
                }

                PercentageButton {
                    id: updateButton
                    text: qsTr("Update")
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter

                    //MouseArea {
                    //    anchors.fill: parent
                    //    cursorShape: Qt.PointingHandCursor
                    //    hoverEnabled: true

                        onClicked: {
                            jadedBus.update(modelData.name)
                            updateButton.visible = false
                            infoText.visible = true
                            infoText.text = qsTr("Waiting")
                        }
                    //}
                }

                Text {
                    id: infoText
                    visible: false
                    anchors.top: updateButton.top
                    anchors.left: updateButton.left
                    //anchors.verticalCenter: parent.verticalCenter
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
        text: qsTr("Update all selected items")

        onClicked: {
            if (checked) {
                bottonAct.enabled = true
            } else {
                bottonAct.enabled = false
            }

            delete selectedItemList
            for (var i = 0; i < updateListView.count; i++) {
                var item = updateListView.contentItem.children[i]
                if (typeof(item) == 'undefined' ||
                    item.objectName != "rectItem") {
                    continue;
                }

                item.checkItem(checked)
            }
        }
    }

    PercentageButton {
        id: bottonAct
        text: qsTr("UpdateAll")
        anchors.right: parent.right
        anchors.rightMargin: 17
        anchors.top: beforeBottonAct.bottom
        anchors.topMargin: 10
        enabled: allChecked.checked? true:false
        //MouseArea {
        //    anchors.fill: parent
        //    cursorShape: Qt.PointingHandCursor

        onClicked: {
            for (var i = 0; i < selectedItemList.length; i++) {
                if (selectedItemList[i] != "") {
                    enabled = false
                    jadedBus.update(selectedItemList[i])
                    selectedItemList[i] = ""
                }
            }

            allChecked.checked = false
            bottonAct.enabled = false
        }
        //}
    }

    MyLoader { id: myLoader }

    MyResultText { id: myResultText; result: qsTr("No Update Available") }
}
