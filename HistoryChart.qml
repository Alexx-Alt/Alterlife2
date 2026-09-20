import QtQuick
import AfterLife
Canvas {
    id:root
    property bool useReplay: false
    Connections { target:sim; function onTick(){root.requestPaint()} }
    onPaint:{var ctx=getContext("2d");ctx.fillStyle=Theme.backgroundAlt;ctx.fillRect(0,0,width,height);var h=root.useReplay && sim.replayAvailable ? sim.replayHistory() : sim.historySnapshot();if(h.length<2)return;function line(key,stroke,maxv){ctx.beginPath();ctx.strokeStyle=stroke;ctx.lineWidth=1.4;for(var i=0;i<h.length;i++){var x=i/(h.length-1)*(width-12)+6;var v=Number(h[i][key]);var y=height-8-Math.max(0,Math.min(1,v/maxv))*(height-16);if(i===0)ctx.moveTo(x,y);else ctx.lineTo(x,y)}ctx.stroke()}line("entropy",Theme.accent,1);line("decisionIndex",Theme.accent2,1);var maxE=1;for(var j=0;j<h.length;j++)maxE=Math.max(maxE,Number(h[j].entities));line("entities",Theme.warning,maxE);ctx.font="9px sans-serif";ctx.fillStyle=Theme.muted;ctx.fillText("entropy",8,13);ctx.fillStyle=Theme.accent2;ctx.fillText("choice",62,13);ctx.fillStyle=Theme.warning;ctx.fillText("entities",105,13)}
}
