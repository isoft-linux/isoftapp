/*                                                                                 
 * Copyright (C) 2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
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
    property var selectedItemList:[]
    property var notInstallItemList:[]
    property int pre_index:-1

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
            anchors.leftMargin: 10                                                 
            //anchors.verticalCenter: parent.verticalCenter
            anchors.top: parent.top
            anchors.topMargin: 10
        }                                                                          

        // 安装时间 大小 类别
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
                anchors.leftMargin: parent.width/8
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
                anchors.leftMargin: parent.width/10*9
                anchors.verticalCenter: parent.verticalCenter
            }



        }
                                                                                   
        Rectangle { width: parent.width; height: 1; color: "#e4ecd7"; }
    }


    ScrollView {
        id: viewScroll
        width: parent.width
        height: parent.height - titleRegion.height - 60 // 60 for button on bottom
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

                // 批处理单选按钮 allChecked 按下时发送信号
                signal checkItem(bool checked,int i)

                onCheckItem: {
                    appCheckBox.checked = checked
                }

                JadedBus {
                    id: jadedBus

                    property string info: "UnknownInfo"

                    Component.onCompleted: {
                        jadedBus.info = jadedBus.getInfo(modelData.name)
                        if (jadedBus.info == "InfoWaiting") {
                            removeButton.visible = false
                            infoText.visible = true
                            infoText.text = qsTr("Waiting")
                        }

                        // 上下滚动时，自动去掉选择状态
                        if (pre_index != uninstallListView.index) {
                            if (allChecked.checked) {
                                allChecked.checked = false
                                allChecked.pressed = false

                                delete selectedItemList
                                delete notInstallItemList
                                for (var i = 0; i < uninstallListView.contentItem.children.length; ++i) {
                                    var item = uninstallListView.contentItem.children[i]
                                    if (typeof(item) == 'undefined' ) {
                                        continue;
                                    }
                                    else if(item.objectName != "rectItem") {
                                        continue;
                                    }
                                    item.checkItem(false,1)
                                }
                            }
                            bottonAct.enabled = false
                        }
                    }
                    onErrored: {
                        if (name == modelData.name) {
                            nameText.text = modelData.version + " (" + qsTr("Error") + ")";
                            removeButton.visible = true;
                            infoText.visible = false;
                        }
                    }

                    // 安装/卸载时 接收到的进度
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
                            for(var i = 0 ; i < uninstallJadedBus.installed.length;i++) {
                                if (uninstallJadedBus.installed[i].id != "unknown") {
                                    // inited in jadeddbus.cpp
                                    newCount ++
                                }
                            }
                            countText.text = qsTr("Uninstall") + ": " + qsTr("%1 total").arg(newCount)
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
                        var x = 9999
                        var k = 0
                        if (appCheckBox.checked) {
                            for (var i = 0; i < selectedItemList.length; i++) {
                                if (selectedItemList[i] == tmpItem) {
                                    nameText.text = modelData.version
                                    findit = true
                                }
                                if (selectedItemList[i] == "") {
                                    x = i
                                }
                            }
                            if (findit == false) {
                                if (x < 9999) {
                                    selectedItemList[x] = modelData.name
                                } else {
                                    selectedItemList.push(modelData.name)
                                }
                                nameText.text = modelData.version
                            }

                            // 选中时，从 notInstallItemList 中去掉
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

                            // 未选中时，插入到表 notInstallItemList
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

                    //text: qsTr("example")
                    }

                Image {
                    id: appIcon
                    source: modelData.icon
                    anchors.left: appCheckBox.right
                    //anchors.leftMargin: 7
                    anchors.verticalCenter: parent.verticalCenter
                    sourceSize.width: 48; sourceSize.height: 48
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
                    text: modelData.version // version in jadedbus.cpp is title
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
                    anchors.leftMargin: parent.width/8*6 // - cateText.width/6
                    anchors.verticalCenter: parent.verticalCenter
                }

                PercentageButton {
                    id: removeButton
                    text: qsTr("Uninstall")
                    textColor: "black"
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            visible = false
                            jadedBus.uninstall(modelData.name) //modelData.id
                            infoText.visible = true
                            infoText.text = qsTr("Waiting")
                            removeButton.visible = false

                            for(var i = 0 ; i < uninstallJadedBus.installed.length;i++) {
                                if (modelData.name == uninstallJadedBus.installed[i].name) {
                                    uninstallJadedBus.installed[i].id = "unknown"
                                }
                            }

                            // 未选中时，插入到表 notInstallItemList
                            var findit = false
                            for (var k = 0; k < notInstallItemList.length; k++) {
                                if (notInstallItemList[k] == modelData.name) {
                                    findit = true
                                }
                            }
                            if (findit == false) {
                                notInstallItemList.push(modelData.name)
                            }
                        }

                    }
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
        text: qsTr("uninstall all selected items")

        onClicked: {
            if (checked) {
                bottonAct.enabled = true
            } else {
                bottonAct.enabled = false
            }

            delete selectedItemList
            //bottonAct.text = "total:" + uninstallListView.count
            for (var i = 0; i < uninstallListView.count; i++) {
                var item = uninstallListView.contentItem.children[i]
                if (typeof(item) == 'undefined' ||
                    item.objectName != "rectItem") {
                    continue;
                }

                item.checkItem(checked,i)
            }
        }
    }


    // 多选，一键卸载
    PercentageButton {
        id: bottonAct
        text: qsTr("UnstallAll")
        anchors.right: parent.right
        anchors.rightMargin: 17
        anchors.top: beforeBottonAct.bottom
        anchors.topMargin: 10
        // 批处理单选按钮 被按下了，此按钮才会可用
        enabled: allChecked.checked? true:false
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor

        onClicked: {
            // 遍历每个被选中的软件包，作相同的动作
            for (var i = 0; i < selectedItemList.length; i++) {
                if (selectedItemList[i] != "") {
                    //jadedBus.uninstall(selectedItemList[i])
                    selectedItemList[i] = ""
                }
            }

            // 遍历所有软件包，去掉没有被选中的记录；
            for(i = 0 ; i < uninstallJadedBus.installed.length;i++) {
                var find = false
                for (var j = 0; j < notInstallItemList.length; j++) {
                    if (notInstallItemList[j] == uninstallJadedBus.installed[i].name) {
                        find = true;
                        break;
                    }
                }
                if (find == true ) {
                    continue;
                }
                jadedBus.uninstall(uninstallJadedBus.installed[i].name )
                uninstallJadedBus.installed[i].id = "unknown"
            }

            allChecked.checked = false
            bottonAct.enabled = false

            if(typeof(uninstallListView.index) == 'undefined' ) {
                pre_index = -2
            } else {
                pre_index = uninstallListView.index
            }
        }
        }
    }

    MyLoader { id: myLoader }

    MyResultText { id: myResultText; result: qsTr("No Installed Available") }
}
