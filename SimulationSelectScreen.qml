import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import AfterLife

Item {
    id: root
    signal backToMenu(); signal simulationChosen(string key)
    property int selectedIndex: 0
    property var simulations: [
        { title:"Космологическая эволюция", subtitle:"гравитация · системы · жизнь · выбор", key:"galaxy", icon:"◎", description:"Нормализованная физическая модель: облака вещества формируют звёздные системы и галактические структуры. На подходящих мирах возникает жизнь, наследует поведение, учится, строит колонии и межпланетные экспедиции." },
        { title:"Рост кристалла", subtitle:"анизотропия · наследование · отбор", key:"crystal", icon:"❄", description:"Эволюционная модель роста: фронт кристалла наследует ориентацию решётки, мутирует и конкурирует за локальный поток вещества. Температура, пересыщение и диффузия меняют морфологию и разнообразие." }
    ]
    Rectangle { anchors.fill:parent; color:Theme.background }
    ParticleField { anchors.fill:parent; shape:ParticleField.FreeForm; density:480; color:Theme.light?"#315b7e":"#c0daff"; opacity:.22 }

    RowLayout {
        anchors.fill:parent; anchors.margins:36; spacing:20
        Rectangle {
            Layout.preferredWidth: 330; Layout.fillHeight:true; color:Theme.panel; radius:Theme.panelRadius; border.color:Theme.border
            ColumnLayout { anchors.fill:parent; anchors.margins:18; spacing:12
                RowLayout { Layout.fillWidth:true; GlassButton{text:"Назад";icon:"←";onClicked:root.backToMenu()} Item{Layout.fillWidth:true} }
                Text { text:"РЕЖИМЫ"; color:Theme.text; font.pixelSize:15; font.letterSpacing:5; font.bold:true }
                Text { text:"Выберите модель мира"; color:Theme.muted; font.pixelSize:11 }
                Repeater { model:root.simulations; delegate: Rectangle {
                    Layout.fillWidth:true; implicitHeight:92; radius:12; color:root.selectedIndex===index?Qt.rgba(.45,.68,1,.11):hover.containsMouse?Qt.rgba(.5,.7,1,.055):"transparent"; border.color:root.selectedIndex===index?Theme.accent:Theme.border
                    Row { anchors.fill:parent; anchors.margins:14; spacing:14
                        Text { text:modelData.icon; color:Theme.accent; font.pixelSize:24; anchors.verticalCenter:parent.verticalCenter }
                        Column { width:parent.width-55; anchors.verticalCenter:parent.verticalCenter; spacing:5
                            Text { text:modelData.title; color:Theme.text; font.pixelSize:12; font.bold:true }
                            Text { text:modelData.subtitle; color:Theme.muted; font.pixelSize:9; wrapMode:Text.WordWrap; width:parent.width }
                        }
                    }
                    MouseArea { id:hover; anchors.fill:parent; hoverEnabled:true; cursorShape:Qt.PointingHandCursor; onClicked:root.selectedIndex=index }
                } }
                Item{Layout.fillHeight:true}
                Text { Layout.fillWidth:true; wrapMode:Text.WordWrap; color:Theme.dim; font.pixelSize:10; text:"Оба режима сохраняют seed, параметры и метрики, поэтому прогоны можно воспроизводить и сравнивать." }
            }
        }

        Rectangle {
            Layout.fillWidth:true; Layout.fillHeight:true; color:Theme.panel; radius:Theme.panelRadius; border.color:Theme.border; clip:true
            Loader { id:preview; anchors.fill:parent; anchors.bottomMargin:220; sourceComponent:root.selectedIndex===0?galaxyPreview:crystalPreview }
            Rectangle { anchors.left:parent.left;anchors.right:parent.right;anchors.bottom:parent.bottom;height:220;color:Theme.panelStrong;opacity:.96 }
            ColumnLayout { anchors.left:parent.left;anchors.right:parent.right;anchors.bottom:parent.bottom;anchors.margins:28;spacing:10
                Text { text:root.simulations[root.selectedIndex].title; color:Theme.text; font.pixelSize:22; font.letterSpacing:2; font.bold:true }
                Text { Layout.fillWidth:true; text:root.simulations[root.selectedIndex].description; color:Theme.muted; font.pixelSize:12; wrapMode:Text.WordWrap; lineHeight:1.35 }
                RowLayout { Layout.fillWidth:true; Item{Layout.fillWidth:true} GlassButton { implicitWidth:190; implicitHeight:44; text:"Открыть симуляцию"; icon:"→"; onClicked:root.simulationChosen(root.simulations[root.selectedIndex].key) } }
            }
        }
    }

    Component { id:galaxyPreview; Item { ParticleField { anchors.fill:parent; shape:ParticleField.Galaxy; density:950; shapeX:width/2;shapeY:height/2;shapeSize:190;opacity:.88 } Text{anchors.centerIn:parent;text:"КОСМОЛОГИЯ";color:Theme.text;font.pixelSize:12;font.letterSpacing:7;opacity:.72} } }
    Component { id:crystalPreview; Item { CrystalField { anchors.fill:parent; running:true; backgroundColor:Theme.background; speed:9; supersaturation:.74; anisotropy:.82; temperature:.34 } Text{anchors.horizontalCenter:parent.horizontalCenter;anchors.bottom:parent.bottom;anchors.bottomMargin:25;text:"ЭВОЛЮЦИОННЫЙ ФРОНТ";color:Theme.text;font.pixelSize:11;font.letterSpacing:6;opacity:.75} } }
    Shortcut { sequence:"Esc"; onActivated:root.backToMenu() }
}
