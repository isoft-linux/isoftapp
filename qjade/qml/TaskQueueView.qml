/*                                                                                 
 * Copyright (C) 2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 */

import QtQuick 2.2
import QtQuick.Controls 1.0                                                        
import QtQuick.Controls.Styles 1.0                                                 
import cn.com.isoft.qjade 2.0

Rectangle {
    id: taskQueueView
    width: parent.width; height: parent.height

    property StackView stackView
    property int count: 0

    JadedBus {                                                                     
        id: taskQueueJadedBus
        Component.onCompleted: {
            taskQueueJadedBus.getTaskQueue()
        }
        onTaskQueueChanged: {                                                       
            taskQueueView.count = count;
            taskListView.model = taskQueueJadedBus.taskQueue; 
            myResultText.isVisible = count == 0 ? true : false;
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
                    taskQueueView.stackView.pop() 
                }
            }
        }
                                                                           
        Text {                                                                     
            id: taskCountText
            text: qsTr("%1 total").arg(taskListView.count)
            font.pixelSize: 12                                       
            anchors.left: navImage.right                                              
            anchors.verticalCenter: parent.verticalCenter                          
        }
                                                                                   
        Rectangle { y: parent.height - 1; width: parent.width; height: 1; color: "#e4ecd7"}
    }

    ScrollView {
        width: parent.width
        height: parent.height - titleRegion.height
        anchors.top: titleRegion.bottom
        anchors.left: titleRegion.left  
        flickableItem.interactive: true

        ListView {
            id: taskListView
            anchors.fill: parent

            delegate: Rectangle {
                id: taskRect
                width: parent.width
                height: 60
                color: index % 2 == 0 ? "white" : "#f5f5f5"

                Image {
                    id: appIcon
                    source: modelData.icon
                    sourceSize.width: 48; sourceSize.height: 48
                    anchors.left: parent.left
                    anchors.leftMargin: 7
                    anchors.verticalCenter: parent.verticalCenter
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
                    width: parent.width - appIcon.width - nameText.width - funcButton.width
                    anchors.left: nameText.left
                    anchors.top: nameText.bottom
                    anchors.topMargin: 8                                           
                    wrapMode: Text.WordWrap                                        
                    font.pixelSize: 11                                             
                    color: "#b7b7b7"
                }

                JadedBus {
                    id: jadedBus

                    property bool isError: false
                    property string info: "UnknownInfo"

                    Component.onCompleted: {                                        
                        jadedBus.info = jadedBus.getInfo(modelData.name);
                        if (jadedBus.info == "InfoRunning") {
                            funcButton.visible = false;
                            infoText.visible = true;
                        }
                    }
                    onTaskStarted: {
                        if (name == modelData.name) {
                            funcButton.visible = false;
                            infoText.visible = true;
                        }
                    }                    
                    onTaskChanged: {                                               
                        if (name == modelData.name) {                              
                            taskRect.height = 0;
                            taskRect.visible = false;
                            taskQueueView.count--;
                            if (taskQueueView.count == 0) {
                                myResultText.visible = true;
                            }
                            taskCountText.text = qsTr("%1 total").arg(taskQueueView.count);
                        }                                                          
                    }
                }

                PercentageButton {                                                 
                    id: funcButton                                                 
                    text: qsTr("Cancel")                                          
                    anchors.right: parent.right                                    
                    anchors.rightMargin: 17                                        
                    anchors.verticalCenter: parent.verticalCenter                  
                    onClicked: {               
                        jadedBus.cancel(modelData.name)
                        taskRect.height = 0
                        taskRect.visible = false
                    }                                                              
                    enabled: modelData.datetime == "doing" ? false:true // modelData.datetime is status
                }

                Text {
                    id: infoText
                    text: qsTr("Running")
                    anchors.right: parent.right
                    anchors.rightMargin: 17
                    anchors.verticalCenter: parent.verticalCenter
                    visible: false
                }
            }
        }

        style: MyScrollViewStyle {}
    }

    MyResultText { id: myResultText; result: qsTr("No Task Available") }
}
