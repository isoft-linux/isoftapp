import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.1

Button {
    id: percentageButton
    width: 90
    height: 21

    property int percentage: 0
    property string textColor: "#61a917"

    style: ButtonStyle {
        background: Rectangle {
            implicitWidth: parent.width
            implicitHeight: parent.height
            color: "#f9fafc"
            border.width: control.activeFocus ? 2 : 1
            border.color: "#dddddd"
            radius: 2

            Rectangle {
                width: parent.width * percentage / 100
                height: parent.height
                color: "#1ba0e1"
            }
        }

        label: Text {
            text: percentageButton.text
            color: textColor
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            anchors.fill: parent
        }
    }
}
