import QtQuick
import QtQuick.Controls
import AfterLife

ApplicationWindow {
    id: window
    visible: true
    width: 1480; height: 920
    minimumWidth: 1080; minimumHeight: 700
    title: "AfterLife — evolutionary systems laboratory"
    color: Theme.background
    palette.window: Theme.background
    palette.base: Theme.backgroundAlt
    palette.text: Theme.text
    palette.windowText: Theme.text
    palette.buttonText: Theme.text
    palette.highlight: Theme.accent
    palette.highlightedText: Theme.background

    StackView {
        id: stack; anchors.fill: parent
        initialItem: mainMenuComponent
        pushEnter: Transition { ParallelAnimation { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 260 } NumberAnimation { property: "scale"; from: .985; to: 1; duration: 320; easing.type: Easing.OutCubic } } }
        pushExit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 180 } }
        popEnter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 220 } }
        popExit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 180 } }
    }

    Component { id: mainMenuComponent; MainMenu { onStartSimulation: stack.push(selectComponent); onQuit: Qt.quit() } }
    Component {
        id: selectComponent
        SimulationSelectScreen {
            onBackToMenu: stack.pop()
            onSimulationChosen: function(key) {
                if (key === "galaxy") stack.push(galaxyComponent)
                else if (key === "crystal") stack.push(crystalComponent)
            }
        }
    }
    Component {
        id: galaxyComponent
        GalaxyScreen {
            onBackToSelect: { sim.running = false; stack.pop() }
            Component.onCompleted: { sim.running = false; sim.reset() }
        }
    }
    Component { id: crystalComponent; CrystalScreen { onBackToSelect: stack.pop() } }
}
