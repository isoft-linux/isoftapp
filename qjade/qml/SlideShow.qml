/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 *               2016 fujiang <fujiang.zhu@i-soft.com.cn>
 */

import QtQuick 2.2

Item {
    id: slideShow
    
    property var slideModel
    property int currentIndex: 0
    property string bannerTitle: bannerListView.count ? slideModel[0].title : ""

    ListView {
        id: bannerListView
        model: slideModel   
        anchors.fill: parent
        orientation: Qt.Horizontal
        spacing: 10
        currentIndex: slideShow.currentIndex 
        onCurrentIndexChanged: slideShow.currentIndex = currentIndex
        interactive: false       
        highlightMoveVelocity: 1000.0

        delegate: Rectangle {
            width: slideShow.width
            Image { 
                source: modelData.icon
                asynchronous: true
                width: slideShow.width
                height: slideShow.height
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        Qt.openUrlExternally(modelData.url)
                    }
                }
            }
        }
    }

    Rectangle {
        id: bannerRegion
        width: slideShow.width
        height: 49
        anchors.bottom: slideShow.bottom
        color: "#656766"
        opacity: 0.8

        Text {
            text: slideShow.bannerTitle
            font.pixelSize: 18
            color: "white"
            anchors.left: parent.left
            anchors.leftMargin: 48
            anchors.verticalCenter: parent.verticalCenter
            width: 300
            clip: true
            elide: Text.ElideRight
        }
    }

    ListView {
        id: selector
        model: slideModel
        width: 180
        
        orientation: Qt.Horizontal
        currentIndex: slideShow.currentIndex
        onCurrentIndexChanged: slideShow.currentIndex = currentIndex
        anchors.top: bannerRegion.top
        anchors.topMargin: 15
        anchors.right: slideShow.right
        anchors.rightMargin: 10
        interactive: false

        delegate: Rectangle {
            id: selectorItem
            width: 30; height: 30; color: "transparent"

            property bool hover

            Rectangle { 
                width: 14; height: 14;
                color: slideShow.currentIndex == index ? "#00c5da" : (selectorItem.hover ? "#00c5da" : "white")
                opacity: slideShow.currentIndex == index ? 1.0 : (selectorItem.hover ? 1.0 : 0.2)
                radius: width/2
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: { 
                    slideShow.currentIndex = index
                    slideShow.bannerTitle = modelData.title
                    selectorItem.hover = true 
                }
                onExited: selectorItem.hover = false
                onClicked: {
                    slideShow.currentIndex = index 
                    slideShow.bannerTitle = modelData.title
                }
            }
        }
    }
}
