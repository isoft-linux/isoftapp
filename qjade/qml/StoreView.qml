/*                                                                                 
 * Copyright (C) 2014 - 2016 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 */

import QtQuick 2.2
import QtQuick.Controls 1.0
import cn.com.isoft.qjade 2.0

Rectangle {
    id: storeView

    StackView {                                                                 
        id: storeStackView                                                          
        width: parent.width - categoryListView.width; height: parent.height 
        anchors.left: categoryRegion.right
        initialItem: HotTodayView {}
    }

    //-------------------------------------------------------------------------
    // FIXME: To cover the left and right part for slideshow
    //-------------------------------------------------------------------------
    /*
    Rectangle {
        width: 163
        height: 200
        color: "white"
    }
*/
    //-------------------------------------------------------------------------
    // TODO: green line
    //-------------------------------------------------------------------------
    Rectangle {
        x: categoryRegion.width
        width: 4
        height: parent.height                                                      
        color: "#c1c1c1"
    }

    //-------------------------------------------------------------------------
    // TODO: 2nd level category[今日推荐/开发工具/办公软件]
    //-------------------------------------------------------------------------
    Rectangle {
        id: categoryRegion
        width: 120 //163
        height: parent.height
        //color: "transparent"
        property bool isFirst: true
        
        CategoryModel {
            id: categoryModel
            onCategoryChanged: {myLoader.visible = false;}
        }

        ListView {
            id: categoryListView 
            model: categoryModel.categories
            anchors.fill: parent
            interactive: false

            //-----------------------------------------------------------------
            // TODO: seven colors
            //-----------------------------------------------------------------
            property variant indexColors: ["#3296e4", "#bdd448", "#69ae55", "#fe5dad", "#4dc0ff", "#f1ef4c", "#a27795","#3896e8"]

            delegate: Rectangle {
                id: categoryElementItem
                width: parent.width
                height: 44 // e4ebd6
                color: categoryListView.currentIndex == index ? (categoryRegion.isFirst ? (hover ? "#c8e4fa" : "transparent") : ( categoryListView.currentIndex == index ? "#c8e4fa" : (hover ? "#c8e4fa" : "transparent") ) ) : (hover ? "#c8e4fa" : "transparent")
                //color: categoryListView.currentIndex == index ? "#c8e4fa" : (hover ? "#c8e4fa" : "transparent")

                property bool hover: false
                property bool click: false
                property bool isMyPkg: modelData.title =="My_pkgs" ? true : false

                JadedBus {
                    id: jadedBus
                    Component.onCompleted: {
                        if (isMyPkg)
                        jadedBus.getMyPkgNumber()
                    }
                    onMyPkgNumChanged:{
                        if (isMyPkg)
                        rightText.text =  (modelData.title == "All_Pkg" ? qsTr("All Pkg") : modelData.title =="My_pkgs" ? qsTr("My_pkgs") :modelData.title ) + "(" + count +")"
                    }
                }

                //-------------------------------------------------------------
                // TODO: change color based on index value
                //-------------------------------------------------------------
                Rectangle {
                    x: parent.width
                    width: 4
                    height: parent.height
                    color: categoryListView.indexColors[index % 8] // 7->8
                }

                Image {
                    id: categoryIcon
                    //source: modelData.icon
                    x: 38
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    id: rightText
                    text: (modelData.title == "All_Pkg" ? qsTr("All Pkg") : modelData.title =="My_pkgs" ? qsTr("My_pkgs") :modelData.title ) + "(" + modelData.number +")"
                    x: 20 //categoryIcon.x + categoryIcon.width + 3
                    anchors.verticalCenter: parent.verticalCenter
                    width: 110
                    clip: true                                                         
                    elide: Text.ElideRight
                }

                Image {
                    source: "../images/sysNav_current.png"
                    x: parent.width - width - 10
                    anchors.verticalCenter: parent.verticalCenter
                    visible: categoryListView.currentIndex == index ? true : (categoryElementItem.hover ? true : false)
                }

                Rectangle {
                    width: parent.width
                    height: 1
                    anchors.top: parent.bottom
                    color: "#e9e9e9"
                }

                MouseArea {                                                        
                    anchors.fill: parent                                           
                    hoverEnabled: true
                    onEntered: categoryElementItem.hover = true
                    onExited: categoryElementItem.hover = false
                    onClicked: { 
                        categoryRegion.isFirst = false
                        var patt = /^category-/;
                        categoryListView.currentIndex = index
                        if (modelData.title =="My_pkgs") {
                            storeStackView.clear()
                            storeStackView.push(Qt.resolvedUrl("HistoryView.qml"))
                        } else
                        if (patt.test(modelData.name)) {
                            storeStackView.clear()
                            storeStackView.push({item: Qt.resolvedUrl("PackageByCategoryView.qml"), properties: {category: modelData.name, title: modelData.title, loading: true}}) 
                        } else {
                            storeStackView.clear()
                            //storeStackView.push(Qt.resolvedUrl("HotTodayView.qml"))
                            // modelData.name:"all-pkg"
                            // modelData.title qsTr("All Pkg")
                            storeStackView.push(
                                        {item: Qt.resolvedUrl("PackageByCategoryView.qml"),
                                         properties: {category: modelData.name,
                                            title: qsTr("All Pkg"),
                                            loading: true}
                                        })
                        }
                    }
                }
            }
        }

        MyLoader { id: myLoader }
    }
}
