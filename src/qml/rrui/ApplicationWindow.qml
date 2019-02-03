import QtQuick 2.12
import QtQuick.Controls.Material 2.3
import Fluid.Controls 1.0 as FluidControls
import com.gecko.rr 1.0 as RR
import "../rrui" as RRUi

FluidControls.ApplicationWindow {
    id: applicationWindow

    readonly property alias snackBar: snackBar

    width: 640
    height: 480
    visible: true
    title: qsTr("Record Rack")
    appBar.visible: false
    color: settings.darkModeActive ? Material.color(Material.Grey, Material.Shade900) : Qt.lighter("lightgray")

    Material.theme: settings.darkModeActive ? Material.Dark : Material.Light
    Material.accent: Material.LightGreen

    RRUi.SnackBar { id: snackBar }
    RR.Settings { id: settings }

    RR.TapTargetFinder {
        tapTargets: applicationWindow.pageStack.currentItem.tapTargets
        onFound: {
            tapTargetViewLoader.active = true;
            tapTargetViewLoader.item.queueTarget(tapTarget);
        }
        onCleared: tapTargetViewLoader.active = false;
    }

    Loader {
        id: tapTargetViewLoader
        anchors.fill: parent
        active: false
        source: Qt.resolvedUrl("TapTargetView.qml")
        z: 10
    }
}
