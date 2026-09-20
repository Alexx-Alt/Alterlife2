import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import AfterLife
Rectangle {
    id: root; color:"transparent"
    property var lifeResult: ({})
    property var model: [
        {group:"НАЧАЛЬНОЕ СОСТОЯНИЕ",name:"Первичные элементы",prop:"quantCount",min:100,max:12000,val:1200,integer:true,help:"Число дискретных элементов первичного вещества. Больше — выше пространственное разрешение, но дороже расчёт."},
        {group:"НАЧАЛЬНОЕ СОСТОЯНИЕ",name:"Температура среды",prop:"initialTemperature",min:0,max:4,val:.85,help:"Нормализованная тепловая дисперсия скоростей. Горячий газ хуже коллапсирует."},
        {group:"НАЧАЛЬНОЕ СОСТОЯНИЕ",name:"Размер мира",prop:"worldRadius",min:180,max:1600,val:520,help:"Радиус вычислительной области в модельных пространственных единицах."},
        {group:"НАЧАЛЬНОЕ СОСТОЯНИЕ",name:"Зародышей галактик",prop:"galaxySeedCount",min:1,max:12,val:4,integer:true,help:"Число начальных крупномасштабных неоднородностей плотности."},
        {group:"НАЧАЛЬНОЕ СОСТОЯНИЕ",name:"Флуктуации плотности",prop:"densityFluctuation",min:0,max:2,val:.55,help:"Разброс плотности первичного вещества вокруг крупномасштабных центров."},
        {group:"ГРАВИТАЦИЯ И ГАЗ",name:"Гравитационная константа",prop:"gravityConstant",min:.01,max:3,val:.42,help:"G в нормализованных единицах. Меняет динамические времена и орбитальные скорости."},
        {group:"ГРАВИТАЦИЯ И ГАЗ",name:"Смягчение гравитации",prop:"gravitationalSoftening",min:.1,max:20,val:3.5,help:"Plummer softening подавляет численную сингулярность силы на нулевом расстоянии."},
        {group:"ГРАВИТАЦИЯ И ГАЗ",name:"Тёмная материя",prop:"darkMatterFactor",min:0,max:8,val:1.8,help:"Масштаб гладкого гало, удерживающего вещество на галактических масштабах."},
        {group:"ГРАВИТАЦИЯ И ГАЗ",name:"Радиус взаимодействия",prop:"interactionRadius",min:6,max:120,val:34,help:"Локальный радиус оценки плотности при гравитационном коллапсе."},
        {group:"ГРАВИТАЦИЯ И ГАЗ",name:"Охлаждение газа",prop:"gasCoolingRate",min:0,max:2,val:.18,help:"Эффективность потери тепловой поддержки. Высокое значение ускоряет звездообразование."},
        {group:"ГРАВИТАЦИЯ И ГАЗ",name:"Диссипация",prop:"friction",min:.90,max:1,val:.997,help:"Численная диссипация мелкомасштабных движений. 1 — без искусственного затухания."},
        {group:"ЗВЁЗДЫ И ОРБИТЫ",name:"Порог коллапса",prop:"starFormationThreshold",min:4,max:80,val:18,help:"Минимальная локальная насыщенность области перед проверкой критерия коллапса."},
        {group:"ЗВЁЗДЫ И ОРБИТЫ",name:"Планет на звезду",prop:"planetsPerStar",min:0,max:16,val:6,integer:true,help:"Верхняя граница числа устойчивых орбитальных полос; фактическое число зависит от массы диска и доступного пространства."},
        {group:"ЗВЁЗДЫ И ОРБИТЫ",name:"Цена образования планеты",prop:"planetFormationCost",min:.5,max:30,val:5,help:"Сколько локально захваченного вещества требуется на одну устойчивую планетную орбиту."},
        {group:"ЗВЁЗДЫ И ОРБИТЫ",name:"Мин. масса планеты",prop:"minPlanetMass",min:.05,max:4,val:.35,help:"Нижняя граница массы планет в земных нормализованных единицах."},
        {group:"ЗВЁЗДЫ И ОРБИТЫ",name:"Макс. масса планеты",prop:"maxPlanetMass",min:.1,max:20,val:6.5,help:"Верхняя граница массы планет; применяется до общего коэффициента массы."},
        {group:"ЗВЁЗДЫ И ОРБИТЫ",name:"Мин. разнос орбит",prop:"minOrbitSpacing",min:5,max:80,val:24,help:"Минимальный радиальный разрыв между круговыми орбитами. Предотвращает пересечение орбит в базовой модели."},
        {group:"ЗВЁЗДЫ И ОРБИТЫ",name:"Спутников на планету",prop:"maxMoonsPerPlanet",min:0,max:8,val:2,integer:true,help:"Верхняя граница числа естественных спутников. Фактическое число ограничивается локальной зоной устойчивости планеты."},
        {group:"ЗВЁЗДЫ И ОРБИТЫ",name:"Разнос орбит спутников",prop:"minMoonOrbitSpacing",min:.5,max:12,val:3.5,help:"Минимальный разрыв между орбитами спутников вокруг одной планеты."},
        {group:"ЗВЁЗДЫ И ОРБИТЫ",name:"Мин. масса звезды",prop:"stellarMassMin",min:.08,max:2,val:.55,help:"Нижняя масса формирующейся звезды в солнечных нормализованных единицах."},
        {group:"ЗВЁЗДЫ И ОРБИТЫ",name:"Макс. масса звезды",prop:"stellarMassMax",min:.2,max:12,val:1.8,help:"Верхняя масса звезды. Светимость масштабируется приблизительно как M^3.5."},
        {group:"ЗВЁЗДЫ И ОРБИТЫ",name:"Внутренняя HZ",prop:"habitableZoneInner",min:.1,max:4,val:.82,help:"Справочный коэффициент внутренней границы зоны потенциальной обитаемости."},
        {group:"ЗВЁЗДЫ И ОРБИТЫ",name:"Внешняя HZ",prop:"habitableZoneOuter",min:.2,max:6,val:1.55,help:"Справочный коэффициент внешней границы зоны потенциальной обитаемости."},
        {group:"ПЛАНЕТАРНАЯ СРЕДА",name:"Масштаб массы планет",prop:"planetMass",min:.05,max:5,val:1,help:"Глобальный множитель массы тел; влияет на удержание атмосферы, гравитационную пригодность и магнитный момент."},
        {group:"ПЛАНЕТАРНАЯ СРЕДА",name:"Скорость вращения",prop:"rotationSpeed",min:.05,max:8,val:1,help:"Нормализованная скорость осевого вращения: задаёт суточную компоненту теплового поля и вклад в магнитное динамо."},
        {group:"ПЛАНЕТАРНАЯ СРЕДА",name:"Среднее альбедо",prop:"meanAlbedo",min:0,max:.95,val:.31,help:"Доля отражаемого излучения; влияет на равновесную температуру."},
        {group:"ПЛАНЕТАРНАЯ СРЕДА",name:"Парниковый эффект",prop:"greenhouseFactor",min:0,max:3,val:.22,help:"Дополнительное удержание тепла атмосферой."},
        {group:"ПЛАНЕТАРНАЯ СРЕДА",name:"Удержание атмосферы",prop:"atmosphereRetention",min:0,max:1.5,val:.72,help:"Базовая способность планет сохранять атмосферу."},
        {group:"ПЛАНЕТАРНАЯ СРЕДА",name:"Доля океана",prop:"oceanFraction",min:0,max:1,val:.62,help:"Средняя доступность жидкого растворителя; отдельные планеты получают случайный разброс."},
        {group:"ПЛАНЕТАРНАЯ СРЕДА",name:"Магнитное поле",prop:"magneticStrength",min:0,max:2,val:.65,help:"Базовая интенсивность магнитного экранирования от звёздного излучения."},
        {group:"ПЛАНЕТАРНАЯ СРЕДА",name:"Радиационный шум",prop:"radiationNoise",min:0,max:1,val:.035,help:"Фоновая стохастическая радиационная нагрузка и возмущения."},
        {group:"ПЛАНЕТАРНАЯ СРЕДА",name:"Приливная амплитуда",prop:"tidalAmplitude",min:0,max:2,val:.25,help:"Амплитуда периодического приливного сенсора для биологических агентов."},
        {group:"ПЛАНЕТАРНАЯ СРЕДА",name:"Частота приливов",prop:"tidalFrequency",min:.01,max:5,val:.5,help:"Частота приливного цикла."},
        {group:"ЭВОЛЮЦИЯ И ВЫБОР",name:"Химическое разнообразие",prop:"chemistryRichness",min:0,max:1.5,val:.72,help:"Доступность химических путей, увеличивающая вероятность абиогенеза."},
        {group:"ЭВОЛЮЦИЯ И ВЫБОР",name:"Частота абиогенеза",prop:"abiogenesisRate",min:0,max:.1,val:.0035,help:"Интенсивность пуассоновского процесса зарождения первой самоподдерживающейся системы."},
        {group:"ЭВОЛЮЦИЯ И ВЫБОР",name:"Мутации",prop:"mutationRate",min:0,max:.8,val:.045,help:"Стандартный масштаб наследуемого изменения весов системы выбора."},
        {group:"ЭВОЛЮЦИЯ И ВЫБОР",name:"Давление отбора",prop:"selectionPressure",min:0,max:3,val:.65,help:"Усиливает связь между пригодностью среды и успешным размножением."},
        {group:"ЭВОЛЮЦИЯ И ВЫБОР",name:"Скорость обучения",prop:"learningRate",min:0,max:.05,val:.0025,help:"Пластичность весов сенсор→действие во время жизни агента."},
        {group:"ЭВОЛЮЦИЯ И ВЫБОР",name:"Память действий",prop:"memoryFactor",min:0,max:.95,val:.28,help:"Доля предыдущего действия, участвующая в новом решении у развитых агентов."},
        {group:"ЭНЕРГЕТИКА",name:"Поглощение энергии",prop:"baseAbsorptionRate",min:0,max:200,val:34,help:"Максимальная скорость получения ресурса организмом в благоприятной среде."},
        {group:"ЭНЕРГЕТИКА",name:"Метаболизм",prop:"metabolismCost",min:0,max:10,val:.42,help:"Постоянные затраты энергии живого агента за единицу времени."},
        {group:"ЭНЕРГЕТИКА",name:"Цена движения",prop:"movementCost",min:0,max:3,val:.08,help:"Энергетическая стоимость скорости перемещения."},
        {group:"ЭНЕРГЕТИКА",name:"Ёмкость среды",prop:"carryingCapacityPerHabitability",min:1,max:400,val:55,help:"Масштаб экологической ёмкости планеты; дополнительно умножается на пригодность и ресурсы."},
        {group:"ЭНЕРГЕТИКА",name:"Порог деления",prop:"replicationEnergyThreshold",min:1,max:300,val:62,help:"Запас энергии, после которого агент может попытаться воспроизвестись."},
        {group:"ЭНЕРГЕТИКА",name:"Доля в потомка",prop:"replicationCost",min:.02,max:.98,val:.46,help:"Часть энергии родителя, передаваемая потомку при репликации."},
        {group:"ЦИВИЛИЗАЦИЯ",name:"Цена корабля",prop:"shipEnergyCost",min:1,max:300,val:45,help:"Энергия, необходимая колонии для межпланетной экспедиции."},
        {group:"ЦИВИЛИЗАЦИЯ",name:"Скорость корабля",prop:"shipSpeed",min:.1,max:80,val:8,help:"Крейсерская скорость корабля в модельных единицах."},
        {group:"ЦИВИЛИЗАЦИЯ",name:"Порог колонизации",prop:"colonizationThreshold",min:0,max:1,val:.38,help:"Минимальная пригодность планеты, чтобы экспедиция считала её целью."},
        {group:"ВЫЧИСЛЕНИЯ",name:"Максимум сущностей",prop:"maxEntities",min:500,max:30000,val:3500,integer:true,help:"Бюджет сущностей. Это вычислительный предел, а не физический закон."},
        {group:"ВЫЧИСЛЕНИЯ",name:"Seed",prop:"seed",min:1,max:4294967294,val:42,integer:true,help:"Seed генератора. Одинаковый seed и одинаковые параметры воспроизводят начальные условия."}
    ]

    Flickable { anchors.fill:parent; contentHeight:content.implicitHeight+24; clip:true; ScrollBar.vertical:ScrollBar{}
        ColumnLayout { id:content; width:parent.width-18; x:9; y:8; spacing:5
            Repeater { model:root.model; delegate:ColumnLayout { Layout.fillWidth:true; spacing:3
                Text { visible:index===0||root.model[index-1].group!==modelData.group; text:modelData.group; color:Theme.accent; font.pixelSize:9; font.letterSpacing:2; font.bold:true; topPadding:index===0?0:12 }
                RowLayout { Layout.fillWidth:true; spacing:5
                    Text { text:modelData.name; color:Theme.muted; font.pixelSize:10; Layout.fillWidth:true; elide:Text.ElideRight }
                    HelpDot { help:modelData.help }
                    TextField { id:num; Layout.preferredWidth:68; implicitHeight:28; text:modelData.integer?Math.round(modelData.val).toString():Number(modelData.val).toFixed(3); color:Theme.text; font.pixelSize:10; horizontalAlignment:Text.AlignRight; background:Rectangle{radius:6;color:Qt.rgba(.5,.7,1,.05);border.color:Theme.border}
                        onEditingFinished:{var v=Number(text.replace(",","."));if(!isNaN(v)){root.model[index].val=modelData.integer?Math.round(v):v;root.model=root.model.slice()}else text=modelData.val.toString()}
                    }
                }
                Slider { Layout.fillWidth:true; implicitHeight:22; from:modelData.min; to:modelData.max; value:Math.max(from,Math.min(to,modelData.val)); onMoved:{root.model[index].val=modelData.integer?Math.round(value):value;root.model=root.model.slice()} }
            } }
            Rectangle{Layout.fillWidth:true;height:1;color:Theme.border;Layout.topMargin:10}
            RowLayout { Layout.fillWidth:true; GlassButton{Layout.fillWidth:true;text:"Применить";icon:"✓";onClicked:root.applyScenario()} GlassButton{Layout.fillWidth:true;text:"Новый seed";icon:"◌";onClicked:root.randomizeSeed()} }
            GlassButton { Layout.fillWidth:true; text:"Оценить потенциал жизни"; icon:"◇"; onClicked:root.lifeResult=sim.assessLifePotential(root.scenarioJson()) }
            Rectangle { visible:Object.keys(root.lifeResult).length>0; Layout.fillWidth:true; implicitHeight:lifeText.implicitHeight+18; radius:8;color:Qt.rgba(.3,.7,.6,.07);border.color:Theme.border
                Text{id:lifeText;anchors.fill:parent;anchors.margins:9;wrapMode:Text.WordWrap;color:Theme.muted;font.pixelSize:9;text:Object.keys(root.lifeResult).length?root.lifeResult.verdict+" · индекс "+Number(root.lifeResult.score).toFixed(2)+"\nВероятность события абиогенеза в аналитическом скрининге: "+(Number(root.lifeResult.abiogenesisChance)*100).toFixed(1)+"%":""}
            }
            Text{text:"ЭКСПОРТ";color:Theme.accent;font.pixelSize:9;font.letterSpacing:2;font.bold:true;Layout.topMargin:8}
            RowLayout{Layout.fillWidth:true;GlassButton{Layout.fillWidth:true;text:"CSV";onClicked:sim.exportCsv("afterlife-metrics.csv")}GlassButton{Layout.fillWidth:true;text:"Metrics JSON";onClicked:sim.exportJson("afterlife-metrics.json")}}
            GlassButton{Layout.fillWidth:true;text:"Полный отчёт JSON";icon:"⇩";onClicked:sim.exportReport("afterlife-report.json")}
            Text{text:"СЦЕНАРИЙ";color:Theme.accent;font.pixelSize:9;font.letterSpacing:2;font.bold:true;Layout.topMargin:6}
            RowLayout{Layout.fillWidth:true;GlassButton{Layout.fillWidth:true;text:"Сохранить";icon:"⇩";onClicked:sim.saveScenario("afterlife-scenario.json",root.scenarioJson())}GlassButton{Layout.fillWidth:true;text:"Загрузить";icon:"↥";onClicked:root.loadScenarioFile()}}
        }
    }
    function scenarioJson(){var s={};for(var i=0;i<root.model.length;i++){var m=root.model[i];s[m.prop]=m.integer?Math.round(Number(m.val)):Number(m.val)}return JSON.stringify(s)}
    function applyScenario(){sim.applyScenarioFromQml(scenarioJson())}
    function randomizeSeed(){for(var i=0;i<root.model.length;i++)if(root.model[i].prop==="seed"){root.model[i].val=Math.floor(1+Math.random()*4294967293);break}root.model=root.model.slice();applyScenario()}
    function loadScenarioFile(){
        var raw=sim.loadScenario("afterlife-scenario.json"); if(!raw)return;
        var obj=JSON.parse(raw); for(var i=0;i<root.model.length;i++){var key=root.model[i].prop;if(obj[key]!==undefined)root.model[i].val=obj[key]} root.model=root.model.slice()
    }
}
