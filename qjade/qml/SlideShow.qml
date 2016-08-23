/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
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
            width: 40; height: 40; color: "transparent"

            property bool hover

            Rectangle { 
                width: 20; height: 20; 
                color: slideShow.currentIndex == index ? "white" : (selectorItem.hover ? "white" : "#66C184")
                opacity: slideShow.currentIndex == index ? 0.2 : (selectorItem.hover ? 0.2 : 1.0)
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
