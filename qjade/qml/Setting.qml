/*                                                                                 
 * Copyright (C) 2016 fj <fujiang@i-soft.com.cn>
 */

import QtQuick 2.2
import QtQuick.Controls 1.0                                                        
import QtQuick.Controls.Styles 1.0                                                 
import cn.com.isoft.qjade 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.4
Rectangle {
    id: settingView
    width: parent.width; height: parent.height
    //color: "red"

    property StackView stackView
    property int count: 0
    property string urlText:"file:///home"
    property int deleteAction:-1

    JadedBus {
        id: jadedBus
        Component.onCompleted: {
            jadedBus.getPathMode()
        }

        onSettingChanged: {
            textInput.text = path;
            deleteAction = mode;
            urlText = path;
            if (urlText.length > 510) {
                urlText = "file:///home"
            }
        }
    }

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
                onClicked: { 
                    settingView.stackView.pop()
                }
            }
        }
                                                                           
        Text {                                                                     
            id: taskCountText
            text: qsTr("Setting")
            font.pixelSize: 12                                       
            anchors.left: navImage.right                                              
            anchors.verticalCenter: parent.verticalCenter                          
        }
                                                                                   
        Rectangle { y: parent.height - 1; width: parent.width; height: 1; color: "#e4ecd7"}
    }

    // path Rectangle
    Rectangle {
        id: pathRegion
        anchors.top: titleRegion.bottom
        anchors.left: parent.left
        anchors.leftMargin: parent.width/20
        width: parent.width
        height: parent.height*3/10
        //color: "green"

        Text {
            id: dlPathText
            text: qsTr("Pkg rpms download roadpath")
            font.pixelSize: 20
            anchors.top: pathRegion.top
            anchors.topMargin: 20
            anchors.left: pathRegion.left
            anchors.verticalCenter: parent.verticalCenter
        }
        Rectangle {
            id: pathRect
            anchors.top: pathRegion.top
            anchors.topMargin: 60
            anchors.left: pathRegion.left
            anchors.leftMargin:pathRegion.width/12
            width: parent.width*6/10
            height: parent.height*1/8
            border.color: "#707070"
            color: "#c1c1c1"
            radius: 4

            Text {
                id: hintText
                anchors { fill: pathRect; leftMargin: 14 }
                verticalAlignment: Text.AlignVCenter
                text: qsTr("Select a folder to store rpms.")
                font.pixelSize: 12
                color: "#707070"
                opacity: textInput.length ? 0 : 1
            }

            Text {
                id: prefixText
                anchors { left: pathRect.left; leftMargin: 14; verticalCenter: pathRect.verticalCenter }
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 12
                color: "#707070"
                opacity: !hintText.opacity
            }

            TextInput {
                id: textInput
                focus: true
                anchors { left: prefixText.right; right: pathRect.right; top: pathRect.top; bottom: pathRect.bottom }
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 12
                //color: "#707070"
                color: "black"
                onAccepted: wrapper.accepted()
                readOnly: true

                property int maxLength: 80 //最大输入长度
                onLengthChanged:
                {
                    if(textInput.length >= maxLength)
                    {
                        var bakStr = textInput.text;
                        var prePosition = cursorPosition;
                        textInput.text = bakStr.substring(0, maxLength/2);
                        textInput.text += "...";
                        textInput.text += bakStr.substring(bakStr.length - maxLength/2 +3 , bakStr.length);
                        cursorPosition = textInput.length;
                    }
                }
            }
        }

        Text {
            text: qsTr("Please choose a suitable folder to store Pkg rpms.")
            font.pixelSize: 12
            anchors.top: pathRect.bottom
            anchors.topMargin: 20
            anchors.left: pathRect.left
            anchors.verticalCenter: parent.verticalCenter
        }

        PercentageButton {
            id: pathButton
            text: qsTr("Change")
            anchors.left: pathRect.right
            anchors.top: pathRect.top
            anchors.leftMargin: 30
            onClicked: {
                folderSelect.open();
            }
        }

        FileDialog {
            id: folderSelect
            title: qsTr("Choose folder")
            folder: shortcuts.documents

            selectFolder: true
            selectMultiple: false
            onFileUrlChanged: {
                urlText = fileUrl;
                if (urlText.length > 510) { // daemon.cpp limited:512
                    urlText = "file:///home"
                }

                textInput.text = urlText;
            }
        }
    }

    // action Rectangle
    Rectangle {
        id: actionRegion
        anchors.top: pathRegion.bottom
        anchors.left: parent.left
        anchors.leftMargin: parent.width/20
        width: parent.width
        height: parent.height*4/10
        //color: "yellow"

        Text {
            id: actionText
            text: qsTr("How to deal with Pkg rpms")
            font.pixelSize: 20
            anchors.top: actionRegion.top
            anchors.left: actionRegion.left
            anchors.verticalCenter: parent.verticalCenter
        }

        Rectangle {
            id: radioRect
            anchors.top: actionRegion.top
            anchors.topMargin: 30
            anchors.left: actionRegion.left
            anchors.leftMargin:actionRegion.width/12

            width: parent.width*6/10
            height: parent.height*5/10
            //color: "red"

                ExclusiveGroup { id: tabPositionGroup }
                RadioButton {
                    text: qsTr("Delete after installation")
                    id: delRadio
                    anchors.top: radioRect.top
                    anchors.topMargin: 20
                    anchors.left: radioRect.left
                    checked: deleteAction == 1? true:false
                    exclusiveGroup: tabPositionGroup
                }
                RadioButton {
                    text: qsTr("Delete after one week")
                    id: weekRadio
                    anchors.top: delRadio.bottom
                    anchors.topMargin: 20
                    anchors.left: radioRect.left
                    exclusiveGroup: tabPositionGroup
                    checked: deleteAction == 2? true:false
                }
                RadioButton {
                    text: qsTr("Manually delete")
                    id: manRadio
                    anchors.top: weekRadio.bottom
                    anchors.topMargin: 20
                    anchors.left: radioRect.left
                    exclusiveGroup: tabPositionGroup
                    checked: deleteAction == 3? true:false
                }
        }

        PercentageButton {
            id: okButton
            text: qsTr("Ok")
            anchors.right: radioRect.right
            anchors.rightMargin: 30
            anchors.top: radioRect.bottom
            anchors.topMargin: 50
            onClicked: {
                deleteAction = delRadio.checked ? 1:weekRadio.checked ? 2:manRadio.checked ? 3:-1
                //actionText.text = urlText + ":" + deleteAction

                jadedBus.setPathMode(urlText,deleteAction)
                settingView.stackView.pop()
            }
        }

        PercentageButton {
            id: cancelButton
            text: qsTr("Cancel")
            anchors.left: radioRect.right
            anchors.leftMargin: 30
            anchors.top: radioRect.bottom
            anchors.topMargin: 50
            onClicked: {
                settingView.stackView.pop()
            }
        }
    }

    // button Rectangle
    Rectangle {
        id: buttonRegion
        anchors.top: actionRegion.bottom
        anchors.left: parent.left
        anchors.leftMargin: parent.width/20
        width: parent.width
        height: parent.height*3/10
        //color: "green"
    }

    MyResultText { id: myResultText; result: qsTr("Setting!") }
}
