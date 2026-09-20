import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import AfterLife

Item {
    id: root
    signal backToSelect()
    property bool planetMode: false
    property var planetData: ({})
    property bool dragged: false
    property real lastX: 0
    property real lastY: 0

    Rectangle { anchors.fill:parent;color:Theme.background }
    UniverseRenderer { id:renderer;anchors.fill:parent;simulator:sim;zoom:1;showOrbits:true;backgroundColor:Theme.background;visible:!root.planetMode }

    MouseArea {
        id:navigation;anchors.fill:parent;z:2;visible:!root.planetMode;acceptedButtons:Qt.LeftButton;hoverEnabled:true
        onPressed:function(m){root.dragged=false;root.lastX=m.x;root.lastY=m.y}
        onPositionChanged:function(m){if(pressed){var dx=m.x-root.lastX,dy=m.y-root.lastY;if(Math.abs(dx)+Math.abs(dy)>1){root.dragged=true;renderer.panBy(dx,dy)}root.lastX=m.x;root.lastY=m.y}}
        onReleased:function(m){if(!root.dragged){var hit=renderer.pickPlanet(m.x,m.y);if(Object.keys(hit).length){root.planetData=hit;root.planetMode=true}}}
        onWheel:function(w){var factor=w.angleDelta.y>0?1.12:.89;renderer.zoom=Math.max(.12,Math.min(18,renderer.zoom*factor));w.accepted=true}
        cursorShape: pressed?Qt.ClosedHandCursor:Qt.OpenHandCursor
    }

    Rectangle {
        anchors.top:parent.top;anchors.horizontalCenter:parent.horizontalCenter;anchors.topMargin:14;z:12;width:500;height:58;radius:29;color:Theme.panel;border.color:Theme.border;visible:!root.planetMode
        RowLayout { anchors.fill:parent;anchors.margins:10;spacing:12
            ColumnLayout { Layout.fillWidth:true;spacing:1
                Text { text:{var n=["ПЕРВИЧНАЯ СРЕДА","ЧАСТИЦЫ","ЗВЁЗДНЫЕ СИСТЕМЫ","ГАЛАКТИКИ","ЖИЗНЬ","КОЛОНИИ","РЕФЛЕКСИВНЫЙ ВЫБОР"];return "ЭПОХА "+sim.epoch+" · "+(n[sim.epoch]||"—")} color:Theme.text;font.pixelSize:11;font.bold:true;font.letterSpacing:2 }
                Text { text:"t "+sim.time.toFixed(2)+"   ·   "+sim.totalEntities+" сущностей   ·   "+sim.galaxyCount+" галактик";color:Theme.dim;font.pixelSize:9;font.family:"monospace" }
            }
            GlassButton { implicitWidth:86; text:"Центр"; icon:"⌂"; onClicked:renderer.resetView() }
        }
    }

    CollapsiblePanel { id:params;x:14;y:14;z:15;icon:"⚙";title:"КОНСТРУКТОР";expandedWidth:350;expandedHeight:Math.min(680,root.height-110);ConfigPanel{anchors.fill:parent} }
    CollapsiblePanel { id:log;x:14;y:params.collapsed?76:Math.min(root.height-expandedHeight-88,params.y+params.height+10);z:15;icon:"≡";title:"СОБЫТИЯ";expandedWidth:350;expandedHeight:220;collapsed:true;onCollapsedChanged:{if(!collapsed)params.collapsed=true}LogPanel{anchors.fill:parent} }
    CollapsiblePanel { id:metrics;x:root.width-expandedWidth-14;y:14;z:15;icon:"▤";title:"МЕТРИКИ";expandedWidth:355;expandedHeight:Math.min(650,root.height-250);MetricsPanel{anchors.fill:parent} }
    CollapsiblePanel { id:ai;x:root.width-expandedWidth-14;y:root.height-expandedHeight-84;z:16;icon:"◇";title:"ЛОКАЛЬНЫЙ АССИСТЕНТ";expandedWidth:410;expandedHeight:250;collapsed:true;onCollapsedChanged:{if(!collapsed)metrics.collapsed=true}AssistantPanel{anchors.fill:parent} }

    Rectangle {
        z:18;anchors.bottom:parent.bottom;anchors.horizontalCenter:parent.horizontalCenter;anchors.bottomMargin:14;width:Math.min(920,parent.width-40);height:64;radius:32;color:Theme.panel;border.color:Theme.border;visible:!root.planetMode
        RowLayout { anchors.fill:parent;anchors.margins:10;spacing:8
            GlassButton { implicitWidth:52;text:sim.running?"Ⅱ":"▶";onClicked:sim.running=!sim.running }
            GlassButton { implicitWidth:52;text:"↻";onClicked:sim.reset() }
            Rectangle{Layout.preferredWidth:1;Layout.fillHeight:true;color:Theme.border}
            Text{text:"×"+sim.speed.toFixed(sim.speed<10?1:0);color:Theme.text;font.pixelSize:10;font.family:"monospace";Layout.preferredWidth:42}
            Slider { Layout.preferredWidth:190;from:0;to:1;value:Math.log(sim.speed/.05)/Math.log(20000);onMoved:sim.speed=.05*Math.pow(20000,value);ToolTip.visible:pressed;ToolTip.text:"Скорость ×"+sim.speed.toFixed(1) }
            GlassButton { implicitWidth:76;text:"+100";onClicked:sim.fastForward(100) }
            GlassButton { implicitWidth:82;text:"+1000";onClicked:sim.fastForward(1000) }
            GlassButton { implicitWidth:116;text:"След. эпоха";icon:"≫";onClicked:sim.skipToNextEpoch() }
            Rectangle{Layout.preferredWidth:1;Layout.fillHeight:true;color:Theme.border}
            CheckBox { text:"Орбиты";checked:renderer.showOrbits;onToggled:renderer.showOrbits=checked }
            GlassButton { implicitWidth:55;text:"AI";onClicked:ai.collapsed=!ai.collapsed }
            GlassButton { implicitWidth:55;text:"←";onClicked:root.backToSelect() }
        }
    }

    Timer { interval:220; repeat:true; running:root.planetMode; onTriggered:{ if(root.planetData.id!==undefined){var d=renderer.planetDetail(root.planetData.id);if(Object.keys(d).length)root.planetData=d} } }
    PlanetDetail { anchors.fill:parent;z:30;visible:root.planetMode;planet:root.planetData;onCloseRequested:{root.planetMode=false;root.planetData=({})} }
    Shortcut { sequence:"Esc";onActivated:{if(root.planetMode)root.planetMode=false;else root.backToSelect()} }
    Shortcut { sequence:"Space";onActivated:sim.running=!sim.running }
}
