import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import AfterLife

Item {
    id: root
    property var planet: ({})
    signal closeRequested()
    property real yaw: 0.2
    property real pitch: -0.15
    property real sphereZoom: 1.0
    property real shipPhase: 0
    property bool showColonies: true
    property bool showTransport: true
    property bool showSatellites: true
    property bool showGrid: true

    Rectangle { anchors.fill: parent; color: Theme.background }
    ParticleField { anchors.fill: parent; shape: ParticleField.FreeForm; density: 360; color: Theme.light ? "#315b7e" : "#c0daff"; opacity: .18 }

    Canvas {
        id: globe
        anchors.fill: parent
        anchors.leftMargin: 330
        anchors.rightMargin: 330
        onPaint: {
            var ctx=getContext("2d"); ctx.clearRect(0,0,width,height)
            var cx=width/2, cy=height/2, r=Math.min(width,height)*.31*root.sphereZoom
            var g=ctx.createRadialGradient(cx-r*.34,cy-r*.32,r*.05,cx,cy,r)
            var h=Number(root.planet.habitability||0), t=Number(root.planet.temperature||1)
            var c1=h>.5?"#9be6c8":t>1.2?"#e0a274":t<.75?"#b8d7f2":"#aab7c7"
            var c2=h>.5?"#1e796b":t>1.2?"#7a3d2c":t<.75?"#315b83":"#455568"
            g.addColorStop(0,c1);g.addColorStop(.58,c2);g.addColorStop(1,"#07101a")
            ctx.fillStyle=g;ctx.beginPath();ctx.arc(cx,cy,r,0,Math.PI*2);ctx.fill()
            ctx.strokeStyle="rgba(190,220,255,.18)";ctx.lineWidth=1;ctx.stroke()

            // latitude guides rotate visually with pitch
            if(root.showGrid){ctx.save();ctx.beginPath();ctx.arc(cx,cy,r,0,Math.PI*2);ctx.clip();ctx.strokeStyle="rgba(210,235,255,.07)";
            for(var lat=-2;lat<=2;lat++){var yy=cy+lat*r*.26+Math.sin(root.pitch)*r*.08;ctx.beginPath();ctx.moveTo(cx-r*.9,yy);ctx.bezierCurveTo(cx-r*.45,yy-r*.16,cx+r*.45,yy-r*.16,cx+r*.9,yy);ctx.bezierCurveTo(cx+r*.45,yy+r*.16,cx-r*.45,yy+r*.16,cx-r*.9,yy);ctx.stroke()}
            ctx.restore()}

            // deterministic colonies: projected points on a rotating sphere
            var count=root.showColonies?Math.min(54,Number(root.planet.cells||0)+Number(root.planet.colonies||0)*3+Number(root.planet.minds||0)*5):0
            for(var i=0;i<count;i++){
                var lon=(i*2.399963+Number(root.planet.id||1)*.31)+root.yaw
                var lat=Math.asin(-1+2*((i+.5)/Math.max(1,count)))+root.pitch*.22
                var x=Math.cos(lat)*Math.sin(lon), y=Math.sin(lat), z=Math.cos(lat)*Math.cos(lon)
                if(z<-.08)continue
                var px=cx+x*r,py=cy-y*r
                var rr=2.2+3.5*Math.max(0,z)
                ctx.fillStyle=i%7===0?"#ff9dd7":"#7ef0c2";ctx.globalAlpha=.35+.65*Math.max(0,z);ctx.beginPath();ctx.arc(px,py,rr,0,Math.PI*2);ctx.fill()
                if(i%5===0){ctx.strokeStyle="rgba(150,245,220,.18)";ctx.beginPath();ctx.arc(px,py,rr*2.3,0,Math.PI*2);ctx.stroke()}
            }
            ctx.globalAlpha=1

            // local satellites
            var moons=root.showSatellites?Math.min(8,Number(root.planet.moons||0)):0;
            for(var m=0;m<moons;m++){var ma=root.shipPhase*.22+m*Math.PI*2/Math.max(1,moons),mr=r*(1.12+.08*(m%3));ctx.strokeStyle="rgba(185,205,230,.10)";ctx.beginPath();ctx.arc(cx,cy,mr,0,Math.PI*2);ctx.stroke();ctx.fillStyle="#bac8d9";ctx.beginPath();ctx.arc(cx+Math.cos(ma)*mr,cy+Math.sin(ma)*mr,2.6,0,Math.PI*2);ctx.fill()}

            // orbiting ships
            var ships=root.showTransport?Math.min(12,Number(root.planet.ships||0)):0;
            for(var s=0;s<ships;s++){var a=root.shipPhase+s*Math.PI*2/Math.max(1,ships), ox=Math.cos(a)*r*1.28, oy=Math.sin(a)*r*.36;ctx.fillStyle="#cfe3ff";ctx.beginPath();ctx.arc(cx+ox,cy+oy,3,0,Math.PI*2);ctx.fill();ctx.strokeStyle="rgba(130,180,255,.24)";ctx.beginPath();ctx.moveTo(cx+ox,cy+oy);ctx.lineTo(cx+ox-Math.cos(a)*16,cy+oy-Math.sin(a)*5);ctx.stroke()}
        }
        MouseArea {
            anchors.fill: parent; acceptedButtons: Qt.LeftButton; hoverEnabled:true
            property real lx:0; property real ly:0
            onPressed:function(m){lx=m.x;ly=m.y}
            onPositionChanged:function(m){if(pressed){root.yaw+=(m.x-lx)*.008;root.pitch=Math.max(-1.2,Math.min(1.2,root.pitch+(m.y-ly)*.006));lx=m.x;ly=m.y;globe.requestPaint()}}
            onWheel:function(w){root.sphereZoom=Math.max(.55,Math.min(1.55,root.sphereZoom*(w.angleDelta.y>0?1.08:.92)));globe.requestPaint();w.accepted=true}
            cursorShape: pressed?Qt.ClosedHandCursor:Qt.OpenHandCursor
        }
    }

    Timer { interval:16;running:true;repeat:true;onTriggered:{root.shipPhase+=.008;globe.requestPaint()} }
    onYawChanged: globe.requestPaint(); onPitchChanged: globe.requestPaint(); onPlanetChanged: globe.requestPaint()

    Rectangle {
        anchors.left:parent.left;anchors.top:parent.top;anchors.bottom:parent.bottom;width:300;color:Theme.panel;border.color:Theme.border
        ColumnLayout { anchors.fill:parent;anchors.margins:20;spacing:12
            GlassButton{text:"Вернуться к космосу";icon:"←";onClicked:root.closeRequested()}
            RowLayout { Layout.fillWidth:true; GlassButton{Layout.fillWidth:true;text:sim.running?"Пауза":"Запуск";icon:sim.running?"Ⅱ":"▶";onClicked:sim.running=!sim.running} GlassButton{Layout.fillWidth:true;text:"+100";icon:"≫";onClicked:sim.fastForward(100)} }
            Text{text:"ПЛАНЕТА #"+(root.planet.id||"—");color:Theme.text;font.pixelSize:18;font.bold:true;font.letterSpacing:3}
            Text{text:"Интерактивная 3D-проекция";color:Theme.dim;font.pixelSize:10}
            Rectangle{Layout.fillWidth:true;height:1;color:Theme.border}
            Repeater { model:[
                ["Пригодность",Number(root.planet.habitability||0).toFixed(3)],
                ["Температура",Number(root.planet.temperature||0).toFixed(3)],
                ["Масса",Number(root.planet.mass||0).toFixed(2)],
                ["Орбита",Number(root.planet.orbitRadius||0).toFixed(1)],
                ["Население",root.planet.population||0],
                ["Клетки",root.planet.cells||0],["Колонии",root.planet.colonies||0],["Разум",root.planet.minds||0],["Спутники",root.planet.moons||0],["Корабли рядом",root.planet.ships||0],["Макс. поколение",root.planet.maxGeneration||0]
            ]; delegate:RowLayout{Layout.fillWidth:true;Text{text:modelData[0];color:Theme.muted;font.pixelSize:10;Layout.fillWidth:true}Text{text:modelData[1];color:Theme.text;font.pixelSize:10;font.family:"monospace"}} }
            Item{Layout.fillHeight:true}
            Text{Layout.fillWidth:true;wrapMode:Text.WordWrap;color:Theme.dim;font.pixelSize:9;lineHeight:1.35;text:"Перетаскивайте планету ЛКМ для вращения, колесом меняйте масштаб. Светящиеся точки — визуализация колоний и развитых агентов; корабли показываются на локальной орбите."}
        }
    }

    Rectangle {
        anchors.right:parent.right;anchors.top:parent.top;anchors.bottom:parent.bottom;width:300;color:Theme.panel;border.color:Theme.border
        ColumnLayout { anchors.fill:parent;anchors.margins:20;spacing:12
            Text{text:"БИОСФЕРА";color:Theme.accent;font.pixelSize:10;font.letterSpacing:3;font.bold:true}
            Rectangle{Layout.fillWidth:true;height:10;radius:5;color:Theme.backgroundAlt;Rectangle{width:parent.width*Math.min(1,Number(root.planet.biosphere||0));height:parent.height;radius:5;color:Theme.accent2}}
            Text{Layout.fillWidth:true;wrapMode:Text.WordWrap;color:Theme.muted;font.pixelSize:10;text:Number(root.planet.minds||0)>0?"Появились агенты с памятью и устойчивой системой выбора.":Number(root.planet.colonies||0)>0?"Колонии конкурируют за ресурс и накапливают технологический уровень.":Number(root.planet.cells||0)>0?"Идёт наследование сенсорно-моторных весов и отбор по энергетической пригодности.":"Стабильной биосферы пока нет."}
            Rectangle{Layout.fillWidth:true;height:1;color:Theme.border}
            Text{text:"СЛОИ ВИЗУАЛИЗАЦИИ";color:Theme.accent;font.pixelSize:10;font.letterSpacing:3;font.bold:true}
            CheckBox{text:"Колонии";checked:root.showColonies;onToggled:{root.showColonies=checked;globe.requestPaint()}} CheckBox{text:"Спутники";checked:root.showSatellites;onToggled:{root.showSatellites=checked;globe.requestPaint()}} CheckBox{text:"Транспорт";checked:root.showTransport;onToggled:{root.showTransport=checked;globe.requestPaint()}} CheckBox{text:"Сетка координат";checked:root.showGrid;onToggled:{root.showGrid=checked;globe.requestPaint()}}
            Item{Layout.fillHeight:true}
            Text{Layout.fillWidth:true;wrapMode:Text.WordWrap;color:Theme.dim;font.pixelSize:9;text:"В текущем прототипе поверхность визуализируется как лёгкая программная 3D-проекция. Это сохраняет низкие требования к GPU и не добавляет обязательную зависимость Qt Quick 3D."}
        }
    }
}
