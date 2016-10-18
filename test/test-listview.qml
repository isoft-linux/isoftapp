// Copyright (C) 2016 Leslie Zhai <xiang.zhai@i-soft.com.cn>

import QtQuick 2.2
import QtQuick.Controls 1.0

Item {
    width: 300; height: 200

    ListModel {
        id: myListModel

        ListElement {
            name: "item1"
            check: false
        }

        Component.onCompleted: {
            for (var i = 0; i < 100; i++) {
                myListModel.append({name: "item" + (i + 2), checked: false});
            }
        }
    }

    Component {
        id: myListViewDelegate

        Rectangle {
            width: parent.width; height: 30

            CheckBox {
                text: name
                checked: check
            }
        }
    }

    ListView {
        id: myListView
        width: parent.width; height: parent.height - checkAllRect.height
        anchors.top: checkAllRect.bottom
        model: myListModel
        delegate: myListViewDelegate
    }
    
    Rectangle {
        id: checkAllRect
        width: parent.width; height: 30

        CheckBox {
            text: "Check All"

            onClicked: {
                for (var i = 0; i < myListModel.count; i++) {
                    myListModel.setProperty(i, "name", "item " + (i + 1) + " " + this.checked);
                    myListModel.setProperty(i, "check", this.checked);
                }
            }
        }
    }
}
