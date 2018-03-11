import QtQuick 2.9
import QtQuick.Controls 2.2 as QQC2
import QtQuick.Controls.Material 2.3
import Fluid.Controls 1.0 as FluidControls
import "../rrui" as RRUi
import com.gecko.rr.models 1.0 as RRModels

ListView {
    id: saleCartListView

    readonly property bool busy: model.busy
    property alias customerName: saleCartModel.customerName
    property alias customerPhoneNumber: saleCartModel.customerPhoneNumber
    property alias transactionId: saleCartModel.transactionId
    property alias note: saleCartModel.note
    readonly property real totalCost: model.totalCost
    property real amountPaid: model.amountPaid

    signal editRequested(var itemInfo)
    signal success(int successCode)
    signal error(int errorCode)

    spacing: 10
    clip: true

    topMargin: 4
    bottomMargin: 10

    model: RRModels.SaleCartModel {
        id: saleCartModel
        onSuccess: saleCartListView.success(successCode);
        onError: saleCartListView.error(errorCode);
    }

    delegate: Item {
        width: ListView.view.width
        height: delegateColumn.height + quantitySpinBox.height + 10

        Column {
            id: delegateColumn
            anchors {
                left: parent.left
                top: parent.top
            }

            spacing: 2

            FluidControls.SubheadingLabel {
                text: item
            }

            FluidControls.CaptionLabel {
                text: "From <i>" + category + "</i><br/>" + available_quantity + " " + unit + " left<br/>Costs " + Number(unit_price).toLocaleCurrencyString(Qt.locale("en_NG")) + " each"
            }
        }

        Row {
            spacing: 0
            anchors {
                right: parent.right
                top: parent.top
            }

            FluidControls.ToolButton {
                id: editButton
                width: FluidControls.Units.iconSizes.medium
                height: width
                icon.name: "image/edit"
                onClicked: saleCartListView.editRequested({ "item_id": item_id, "quantity": quantity, "unit_price": unit_price });
            }

            FluidControls.ToolButton {
                id: deleteButton
                width: FluidControls.Units.iconSizes.medium
                height: width
                icon.name: "action/delete"
                onClicked: saleCartListView.model.removeItem(item_id);
            }
        }

        FluidControls.SubheadingLabel {
            anchors {
                right: parent.right
                bottom: parent.bottom
            }

            text: Number(cost).toLocaleCurrencyString(Qt.locale("en_NG"))
        }

        QuantitySpinBox {
            id: quantitySpinBox
            quantity: model.quantity
            maximumQuantity: model.available_quantity
            unit: model.unit
            anchors {
                bottomMargin: 4
                bottom: parent.bottom
            }

            onQuantityUpdated: saleCartListView.model.setItemQuantity(model.item_id, newQuantity);
        }

        Rectangle {
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            color: "lightgray"
            height: 1
            visible: index < saleCartListView.count - 1
        }
    }

    function addItem(itemInfo) { saleCartListView.model.addItem(itemInfo); }
    function submitTransaction() { saleCartListView.model.submitTransaction(); }
    function suspendTransaction(params) { saleCartListView.model.suspendTransaction(params); }
    function undoLastTransaction() { saleCartListView.model.undoLastCommit(); }
    function clearAll() { saleCartListView.model.clearAll(); }
}