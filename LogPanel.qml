import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import AfterLife
Rectangle { id:root;color:"transparent";ListModel{id:logs}
    Connections{target:sim;function onLogMessage(msg){logs.append({t:sim.time.toFixed(1),msg:msg});if(logs.count>400)logs.remove(0);Qt.callLater(list.positionViewAtEnd)}}
    ColumnLayout{anchors.fill:parent;spacing:5
        RowLayout{Layout.fillWidth:true;Text{text:logs.count+" событий";color:Theme.dim;font.pixelSize:9}Item{Layout.fillWidth:true}GlassButton{implicitWidth:72;implicitHeight:28;text:"Очистить";onClicked:logs.clear()}}
        ListView{id:list;Layout.fillWidth:true;Layout.fillHeight:true;clip:true;model:logs;spacing:3;delegate:Text{width:list.width;text:"["+model.t+"]  "+model.msg;color:Theme.muted;font.pixelSize:9;font.family:"monospace";wrapMode:Text.WrapAnywhere}}
    }
}
