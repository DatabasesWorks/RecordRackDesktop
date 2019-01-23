import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.3 as QQLayouts
import QtQuick.Controls.Material 2.3
import Fluid.Controls 1.0 as FluidControls
import "../rrui" as RRUi
import com.gecko.rr.models 1.0 as RRModels
import "../singletons"

TableView {
    id: incomeTransactionTableView

    property Component buttonRow: null
    property string filterText: ""
    property int filterColumn: -1
    property int keys: RRModels.IncomeTransactionModel.Completed
    property date from: new Date()
    property date to: new Date()
    property var columnWidths: [ 40, width - 280, 100, 120 ]

    enum Columns {
        TransactionIdColumn,
        ClientNameColumn,
        AmountColumn,
        ActionColumn
    }

    signal success(int successCode)
    signal error(int errorCode)

    function removeTransaction(transactionId) { incomeTransactionTableView.model.removeTransaction(transactionId); }

    bottomMargin: 20
    clip: true
    visible: !model.busy
    columnWidthProvider: function (column) { return columnWidths[column]; }
    flickableDirection: TableView.VerticalFlick
    columnSpacing: 8

    FluidControls.Placeholder {
        visible: incomeTransactionTableView.rows == 0 && !incomeTransactionTableView.model.busy
        anchors.centerIn: parent
        icon.source: Qt.resolvedUrl("qrc:/icons/cash-multiple.svg")
        text: qsTr("No transactions took place on this day.")
    }

    QQC2.BusyIndicator {
        anchors.centerIn: parent
        visible: incomeTransactionTableView.model.busy
    }

    model: RRModels.IncomeTransactionModel {
        filterText: incomeTransactionTableView.filterText
        filterColumn: incomeTransactionTableView.filterColumn
        keys: incomeTransactionTableView.keys
        from: incomeTransactionTableView.from
        to: incomeTransactionTableView.to
        onSuccess: incomeTransactionTableView.success(successCode);
        onError: incomeTransactionTableView.error(errorCode);
    }

    QQC2.ScrollBar.vertical: QQC2.ScrollBar {
        policy: QQC2.ScrollBar.AlwaysOn
        visible: incomeTransactionTableView.contentHeight > incomeTransactionTableView.height
        size: .3
        width: 5
        contentItem: Rectangle {
            color: Material.color(Material.Grey)
            radius: width / 2
        }
    }

    delegate: Item {
        implicitWidth: label.visible ? label.implicitWidth : actionRow.width
        implicitHeight: 40

        FluidControls.SubheadingLabel {
            id: label
            anchors.fill: parent
            horizontalAlignment: {
                switch (column) {
                case IncomeTransactionTableView.Columns.TransactionIdColumn:
                    Qt.AlignRight
                    break;
                case IncomeTransactionTableView.Columns.ClientNameColumn:
                    Qt.AlignLeft
                    break;
                case IncomeTransactionTableView.Columns.AmountColumn:
                    Qt.AlignRight
                    break;
                default:
                    undefined
                }
            }

            verticalAlignment: Qt.AlignVCenter

            visible: column !== IncomeTransactionTableView.Columns.ActionColumn
            text: {
                switch (column) {
                case IncomeTransactionTableView.Columns.TransactionIdColumn:
                    transaction_id
                    break;
                case IncomeTransactionTableView.Columns.ClientNameColumn:
                    client_name
                    break;
                case IncomeTransactionTableView.Columns.AmountColumn:
                    Number(amount).toLocaleCurrencyString(Qt.locale(GlobalSettings.currencyLocaleName))
                    break;
                default:
                    ""
                }
            }
        }

        Loader {
            id: actionRow
            visible: column === IncomeTransactionTableView.Columns.ActionColumn
            anchors.centerIn: parent

            readonly property var modelData: {
                "client_id": model.client_id,
                "transaction_id": model.transaction_id
            }

            sourceComponent: incomeTransactionTableView.buttonRow
        }
    }

    function refresh() { incomeTransactionTableView.model.refresh(); }
    function undoLastCommit() { incomeTransactionTableView.model.undoLastCommit(); }
}
