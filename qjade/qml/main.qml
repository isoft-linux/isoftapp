/*                                                                              
 * Copyright (C) 2014 - 2016 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 *               2014 Jeff Bai <jeffbaichina@members.fsf.org>
 *               2016 fujiang <fujiang.zhu@i-soft.com.cn>
 */

import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Dialogs 1.0
import cn.com.isoft.qjade 2.0

ApplicationWindow {
    id: rootWindow
    width: 920; height: 680
    minimumWidth: 920; minimumHeight: 680
    flags: Qt.FramelessWindowHint | Qt.CustomizeWindowHint
    x: (Screen.width - width) / 2; y: (Screen.height - height) / 2
    color: "transparent"

    //-------------------------------------------------------------------------
    // TODO: Brand information
    //-------------------------------------------------------------------------
    Brand { id: brand }

    //-------------------------------------------------------------------------
    // TODO: Application information
    //-------------------------------------------------------------------------
    AppInfo { id: appInfo }

    //-------------------------------------------------------------------------
    // TODO: The rightside content beside the 1st level category
    //-------------------------------------------------------------------------
    StackView {
        id: stackView
        width: parent.width - toolBar.width
        height: parent.height - windowTitle.height/* - statusBar.height*/
        anchors.top: windowTitle.bottom; //anchors.left: toolBar.right
        initialItem: StoreView {}
    }

    //-------------------------------------------------------------------------
    // TODO: Window titlebar
    //-------------------------------------------------------------------------
    Rectangle {
        id: windowTitle; width: parent.width; height: 69; color: "#3296e4" //4b5b2a

        //---------------------------------------------------------------------
        // TODO: logo
        //---------------------------------------------------------------------
        Image {
            id: logoImage
            source: brand.logo
            sourceSize.width: 50; sourceSize.height: 50
            //anchors.top: parent.top;
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors { topMargin: 6; leftMargin: 16 }
            smooth: true
        }

        Text {
            id:brandText
            text: brand.name
            x: logoImage.x + logoImage.width + 2
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 20
            font.weight: Font.DemiBold
            color: "#e3f0fc"
        }

        //---------------------------------------------------------------------
        // TODO: Drag area
        //---------------------------------------------------------------------
        DragArea { width: parent.width; height: parent.height }

        //-------------------------------------------------------------------------
        // TODO: Toolbar // 最上边的软件大全/软件升级/软件卸载
        //-------------------------------------------------------------------------
        Rectangle {
            id: toolBar
            //width: 12; height: 12 //75,rootWindow.height - windowTitle.height
            //anchors.top: parent.top//windowTitle.bottom
            //anchors.left: brandText.right
            x: brandText.x + brandText.width + 30
            y: parent.y
            color: "#e4ecd7"

            MyToolButton {
                id: storeToolButton
                title: qsTr("MainPage")
                //icon: "../images/store-toolbutton.png"
                click: true

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        storeToolButton.click = true
                        updateToolButton.click = false
                        uninstallToolButton.click = false
                        stackView.clear()
                        stackView.push(Qt.resolvedUrl("StoreView.qml"))
                        jadedBus.getUpdate()
                        //storeToolButton.click_color = "white"
                    }
                    cursorShape: Qt.PointingHandCursor
                }
            }
            MyToolButton {
                id: updateToolButton
                title: qsTr("Upgrade")
                upgrade_button:true
                //icon: "../images/update-toolbutton.png"
                //normal_color: "#a7be7a"
                //anchors.top: storeToolButton.bottom
                //anchors.left: storeToolButton.right
                x: storeToolButton.x + storeToolButton.width +10

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        storeToolButton.click = false
                        updateToolButton.click = true
                        uninstallToolButton.click = false
                        stackView.clear()
                        stackView.push(Qt.resolvedUrl("UpdateView.qml"))
                    }
                    cursorShape: Qt.PointingHandCursor
                }
            }
            MyToolButton {
                id: uninstallToolButton
                title: qsTr("Uninstall")
                //icon: "../images/uninstall-toolbutton.png"
                //normal_color: "#b2c689"
                //anchors.top: updateToolButton.bottom
                //anchors.left: updateToolButton.right
                x: updateToolButton.x + updateToolButton.width +10

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        storeToolButton.click = false
                        updateToolButton.click = false
                        uninstallToolButton.click = true
                        stackView.clear()
                        stackView.push(Qt.resolvedUrl("UninstallView.qml"))
                    }
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }


        //---------------------------------------------------------------------
        // TODO: Control panel
        //---------------------------------------------------------------------
        JadedBus {
            id: jadedBus

            Component.onCompleted: {
                jadedBus.getUpdate()
            }
            onGetUpdateError:{
                updateToolButton.upgrade_number = 0
            }

            onUpdateChanged:{
                updateToolButton.upgrade_number = count
                //updateToolButton.visible = false
            }
        }
        
        FileDialog {
            id: fileDialog
            title: qsTr("Please choose a package file")
            onAccepted: {
                jadedBus.installFile(fileDialog.fileUrl);
            }
        }
        
        Menu {
            id: sysNavMenu

            MenuItem {
                text: qsTr("Task queue")
                onTriggered: {
                    stackView.push({item: Qt.resolvedUrl("TaskQueueView.qml"),        
                        properties: {stackView: stackView}})
                }
            }

            MenuItem {
                text: qsTr("Install from local")
                onTriggered: {
                    fileDialog.open()
                }
                enabled:false
                visible: false
            }

            MenuItem {
                text: qsTr("Setting")
                onTriggered: {
                    stackView.push({item: Qt.resolvedUrl("Setting.qml"),
                        properties: {stackView: stackView}})
                }
            }

            MenuItem {
                text: qsTr("About")
                onTriggered: {
                    var component = Qt.createComponent("AboutView.qml");
                    var aboutWin = component.createObject(rootWindow);
                    aboutWin.x = rootWindow.x + (rootWindow.width-440)/2;
                    aboutWin.y = rootWindow.y + (rootWindow.height-266)/2;
                    aboutWin.show();
                }
            }
        }
        
        Rectangle {
            id: sysNav
            width: 26; height: 20
            y: 30
            anchors.right: parent.right
            color: "transparent"

            property bool hover: false
            property bool click: false

            Image {
                id: sysNavIcon
                source: "../images/sysNav_icon" + (sysNav.click ? "_hover" : 
                    sysNav.hover ? "_hover" : "") + ".png"
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            Image {
                source: "../images/sysMsg_icon.png"
                anchors.horizontalCenter: sysNavIcon.left
                anchors.verticalCenter: sysNavIcon.bottom
                visible: true
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: sysNav.hover = true
                onExited: sysNav.hover = false
                onClicked: {
                    sysNavMenu.popup()
                }
                cursorShape: Qt.PointingHandCursor
            }
        }

        //---------------------------------------------------------------------
        // TODO: SearchBar
        //---------------------------------------------------------------------
        TextField {
            id: searchTextField
            width: 200 //138
            x: sysNav.x - width - 8
            anchors.bottom: sysNav.bottom
            anchors.bottomMargin: -5
            placeholderText: qsTr("Search...")
            //style: activeFocus ? searchFocusStyle : searchStyle
            onAccepted: {
                if (text != "") {
                    if (searchButtonRegion.name == "clear") stackView.pop()
                    searchButtonRegion.name = "clear"
                    stackView.push({item: Qt.resolvedUrl("SearchView.qml"), 
                        properties: {keyword: searchTextField.text, 
                                     stackView: stackView, 
                                     rect: searchButtonRegion}})
                }
            }
        }

        Component {
            id: searchStyle

            TextFieldStyle {
                textColor: "white"
                background: Rectangle { color: "#e9f2f9"; height: 22; } //627639
            }
        }

        Component {                                                                
            id: searchFocusStyle 
                                                                                   
            TextFieldStyle {                                                       
                textColor: "black"
                background: Rectangle { color: "white"; height: 22; }            
            }                                                                      
        }

        //---------------------------------------------------------------------
        // TODO: Search EnterButton
        //---------------------------------------------------------------------
        Rectangle {
            id: searchButtonRegion
            color: "transparent"
            width: 16; height: 19
            anchors.right: searchTextField.right; anchors.bottom: searchTextField.bottom

            property string name: "search_btn"
            property bool enter: false
            property bool hover: false

            Image {
                anchors.verticalCenter: parent.verticalCenter
                source: "../images/" + searchButtonRegion.name + 
                    (searchButtonRegion.hover ? "_hover" : "") + ".png"
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: searchButtonRegion.hover = true
                onExited: searchButtonRegion.hover = false
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (searchButtonRegion.name == "search_btn") {
                        if (searchTextField.text != "") {
                            searchButtonRegion.name = "clear"
                            stackView.push({item: Qt.resolvedUrl("SearchView.qml"), 
                                properties: {keyword: searchTextField.text, 
                                             stackView: stackView, 
                                             rect: searchButtonRegion}})
                        }
                    } else {
                        searchButtonRegion.name = "search_btn"
                        searchTextField.text = ""
                        searchTextField.focus = false
                        stackView.pop()
                    }
                }
            }
        }

        //---------------------------------------------------------------------
        // TODO: Close, maximize and minmize button
        //---------------------------------------------------------------------
        MyWindowButton {
            id: closeWindowButton
            name: "close"
            sourceHeight: 12
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.topMargin: 15
            y: parent.y + 8
        }
        MyWindowButton {
            id: minWindowButton;
            anchors.right: closeWindowButton.left
            anchors.rightMargin: 12
            y: parent.y + 3
        }
    }
    
    //-------------------------------------------------------------------------
    // TODO: StatusBar
    //-------------------------------------------------------------------------
    Rectangle {
        id: statusBar
        width: parent.width; height: 19
        anchors.bottom: parent.bottom
        color: "#c0e0f9" //e4ecd7

        Text {
            text: qsTr("Version: %1").arg(appInfo.version) // appInfo.version
            color: "#999997"
            font.pixelSize: 10
            anchors.verticalCenter: parent.verticalCenter
            x: 3
        }
    }
}
