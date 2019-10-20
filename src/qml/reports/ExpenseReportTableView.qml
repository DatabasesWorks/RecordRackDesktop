import QtQuick 2.12
import Qt.labs.qmlmodels 1.0
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Controls.Material 2.3
import Fluid.Controls 1.0 as FluidControls
import com.gecko.rr.models 1.0 as RRModels
import "../rrui" as RRUi
import "../singletons"

RRUi.DataTableView {
    id: expenseReportTableView

    property alias busy: expenseReportModel.busy
    property alias autoQuery: expenseReportModel.autoQuery
    property Component buttonRow: null

    signal success(int successCode)
    signal error(int errorCode)

    function refresh() { expenseReportModel.refresh(); }

    flickableDirection: TableView.VerticalFlick

    model: RRModels.ExpenseReportModel {
        id: expenseReportModel
        onSuccess: expenseReportTableView.success(successCode);
        onError: expenseReportTableView.error(errorCode);
    }

    QQC2.ScrollBar.vertical: RRUi.ScrollBar {
        policy: QQC2.ScrollBar.AlwaysOn
        visible: expenseReportTableView.contentHeight > expenseReportTableView.height
    }

    delegate: DelegateChooser {
        DelegateChoice {
            column: RRModels.ExpenseReportModel.PurposeColumn
            delegate: RRUi.TableDelegate {
                implicitWidth: expenseReportTableView.columnHeader.children[RRModels.ExpenseReportModel.PurposeColumn].width
                implicitHeight: expenseReportTableView.rowHeader.children[0].height

                FluidControls.SubheadingLabel {
                    anchors {
                        left: parent.left
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }

                    elide: Text.ElideRight
                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter
                    text: purpose
                }
            }
        }

        DelegateChoice {
            column: RRModels.ExpenseReportModel.AmountColumn
            delegate: RRUi.TableDelegate {
                implicitWidth: expenseReportTableView.columnHeader.children[RRModels.ExpenseReportModel.AmountColumn].width
                implicitHeight: expenseReportTableView.rowHeader.children[0].height

                FluidControls.SubheadingLabel {
                    anchors {
                        left: parent.left
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }

                    elide: Text.ElideRight
                    horizontalAlignment: Qt.AlignRight
                    verticalAlignment: Qt.AlignVCenter
                    text: Number(amount).toLocaleCurrencyString(Qt.locale(GlobalSettings.currencyLocaleName))
                }
            }
        }

        DelegateChoice {
            column: RRModels.ExpenseReportModel.ActionColumn
            delegate: RRUi.TableDelegate {
                implicitWidth: expenseReportTableView.columnHeader.children[RRModels.ExpenseReportModel.ActionColumn].width
                implicitHeight: expenseReportTableView.rowHeader.children[0].height

                Loader {
                    readonly property var modelData: {
                        "client_id": model.client_id,
                        "transaction_id": model.transaction_id
                    }

                    anchors.centerIn: parent
                    sourceComponent: expenseReportTableView.buttonRow
                }
            }
        }
    }
}
