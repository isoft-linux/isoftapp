/*                                                                                 
 * Copyright (C) 2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 *               2016 fujiang <fujiang.zhu@i-soft.com.cn>
 */

import QtQuick 2.2
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0
import cn.com.isoft.qjade 2.0

Rectangle {
    id: uninstallView
    width: parent.width
    height: parent.height

    property int count: uninstallListView.count

    ListModel {
        id: myListModel
        ListElement {
            name: ""
            checked: false
            installed: false
        }
    }

    JadedBus {                                                                     
        id: uninstallJadedBus
        Component.onCompleted: {                                                   
            uninstallJadedBus.getInstalled();
        }                                                                          
        onGetInstalledError: {
            myLoader.visible = false;
            myResultText.isVisible = true;
        }
        onInstalledChanged: {                                                         
            myLoader.visible = false;
            uninstallListView.model = uninstallJadedBus.installed;
            uninstallListView.visible = count == 0 ? false : true;
            allChecked.enabled = count == 0 ? false : true;

            for(var i = 0; i < uninstallJadedBus.installed.length; i++) {
                myListModel.append({name: uninstallJadedBus.installed[i].name,
                                    checked: false,
                                   installed:true});
            }
        }                                                                          
    }

    Rectangle {                                                                    
        id: titleRegion                                                            
        width: parent.width; height: 70
        color: "#f5f5f5"                                                           
                                                                                   
        Text {                                                                  
            id: countText  
            text: qsTr("Uninstall") + ": " + qsTr("%1 total").arg(uninstallView.count)
            font.pixelSize: 14                                                     
            anchors.left: parent.left                                              
            anchors.leftMargin: 70
            //anchors.verticalCenter: parent.verticalCenter
            anchors.top: parent.top
            anchors.topMargin: 10
        }                                                                          

        Rectangle {
            id: secondTitleRegion
            anchors.top: countText.bottom
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
                id: datetimeText
                text: qsTr("Datetime")
                font.pixelSize: 14
                anchors.left: parent.left
                anchors.leftMargin: parent.width/2 - datetimeText.width
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                id: sizeText
                text: qsTr("Size")
                font.pixelSize: 14
                anchors.left: parent.left
                anchors.leftMargin: parent.width/8*5
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                id: cateText
                text: qsTr("Category")
                font.pixelSize: 14
                anchors.left: parent.left
                anchors.leftMargin: parent.width/8*6
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
                                                                                   
        Rectangle { width: parent.width; height: 1; color: "#e4ecd7"; }
    }


    ScrollView {
        id: viewScroll
        width: parent.width
        height: parent.height - titleRegion.height - 60
        anchors.top: titleRegion.bottom
        flickableItem.interactive: true

        ListView {
            id: uninstallListView
            anchors.fill: parent

            delegate: Rectangle {
                id: uninstallRect
                objectName: "rectItem"
                width: parent.width
                height: 60
                color: index % 2 == 0 ? "white" : "#f5f5f5"

                signal checkItem(bool checked,int i)

                onCheckItem: {
                    appCheckBox.checked = checked
                }

                JadedBus {
                    id: jadedBus

                    property string info: "UnknownInfo"

                    Component.onCompleted: {
                        jadedBus.info = jadedBus.getInfo(modelData.name)
                        for(var i = 0; i < myListModel.count; i++) {
                            if (myListModel.get(i).name  == modelData.name) {
                                if (myListModel.get(i).checked ) {
                                    appCheckBox.checked = true;
                                } else {
                                    appCheckBox.checked = false;
                                }
                                break;
                            }
                        }

                        if (jadedBus.info == "InfoWaiting") {
                            removeButton.visible = false
                            infoText.visible = true
                            infoText.text = qsTr("Waiting")
                        } else if (jadedBus.info == "InfoAvailable"){
                            uninstallRect.visible = false
                            uninstallRect.height = 0
                        }
                    }
                    onErrored: {
                        if (name == modelData.name) {
                            nameText.text = modelData.version + " (" + qsTr("Error") + ")";
                            removeButton.visible = true;
                            infoText.visible = false;
                        }
                    }

                    onPerChanged: {
                        if (name == modelData.name) {
                            removeButton.visible = false
                            progressInfo.value = perCent;
                            if (perCent != 100) {
                                //funcButton.visible = false
                                infoText.visible = true
                                infoText.text = qsTr("Waiting")
                                progressInfo.visible = true

                            }

                        }
                    }
                    onTaskChanged: {
                        if (name == modelData.name) {
                            removeButton.visible = false
                            infoText.visible = true
                            infoText.text = qsTr("Uninstalled")
                            uninstallRect.visible = false
                            uninstallRect.height = 0
                            uninstallView.count--
                            var newCount = 0

                            for(var i = 0; i < myListModel.count; i++) {
                                if (myListModel.get(i).installed) {
                                    newCount ++
                                }
                            }
                            if (appCheckBox.checked) {
                                appCheckBox.checked = false
                            }
                            countText.text = qsTr("Uninstall") + ": " + qsTr("%1 total").arg(newCount)
                            allChecked.enabled = newCount == 0 ? false : true;
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
                        if (appCheckBox.checked) {
                            for(var i = 0; i < myListModel.count; i++) {
                                if (myListModel.get(i).name  == modelData.name) {
                                    myListModel.setProperty(i, "checked", true);
                                    bottonAct.enabled = true
                                    break;
                                }
                            }
                        } else {
                            var hasChecked = false;
                            for(var i = 0; i < myListModel.count; i++) {
                                if (myListModel.get(i).name  == modelData.name) {
                                    myListModel.setProperty(i, "checked", false);
                                    //break;
                                }
                                if (myListModel.get(i).installed && myListModel.get(i).checked) {
                                    hasChecked = true;
                                }
                            }
                            if (!hasChecked) {
                                bottonAct.enabled = false
                            }
                        }
                    }
                }

                Image {
                    id: appIcon
                    source: modelData.icon
                    anchors.left: appCheckBox.right
                    //anchors.leftMargin: 7
                    anchors.verticalCenter: parent.verticalCenter
                    sourceSize.width: 48; sourceSize.height: 48
                    // offline-> online:meet error:[QML Image: Network access is disabled]
                    //onStatusChanged: if (appIcon.status == Image.Error) appIcon.source = "/var/cache/isoftapp/qjade/vlc.png"
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
                    text: modelData.version
                    anchors.left: appIcon.right
                    anchors.leftMargin: 8
                    anchors.top: parent.top
                    anchors.topMargin: 6
                    font.pixelSize: 14
                }

                Text {
                    text: modelData.description
                    width: parent.width - appIcon.width - nameText.width - removeButton.width
                    anchors.left: nameText.left
                    anchors.top: nameText.bottom
                    anchors.topMargin: 8
                    wrapMode: Text.WordWrap
                    font.pixelSize: 11
                    color: "#b7b7b7"
                }


                Text {
                    id: datetimeInfoText
                    text: modelData.datetime
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width/2 - datetimeInfoText.width/5*3
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    id: sizeInfoText
                    text: modelData.size
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width/8*5 - sizeInfoText.width/6
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    id: categoryText
                    text: modelData.category
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width/8*6
                    anchors.verticalCenter: parent.verticalCenter
                }

                PercentageButton {
                    id: removeButton
                    text: qsTr("Uninstall")
                    textColor: "black"
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    //MouseArea {
                    //    anchors.fill: parent
                    //    cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            visible = false
                            jadedBus.uninstall(modelData.name)
                            infoText.visible = true
                            infoText.text = qsTr("Waiting")
                            removeButton.visible = false

                            for(var i = 0 ; i < uninstallJadedBus.installed.length;i++) {
                                if (modelData.name == uninstallJadedBus.installed[i].name) {
                                    uninstallJadedBus.installed[i].id = "unknown"
                                }
                            }

                            for(var i = 0; i < myListModel.count; i++) {
                                if (myListModel.get(i).name  == modelData.name) {
                                    myListModel.setProperty(i, "checked", false);
                                    myListModel.setProperty(i, "installed", false);
                                    break;
                                }
                            }
                        }

                    //}
                }

                Text {
                    id: infoText
                    text: qsTr("Uninstalling")
                    visible: false
                    anchors.top: removeButton.top
                    anchors.left: removeButton.left
                    font.pixelSize: 11
                    color: "#979797"
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
        text: qsTr("uninstall all selected items")
        enabled: false

        onClicked: {
            if (checked) {
                bottonAct.enabled = true
            } else {
                bottonAct.enabled = false
            }
            for(var i = 0; i < myListModel.count; i++) {
                if (myListModel.get(i).installed) {
                    if (allChecked.checked ) {
                        myListModel.setProperty(i, "checked", true);
                    } else {
                        myListModel.setProperty(i, "checked", false);
                    }
                }
            }

            for (var i = 0; i < uninstallListView.contentItem.children.length; ++i) {
                var item = uninstallListView.contentItem.children[i]
                if (typeof(item) == 'undefined' ||
                    item.objectName != "rectItem") {
                    continue;
                }
                item.checkItem(checked,i)
            }
        }
    }

    PercentageButton {
        id: bottonAct
        text: qsTr("UnstallAll")
        anchors.right: parent.right
        anchors.rightMargin: 17
        anchors.top: beforeBottonAct.bottom
        anchors.topMargin: 10
        enabled: allChecked.checked? true:false
        //MouseArea {
        //    anchors.fill: parent
        //    cursorShape: Qt.PointingHandCursor

        onClicked: {
            for(var i = 0; i < myListModel.count; i++) {
                if (myListModel.get(i).installed && myListModel.get(i).checked) {
                    jadedBus.uninstall(myListModel.get(i).name)
                    myListModel.setProperty(i, "checked", false);
                    myListModel.setProperty(i, "installed", false);
                }
            }

            allChecked.checked = false
            bottonAct.enabled = false

            for (var i = 0; i < uninstallListView.count; i++) {
                var item = uninstallListView.contentItem.children[i]
                if (typeof(item) == 'undefined' ||
                    item.objectName != "rectItem") {
                    continue;
                }
                item.checkItem(checked,i)
            }
        }
        //}
    }

    MyLoader { id: myLoader }

    MyResultText { id: myResultText; result: qsTr("No Installed Available") }
}
