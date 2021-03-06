import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12 as QQLayouts
import QtQuick.Controls.Material 2.3
import Fluid.Controls 1.0 as FluidControls
import "../../rrui" as RRUi
import com.gecko.rr.models 1.0 as RRModels
import "../../singletons"

ListView {
    id: creditorListView

    property Component buttonRow: null
    property string filterText: ""
    property int filterColumn: -1
    property bool autoQuery: true

    signal success(int successCode)
    signal error(int errorCode)

    topMargin: 20
    bottomMargin: 20
    clip: true
    visible: !model.busy

    model: RRModels.CreditorModel {
        filterText: creditorListView.filterText
        filterColumn: creditorListView.filterColumn
        autoQuery: creditorListView.autoQuery
        onSuccess: creditorListView.success(successCode);
        onError: creditorListView.error(errorCode);
    }

    QQC2.ScrollBar.vertical: QQC2.ScrollBar {
        policy: QQC2.ScrollBar.AlwaysOn
        visible: creditorListView.contentHeight > creditorListView.height
        size: .3
        width: 5
        contentItem: Rectangle {
            color: Material.color(Material.Grey)
            radius: width / 2
        }
    }

    delegate: FluidControls.ListItem {
        width: ListView.view.width
        height: 40
        showDivider: true

        QQLayouts.RowLayout {
            anchors.fill: parent
            spacing: 16

            RRUi.LetterCircleImage {
                QQLayouts.Layout.alignment: Qt.AlignVCenter
                name: preferred_name
            }

            FluidControls.SubheadingLabel {
                QQLayouts.Layout.alignment: Qt.AlignVCenter
                QQLayouts.Layout.fillWidth: true
                text: preferred_name
            }

            Row {
                spacing: 8

                FluidControls.SubheadingLabel {
                    text: Number(model.total_credit).toLocaleCurrencyString(Qt.locale(GlobalSettings.currencyLocaleName))
                    verticalAlignment: Qt.AlignVCenter
                    anchors.verticalCenter: parent.verticalCenter
                }

                Loader {
                    id: rightButtonLoader

                    readonly property var modelData: {
                        "client_id": model.client_id,
                        "creditor_id": model.creditor_id,
                        "preferred_name": model.preferred_name
                    }

                    QQLayouts.Layout.fillHeight: true
                    QQLayouts.Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    sourceComponent: creditorListView.buttonRow
                }
            }
        }
    }

    add: Transition {
        NumberAnimation { property: "y"; from: 100; duration: 300; easing.type: Easing.OutCubic }
        NumberAnimation { property: "opacity"; to: 1; duration: 300; easing.type: Easing.OutCubic }
    }

    remove: Transition {
        NumberAnimation { property: "opacity"; to: 0; duration: 300; easing.type: Easing.OutCubic }
    }

    removeDisplaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 300 }
    }

    function refresh() { creditorListView.model.refresh(); }
    function undoLastCommit() { creditorListView.model.undoLastCommit(); }
}
