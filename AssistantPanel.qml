import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import AfterLife
Rectangle { id:root;color:"transparent";ListModel{id:chat}
    Connections{target:assistant;function onResponseReady(text){chat.append({who:"AI",body:text});Qt.callLater(view.positionViewAtEnd)}function onCommandExecuted(text){chat.append({who:"SYS",body:text})}}
    ColumnLayout{anchors.fill:parent;spacing:6
        RowLayout{Layout.fillWidth:true;Text{text:"LOCAL ASSISTANT";color:Theme.accent;font.pixelSize:9;font.letterSpacing:2;font.bold:true}Item{Layout.fillWidth:true}Text{text:assistant.busy?"думает…":assistant.status;color:Theme.dim;font.pixelSize:8;elide:Text.ElideRight;Layout.maximumWidth:150}}
        ListView{id:view;Layout.fillWidth:true;Layout.fillHeight:true;clip:true;model:chat;spacing:7;delegate:Rectangle{width:view.width;implicitHeight:body.implicitHeight+16;radius:9;color:model.who==="USER"?Qt.rgba(.45,.68,1,.10):Qt.rgba(.5,.7,.65,.06);Text{id:body;anchors.fill:parent;anchors.margins:8;text:model.body;color:Theme.muted;font.pixelSize:9;wrapMode:Text.WordWrap}}}
        RowLayout{Layout.fillWidth:true;TextField{id:input;Layout.fillWidth:true;placeholderText:"Спросить или: «скорость x20», «следующая эпоха»";color:Theme.text;selectByMouse:true;background:Rectangle{radius:9;color:Theme.backgroundAlt;border.color:Theme.border}onAccepted:root.send()}GlassButton{implicitWidth:68;text:"→";onClicked:root.send()}}
        RowLayout{Layout.fillWidth:true;Text{text:"LLM endpoint";color:Theme.dim;font.pixelSize:8}TextField{Layout.fillWidth:true;implicitHeight:27;text:assistant.endpoint;color:Theme.muted;font.pixelSize:8;selectByMouse:true;background:Rectangle{radius:7;color:Theme.backgroundAlt;border.color:Theme.border}onEditingFinished:assistant.endpoint=text}}
    }
    function send(){var q=input.text.trim();if(!q)return;chat.append({who:"USER",body:q});input.text="";assistant.ask(q);Qt.callLater(view.positionViewAtEnd)}
}
