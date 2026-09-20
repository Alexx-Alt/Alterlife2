import QtQuick
import QtQuick.Controls
import AfterLife
Control {
    id: root
    property string text: ""
    property string icon: ""
    signal clicked()
    implicitWidth: 120; implicitHeight: 38
    hoverEnabled: true
    background: Rectangle { radius: 10; color: root.hovered ? Qt.rgba(0.55,0.75,1,0.14) : Theme.panel; border.color: root.hovered ? Theme.accent : Theme.border; border.width: 1; Behavior on color { ColorAnimation { duration: 140 } } }
    contentItem: Row { spacing: 7; anchors.centerIn: parent; Text { text: root.icon; color: Theme.accent; font.pixelSize: 13 } Text { text: root.text; color: Theme.text; font.pixelSize: 11; font.letterSpacing: 1 } }
    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: root.clicked() }
}
