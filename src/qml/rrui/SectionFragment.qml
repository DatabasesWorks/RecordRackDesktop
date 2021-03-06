import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Controls.Material 2.3
import Fluid.Controls 1.0 as FluidControls
import Fluid.Core 1.0 as FluidCore
import com.gecko.rr.models 1.0 as RRModels
import "../rrui" as RRUi
import "../common"

Item {
    id: sectionFragment

    property string title: ""
    property bool dividerVisible: true
    default property alias content: dataColumn.data

    implicitWidth: 200
    implicitHeight: column.height + divider.height

    Column {
        id: column
        anchors {
            left: parent.left
            right: parent.right
        }

        spacing: 12

        FluidControls.HeadlineLabel {
            anchors {
                left: parent.left
                right: parent.right
            }

            text: sectionFragment.title
        }

        Column {
            id: dataColumn
            anchors {
                left: parent.left
                right: parent.right
            }
        }

        Rectangle {
            id: divider
            anchors {
                left: parent.left
                right: parent.right
            }
            height: 1
            color: "lightgray"
            visible: sectionFragment.dividerVisible
        }
    }
}
