import QtQuick
import QtQuick.Controls
import AfterLife
Item {
    id: root
    property bool collapsed:false; property string icon:"☰"; property string title:""; property int expandedWidth:300; property int expandedHeight:400; property bool draggable:true
    default property alias content: container.data
    readonly property int islandSize:46; readonly property int headerHeight:38
    width:collapsed?islandSize:expandedWidth;height:collapsed?islandSize:expandedHeight
    Behavior on width { enabled:!drag.drag.active; NumberAnimation{duration:220;easing.type:Easing.OutCubic} }
    Behavior on height { enabled:!drag.drag.active; NumberAnimation{duration:220;easing.type:Easing.OutCubic} }
    Rectangle { anchors.fill:parent; radius:collapsed?23:Theme.panelRadius; color:Theme.panel; border.color:collapsed?Theme.accent:Theme.border; clip:true
        Item { anchors.fill:parent; visible:root.collapsed; Text{anchors.centerIn:parent;text:root.icon;color:Theme.accent;font.pixelSize:17} MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:root.collapsed=false} }
        Item { anchors.fill:parent; visible:!root.collapsed
            Rectangle { id:header;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;height:root.headerHeight;color:Qt.rgba(.4,.65,.9,.035)
                Row { anchors.left:parent.left;anchors.leftMargin:12;anchors.verticalCenter:parent.verticalCenter;spacing:8;Text{text:root.icon;color:Theme.accent;font.pixelSize:13}Text{text:root.title;color:Theme.muted;font.pixelSize:10;font.letterSpacing:2;font.bold:true} }
                Rectangle { anchors.right:parent.right;anchors.rightMargin:7;anchors.verticalCenter:parent.verticalCenter;width:25;height:25;radius:13;color:collapse.containsMouse?Qt.rgba(.5,.7,1,.10):"transparent";Text{anchors.centerIn:parent;text:"−";color:Theme.muted;font.pixelSize:15}MouseArea{id:collapse;anchors.fill:parent;hoverEnabled:true;cursorShape:Qt.PointingHandCursor;onClicked:root.collapsed=true} }
                MouseArea { id:drag;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;anchors.bottom:parent.bottom;anchors.rightMargin:42;enabled:root.draggable;cursorShape:Qt.SizeAllCursor;drag.target:root;drag.axis:Drag.XAndYAxis;drag.minimumX:0;drag.minimumY:0;drag.maximumX:root.parent?Math.max(0,root.parent.width-root.width):9999;drag.maximumY:root.parent?Math.max(0,root.parent.height-root.height):9999 }
            }
            Item { id:container;anchors.top:header.bottom;anchors.left:parent.left;anchors.right:parent.right;anchors.bottom:parent.bottom;anchors.margins:8 }
        }
    }
}
