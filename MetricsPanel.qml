import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import AfterLife
Rectangle {
    color:"transparent"
    Flickable { anchors.fill:parent;contentHeight:col.implicitHeight+20;clip:true;ScrollBar.vertical:ScrollBar{}
        ColumnLayout { id:col;width:parent.width-18;x:9;y:7;spacing:8
            Text{text:"СОСТОЯНИЕ";color:Theme.accent;font.pixelSize:9;font.letterSpacing:2;font.bold:true}
            ColumnLayout { Layout.fillWidth:true;spacing:4
                Repeater { model:[
                    ["Время",sim.time.toFixed(2)],["Сущности",sim.totalEntities],["Звёзды",sim.starCount],["Планеты",sim.planetCount],["Спутники",sim.moonCount],["Пригодные",sim.habitablePlanetCount],["Галактики",sim.galaxyCount],["Сверхструктуры",sim.superclusterCount],["Корабли",sim.shipCount],
                    ["Кванты",sim.quantCount],["Частицы",sim.particleCount],["Клетки",sim.cellCount],["Колонии",sim.colonyCount],["Разум",sim.mindCount]
                ]; delegate:Item { Layout.fillWidth:true;implicitHeight:19;property var row:modelData; Text{text:row[0];color:Theme.muted;font.pixelSize:10} Text{anchors.right:parent.right;text:row[1];color:Theme.text;font.pixelSize:10;font.family:"monospace"} } }
            }
            Rectangle{Layout.fillWidth:true;height:1;color:Theme.border}
            Text{text:"СИСТЕМА ВЫБОРА";color:Theme.accent;font.pixelSize:9;font.letterSpacing:2;font.bold:true}
            ColumnLayout { Layout.fillWidth:true;spacing:5
                Repeater { model:[
                    ["Мощность",sim.power.toFixed(3),"Средний доступный энергетический запас на сущность."],
                    ["Задержка ответа",sim.responseLatency.toFixed(3),"Средняя нормализованная задержка реакции живых агентов."],
                    ["Эффективность",sim.efficiency.toFixed(3),"Отношение полезного энергетического потока к накопленным затратам."],
                    ["Энтропия",sim.entropy.toFixed(3),"Нормированная пространственная энтропия распределения сущностей."],
                    ["Индекс выбора",sim.decisionIndex.toFixed(3),"Сводная метрика уровня памяти, поколений и сложности принятия решения."],
                    ["Сложность",sim.complexity.toFixed(3),"Структурная метрика массы и числа развитых уровней."],
                    ["Темп эволюции",sim.evolutionRate.toFixed(2),"Изменение числа сущностей за единицу моделируемого времени."],
                    ["Ср. поколение",sim.avgGeneration.toFixed(2),"Средний номер поколения среди всех сущностей."],
                    ["Ср. уровень",sim.avgLevel.toFixed(2),"Средний дискретный уровень системы выбора."]
                ]; delegate:RowLayout { Layout.fillWidth:true; Text{text:modelData[0];color:Theme.muted;font.pixelSize:10;Layout.fillWidth:true} HelpDot{help:modelData[2]} Text{text:modelData[1];color:Theme.text;font.pixelSize:10;font.family:"monospace";Layout.preferredWidth:58;horizontalAlignment:Text.AlignRight} } }
            }
            Rectangle{Layout.fillWidth:true;height:1;color:Theme.border}
            Text{text:"ПОСЛЕДНИЕ 720 ТОЧЕК";color:Theme.accent;font.pixelSize:9;font.letterSpacing:2;font.bold:true}
            RowLayout { Layout.fillWidth:true; CheckBox{id:replay; text:"Отчёт"; enabled:sim.replayAvailable} GlassButton{Layout.fillWidth:true;text:"Загрузить afterlife-report.json";onClicked:{if(sim.loadReportForReplay("afterlife-report.json"))replay.checked=true}} }
            HistoryChart{Layout.fillWidth:true;Layout.preferredHeight:155;useReplay:replay.checked}
        }
    }
}
