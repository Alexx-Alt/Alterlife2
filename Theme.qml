pragma Singleton
import QtQuick
QtObject {
    property string mode: "dark"
    readonly property bool light: mode === "light"
    readonly property bool liquid: mode === "liquid"
    property color background: light ? "#edf2f7" : liquid ? "#07111f" : "#03060c"
    property color backgroundAlt: light ? "#f7f9fc" : liquid ? "#0b1828" : "#07101b"
    property color panel: light ? Qt.rgba(1,1,1,.88) : liquid ? Qt.rgba(.08,.13,.20,.56) : Qt.rgba(.025,.045,.075,.92)
    property color panelStrong: light ? "#ffffff" : liquid ? Qt.rgba(.08,.16,.25,.78) : "#09111c"
    property color border: light ? "#cbd5e1" : liquid ? Qt.rgba(.63,.82,1,.22) : "#1b2b3d"
    property color text: light ? "#122033" : "#e8f2ff"
    property color muted: light ? "#5d6b7d" : "#8197af"
    property color dim: light ? "#8793a3" : "#51677e"
    property color accent: light ? "#286aa6" : "#9fcfff"
    property color accent2: light ? "#2a8f73" : "#7ef0c2"
    property color warning: "#f0b36b"
    property color danger: "#ff7d91"
    property real panelRadius: liquid ? 20 : 12
    property real glassOpacity: liquid ? .64 : .94
    function setTheme(name) { mode = name }
}
