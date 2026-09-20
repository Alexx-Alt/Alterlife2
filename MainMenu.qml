import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import AfterLife

Item {
    id: root
    signal startSimulation(); signal quit()
    property Item activeButton: null
    property real orbitPhase: 0

    Rectangle { anchors.fill: parent; color: Theme.background }
    ParticleField { id: ambient; anchors.fill: parent; shape: ParticleField.FreeForm; density: 700; color: Theme.light ? "#335b7c" : "#c0daff"; opacity: Theme.light ? .16 : .42; mouseX: mouseTracker.mouseX; mouseY: mouseTracker.mouseY; interactionRadius: 130; repelStrength: .42 }
    ParticleField { id: titleParticles; anchors.fill: parent; shape: ParticleField.Text; text: "AFTERLIFE"; density: 1500; color: Theme.light ? "#24577f" : "#d8e9ff"; shapeX: width/2; shapeY: Math.max(145, height*.21); shapeSize: Math.min(630, width*.52); mouseX: mouseTracker.mouseX; mouseY: mouseTracker.mouseY; interactionRadius: 145; repelStrength: 1.2 }
    ParticleField { id: outlineParticles; anchors.fill: parent; shape: ParticleField.FreeForm; density: 340; color: Theme.light ? "#315a7c" : "#b8d7ff"; opacity: .82; mouseX: mouseTracker.mouseX; mouseY: mouseTracker.mouseY }

    MouseArea { id: mouseTracker; anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.NoButton }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter; y: Math.max(250,height*.33); spacing: 9
        Text { anchors.horizontalCenter: parent.horizontalCenter; text: "ЭВОЛЮЦИОННАЯ ЛАБОРАТОРИЯ"; color: Theme.text; font.pixelSize: 13; font.letterSpacing: 6; font.weight: Font.Medium }
        Text { anchors.horizontalCenter: parent.horizontalCenter; text: "физика → выбор → адаптация → воспроизводство"; color: Theme.muted; font.pixelSize: 11; font.letterSpacing: 2 }
    }

    NumberAnimation on orbitPhase { from: 0; to: Math.PI*2; duration: 76000; loops: Animation.Infinite; running: true }
    Timer { interval: 16; running: root.activeButton !== null; repeat: true; onTriggered: { if(!root.activeButton)return; var p=root.activeButton.mapToItem(outlineParticles,0,0); outlineParticles.outlineX=p.x; outlineParticles.outlineY=p.y; outlineParticles.outlineW=root.activeButton.width; outlineParticles.outlineH=root.activeButton.height; outlineParticles.cornerRadius=root.activeButton.radius } }

    Item {
        id: orbit; anchors.centerIn: parent; width: 1; height: 1; anchors.verticalCenterOffset: 92
        property real rx: Math.min(350, root.width*.28); property real ry: Math.min(150, root.height*.18)
        Repeater {
            model: [ {txt:"НАЧАТЬ",a:0}, {txt:"НАСТРОЙКИ",a:Math.PI/2}, {txt:"О ПРОЕКТЕ",a:Math.PI}, {txt:"ВЫХОД",a:Math.PI*1.5} ]
            delegate: MenuButton {
                property real a: modelData.a + root.orbitPhase
                property real zDepth: (Math.sin(a)+1)/2
                depth: .86 + zDepth*.20
                x: Math.cos(a)*orbit.rx - width/2
                y: Math.sin(a)*orbit.ry - height/2
                z: 2 + zDepth*5
                opacity: .72 + zDepth*.28
                text: modelData.txt
                onClicked: { if(modelData.txt==="НАЧАТЬ")root.startSimulation(); else if(modelData.txt==="НАСТРОЙКИ")settings.open(); else if(modelData.txt==="О ПРОЕКТЕ")about.open(); else root.quit() }
                onHoverChanged: function(h){ if(h){root.activeButton=this;outlineParticles.shape=ParticleField.Outline}else if(root.activeButton===this){root.activeButton=null;outlineParticles.shape=ParticleField.FreeForm} }
            }
        }
    }

    Row {
        anchors.bottom: parent.bottom; anchors.bottomMargin: 22; anchors.horizontalCenter: parent.horizontalCenter; spacing: 18
        Text { text: "drag-free interface"; color: Theme.dim; font.pixelSize: 9; font.letterSpacing: 2 }
        Text { text: "•"; color: Theme.dim }
        Text { text: Theme.mode.toUpperCase(); color: Theme.dim; font.pixelSize: 9; font.letterSpacing: 2 }
    }

    Popup {
        id: settings; modal: true; focus: true; x: Math.round((parent.width-width)/2); y: Math.round((parent.height-height)/2); width: 470; height: 390; padding: 0
        background: Rectangle { color: Theme.panelStrong; radius: Theme.panelRadius; border.color: Theme.border }
        contentItem: ColumnLayout {
            anchors.fill: parent; anchors.margins: 24; spacing: 16
            RowLayout { Layout.fillWidth: true; Text { text:"ИНТЕРФЕЙС"; color:Theme.text; font.pixelSize:14; font.letterSpacing:4; font.bold:true } Item{Layout.fillWidth:true} ToolButton{text:"×";onClicked:settings.close()} }
            Text { text:"Системная тема"; color:Theme.muted; font.pixelSize:11 }
            RowLayout { Layout.fillWidth:true; Repeater { model:[{n:"Тёмная",k:"dark"},{n:"Светлая",k:"light"},{n:"Liquid glass",k:"liquid"}]; delegate: GlassButton { Layout.fillWidth:true; text:modelData.n; onClicked:Theme.setTheme(modelData.k) } } }
            Rectangle { Layout.fillWidth:true; height:1; color:Theme.border }
            Text { text:"Фоновая система частиц реагирует на курсор. Кнопки движутся по проекции 3D-орбиты, но сами не вращаются — область клика всегда совпадает с визуальным элементом."; color:Theme.muted; wrapMode:Text.WordWrap; Layout.fillWidth:true; lineHeight:1.35 }
            Text { text:"Плотность частиц"; color:Theme.muted; font.pixelSize:11 }
            Slider { Layout.fillWidth:true; from:250; to:2200; value:titleParticles.density; onMoved:titleParticles.density=Math.round(value) }
            Item { Layout.fillHeight:true }
            GlassButton { Layout.alignment:Qt.AlignRight; text:"Готово"; icon:"✓"; onClicked:settings.close() }
        }
    }

    Popup {
        id: about; modal:true; focus:true; x: Math.round((parent.width-width)/2); y: Math.round((parent.height-height)/2); width:560; height:420; padding:0
        background: Rectangle { color:Theme.panelStrong; radius:Theme.panelRadius; border.color:Theme.border }
        contentItem: ColumnLayout { anchors.fill:parent; anchors.margins:28; spacing:14
            RowLayout { Layout.fillWidth:true; Text{text:"AFTERLIFE";color:Theme.text;font.pixelSize:18;font.letterSpacing:5;font.bold:true} Item{Layout.fillWidth:true} ToolButton{text:"×";onClicked:about.close()} }
            Text { Layout.fillWidth:true; wrapMode:Text.WordWrap; color:Theme.muted; lineHeight:1.45; text:"Конструктор вычислительных миров, в котором физические состояния становятся входами системы принятия решений, а наследование, отбор, память и энергетический бюджет превращают простые реакции в более сложное поведение." }
            Rectangle { Layout.fillWidth:true;height:1;color:Theme.border }
            Text { Layout.fillWidth:true; wrapMode:Text.WordWrap; color:Theme.dim; lineHeight:1.4; text:"Важно: режим галактики — нормализованная вычислительная модель, а не прецизионный астрофизический пакет. Законы и зависимости выбраны физически мотивированными, но масштаб и timestep адаптированы для интерактивной работы на обычном компьютере." }
            Item{Layout.fillHeight:true}
            GlassButton { Layout.alignment:Qt.AlignRight; text:"Закрыть"; onClicked:about.close() }
        }
    }
}
