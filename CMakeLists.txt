cmake_minimum_required(VERSION 3.16)
project(AfterLife VERSION 0.2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 Network)
qt_standard_project_setup()
set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_executable(appAfterLife
    main.cpp entity.h physics.h physics.cpp scenario.h scenario.cpp metrics.h metrics.cpp
    universe.h universe.cpp simulationworker.h simulationworker.cpp simulator.h simulator.cpp
    universerenderer.h universerenderer.cpp particlefield.h particlefield.cpp
    crystalfield.h crystalfield.cpp localassistant.h localassistant.cpp
)
qt_add_qml_module(appAfterLife
    URI AfterLife VERSION 1.0
    QML_FILES
        Main.qml MainMenu.qml MenuButton.qml SimulationSelectScreen.qml GalaxyScreen.qml
        CollapsiblePanel.qml ConfigPanel.qml MetricsPanel.qml LogPanel.qml Theme.qml
        HelpDot.qml HistoryChart.qml PlanetDetail.qml AssistantPanel.qml CrystalScreen.qml
        CrystalConfigPanel.qml GlassButton.qml
)
set_target_properties(appAfterLife PROPERTIES MACOSX_BUNDLE TRUE WIN32_EXECUTABLE TRUE)
target_link_libraries(appAfterLife PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::Network)
