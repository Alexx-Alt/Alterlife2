import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import AfterLife
Rectangle {
    id:root;color:"transparent"
    required property var field
    ColumnLayout { anchors.fill:parent;spacing:8
        Repeater { model:[
            ["Температура","temperature",0,2,"Повышение температуры сглаживает фронт и ослабляет закрепление ветвей."],
            ["Пересыщение","supersaturation",0,2,"Доступность вещества для роста; увеличивает число успешных актов присоединения."],
            ["Анизотропия","anisotropy",0,1,"Сила ориентационного отбора решётки. Высокая создаёт выраженные направления роста."],
            ["Мутации","mutationRate",0,1,"Наследуемое изменение предпочтительной ориентации дочерней ячейки."],
            ["Ветвление","branching",0,1,"Преимущество одиночного фронта перед заполнением плотных областей."],
            ["Диффузия","diffusion",0,1,"Эффективность доставки вещества к активному фронту."],
            ["Скорость","speed",.02,80,"Число попыток роста за кадр; физические коэффициенты не умножаются напрямую."]
        ];delegate:ColumnLayout { Layout.fillWidth:true;spacing:2
            RowLayout { Layout.fillWidth:true;Text{text:modelData[0];color:Theme.muted;font.pixelSize:10;Layout.fillWidth:true}HelpDot{help:modelData[4]}Text{text:Number(root.field[modelData[1]]).toFixed(3);color:Theme.text;font.pixelSize:10;font.family:"monospace";Layout.preferredWidth:55;horizontalAlignment:Text.AlignRight} }
            Slider { Layout.fillWidth:true;from:modelData[2];to:modelData[3];value:root.field[modelData[1]];onMoved:root.field[modelData[1]]=value }
        } }
        Rectangle{Layout.fillWidth:true;height:1;color:Theme.border}
        RowLayout { Layout.fillWidth:true;Text{text:"Разрешение сетки";color:Theme.muted;font.pixelSize:10;Layout.fillWidth:true}HelpDot{help:"Число ячеек по стороне. Увеличение повышает детализацию и стоимость отрисовки/роста."}TextField{text:root.field.gridSize.toString();Layout.preferredWidth:80;color:Theme.text;background:Rectangle{radius:7;color:Theme.backgroundAlt;border.color:Theme.border}onEditingFinished:{var v=Number(text);if(!isNaN(v))root.field.gridSize=Math.round(v)}} }
        RowLayout { Layout.fillWidth:true;Text{text:"Seed";color:Theme.muted;font.pixelSize:10;Layout.fillWidth:true}TextField{id:seedField;text:root.field.seed.toString();Layout.preferredWidth:100;color:Theme.text;background:Rectangle{radius:7;color:Theme.backgroundAlt;border.color:Theme.border}
                onEditingFinished:{var v=Number(text);if(!isNaN(v))root.field.seed=Math.max(1,Math.floor(v))}} }
        RowLayout { Layout.fillWidth:true;GlassButton{Layout.fillWidth:true;text:"Сброс";icon:"↻";onClicked:root.field.reset()}GlassButton{Layout.fillWidth:true;text:"+1000 поколений";icon:"≫";onClicked:root.field.skipGenerations(1000)} }
        GlassButton { Layout.fillWidth:true;text:"Экспорт отчёта JSON";icon:"⇩";onClicked:root.field.exportReport("afterlife-crystal-report.json") }
        Text { Layout.fillWidth:true;wrapMode:Text.WordWrap;color:Theme.dim;font.pixelSize:9;lineHeight:1.3;text:"Клик по полю создаёт дополнительный центр кристаллизации. Ориентация фронта наследуется, мутирует и проходит отбор через анизотропию и локальную геометрию." }
        Item{Layout.fillHeight:true}
    }
}
