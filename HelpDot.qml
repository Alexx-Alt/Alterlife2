import QtQuick
import QtQuick.Controls
import AfterLife
Item {
    id: root; width: 18; height: 18
    property string help: ""
    Rectangle { anchors.fill: parent; radius: 9; color: hover.containsMouse ? Qt.rgba(.5,.7,1,.16) : "transparent"; border.color: Theme.border }
    Text { anchors.centerIn: parent; text: "?"; color: Theme.muted; font.pixelSize: 10; font.bold: true }
    MouseArea { id: hover; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.WhatsThisCursor }
    ToolTip.visible: hover.containsMouse && root.help.length > 0
    ToolTip.text: root.help
    ToolTip.delay: 250
    ToolTip.timeout: 10000
}
