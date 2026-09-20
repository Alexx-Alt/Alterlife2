import QtQuick
import QtQuick.Controls
import AfterLife
Item {
    id: root
    property alias text: label.text
    property bool hovered: mouse.containsMouse
    property real depth: 1
    readonly property real radius: 23
    signal clicked(); signal hoverChanged(bool hovered)
    onHoveredChanged: hoverChanged(hovered)
    implicitWidth: 190; implicitHeight: 46
    scale: depth * (hovered ? 1.04 : 1)
    Behavior on scale { NumberAnimation { duration: 160; easing.type: Easing.OutCubic } }
    Rectangle { anchors.fill: parent; radius: root.radius; color: root.hovered ? Qt.rgba(.55,.75,1,.12) : Theme.panel; border.color: root.hovered ? Theme.accent : Theme.border; border.width: 1 }
    Text { id: label; anchors.centerIn: parent; color: root.hovered ? Theme.text : Theme.muted; font.pixelSize: 11; font.letterSpacing: 3; font.weight: Font.Medium }
    MouseArea { id: mouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: root.clicked() }
}
