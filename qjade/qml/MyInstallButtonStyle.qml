import QtQuick 2.0
import QtQuick.Controls.Styles 1.0

ButtonStyle {
    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 25
        border.width: control.activeFocus ? 2 : 1
        border.color: "#dddddd"
        radius: 4
        color: "#f9fafc"
    }
}
