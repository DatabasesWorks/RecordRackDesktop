import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.3
import Fluid.Controls 1.0 as FluidControls
import com.gecko.rr.models 1.0 as RRModels

FluidControls.Page {
    title: qsTr("Dashboard")
    topPadding: 0
    bottomPadding: 0
    leftPadding: 20
    rightPadding: 20

    ListView {
        anchors.fill: parent
        topMargin: 20
        bottomMargin: 20

        //model: RR.DashboardHomeModel { }

        delegate: FluidControls.Card {
            width: ListView.view.width
            height: 40
        }
    }
}