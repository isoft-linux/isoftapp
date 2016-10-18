// Copyright (C) 2016 Leslie Zhai <xiang.zhai@i-soft.com.cn>

import QtQuick 2.2
import QtQuick.Controls 1.0

Item {
    width: 300; height: 200

    ListModel {
        id: myListModel

        ListElement {
            name: "item0"
            check: false
        }

        Component.onCompleted: {
            for (var i = 1; i < 100; i++) {
                myListModel.append({name: "item" + i, checked: false});
            }
        }
    }

    Component {
        id: myListViewDelegate

        Rectangle {
            objectName: "myItem"
            width: parent.width; height: 30

            signal checkItem(bool checked, int i);
            onCheckItem: {
                myCheckBox.checked = checked;
            }

            CheckBox {
                id: myCheckBox
                text: name
                checked: check

                onClicked: {
                    myListModel.setProperty(index, "check", this.checked);
                    if (this.checked) {
                        var allChecked = true;
                        for (var i = 0; i < myListModel.count; i++) {
                            if (myListModel.get(i).check == false) {
                                allChecked = false;
                                break;
                            }
                        }
                        checkAllBox.checked = allChecked;
                    } else {
                        checkAllBox.checked = false;
                    }
                    var moreThanOne = false;
                    for (var i = 0; i < myListModel.count; i++) {
                        if (myListModel.get(i).check) {
                            moreThanOne = true;
                            break;
                        }
                    }
                    myButton.visible = moreThanOne;
                }
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
            id: checkAllBox
            text: "Check All"

            onClicked: {
                myButton.visible = this.checked;
                for (var i = 0; i < myListModel.count; i++) {
                    myListModel.setProperty(i, "check", this.checked);
                }
                for (var i = 0; i < myListView.contentItem.children.length; i++) {
                    var item = myListView.contentItem.children[i];
                    if (typeof(item) == 'undefined')
                        continue;
                    else if (item.objectName != "myItem")
                        continue;
                    item.checkItem(this.checked, i);
                }
            }
        }

        Button {
            id: myButton
            anchors.left: checkAllBox.right
            text: "Operation"
            visible: false
        }
    }
}
