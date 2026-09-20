import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import AfterLife
Item {
    id:root;signal backToSelect()
    Rectangle{anchors.fill:parent;color:Theme.background}
    CrystalField{id:crystal;anchors.fill:parent;running:false;backgroundColor:Theme.background}
    MouseArea{anchors.fill:parent;z:2;acceptedButtons:Qt.LeftButton;onClicked:function(m){crystal.nucleate(m.x,m.y)} }

    CollapsiblePanel { x:14;y:14;z:10;icon:"❄";title:"СРЕДА И ОТБОР";expandedWidth:340;expandedHeight:560;CrystalConfigPanel{anchors.fill:parent;field:crystal} }
    CollapsiblePanel { x:root.width-expandedWidth-14;y:14;z:10;icon:"▤";title:"МЕТРИКИ КРИСТАЛЛА";expandedWidth:310;expandedHeight:320
        ColumnLayout{anchors.fill:parent;spacing:10
            Repeater{model:[ ["Поколение",crystal.generation],["Покрытие",(crystal.coverage*100).toFixed(2)+"%"],["Разнообразие",crystal.diversity.toFixed(3)],["Энергия фронта",crystal.frontierEnergy.toFixed(3)] ];delegate:RowLayout{Layout.fillWidth:true;Text{text:modelData[0];color:Theme.muted;font.pixelSize:10;Layout.fillWidth:true}Text{text:modelData[1];color:Theme.text;font.pixelSize:10;font.family:"monospace"}}}
            Rectangle{Layout.fillWidth:true;height:1;color:Theme.border}
            Text{Layout.fillWidth:true;wrapMode:Text.WordWrap;color:Theme.dim;font.pixelSize:9;lineHeight:1.35;text:"Разнообразие — энтропия распределения наследуемой ориентации. Энергия фронта — отношение активной границы к объёму кристалла; она падает при уплотнении структуры."}
            Item{Layout.fillHeight:true}
        }
    }

    Rectangle { z:12;anchors.bottom:parent.bottom;anchors.horizontalCenter:parent.horizontalCenter;anchors.bottomMargin:14;width:570;height:62;radius:31;color:Theme.panel;border.color:Theme.border
        RowLayout{anchors.fill:parent;anchors.margins:10;spacing:8
            GlassButton{implicitWidth:60;text:crystal.running?"Ⅱ":"▶";onClicked:crystal.running=!crystal.running}
            GlassButton{implicitWidth:65;text:"↻";onClicked:crystal.reset()}
            Text{text:"ген. "+crystal.generation;color:Theme.muted;font.pixelSize:10;font.family:"monospace";Layout.fillWidth:true}
            GlassButton{implicitWidth:110;text:"+100";onClicked:crystal.skipGenerations(100)}
            GlassButton{implicitWidth:130;text:"+5000";onClicked:crystal.skipGenerations(5000)}
            GlassButton{implicitWidth:60;text:"←";onClicked:root.backToSelect()}
        }
    }
    Shortcut{sequence:"Space";onActivated:crystal.running=!crystal.running}Shortcut{sequence:"Esc";onActivated:root.backToSelect()}
}
