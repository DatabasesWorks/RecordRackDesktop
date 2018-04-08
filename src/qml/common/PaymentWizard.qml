import QtQuick 2.10
import QtQuick.Controls 2.2 as QQC2
import Fluid.Controls 1.0 as FluidControls
import QtQuick.Layouts 1.3 as QQLayouts
import "paymentwizard"

QQC2.Dialog {
    id: paymentWizard

    property real totalCost: 0
    property real amountPaid: 0
    property real balance: 0
    property string customerName: ""
    property string customerPhoneNumber: ""
    property date dueDate: new Date()
    property bool overlookBalance: false
    property bool giveChange: false

    signal finished(var paymentInfo)

    x: (QQC2.ApplicationWindow.contentItem.width - width) / 2
    y: (QQC2.ApplicationWindow.contentItem.height - height) / 2
    parent: QQC2.ApplicationWindow.contentItem

    width: 600
    height: 540

    focus: true
    modal: true
    closePolicy: QQC2.Popup.CloseOnEscape

    QtObject {
        id: privateProperties

        readonly property bool acceptCash: cashPayments < maxCashPayments
        readonly property bool acceptCard: cardPayments < maxCardPayments
        readonly property int maxCashPayments: 1
        readonly property int maxCardPayments: 2

        property int cashPayments: 0
        property int cardPayments: 0
    }

    QQLayouts.ColumnLayout {
        anchors.fill: parent

        Row {
            QQLayouts.Layout.preferredHeight: 25
            FluidControls.ToolButton {
                anchors.verticalCenter: parent.verticalCenter
                width: FluidControls.Units.iconSizes.large
                height: width
                icon.name: "navigation/arrow_back"
                visible: stackView.depth > 1 && stackView.currentObjectName != "paymentFinishPage"
                onClicked: {
                    if (stackView.currentObjectName == "paymentBalancePage" && stackView.currentItem.isCashPayment && privateProperties.cashPayments > 0) {
                        paymentWizard.overlookBalance = false;
                        paymentWizard.balance = stackView.currentItem.balance;
                        privateProperties.cashPayments--;
                    }
                    else if (stackView.currentObjectName == "paymentBalancePage" && !stackView.currentItem.isCashPayment && privateProperties.cardPayments > 0) {
                        paymentWizard.overlookBalance = false;
                        paymentWizard.balance = stackView.currentItem.balance;
                        privateProperties.cardPayments--;
                    }

                    stackView.pop();
                }
            }

            FluidControls.DialogLabel {
                id: dialogLabel
                anchors.verticalCenter: parent.verticalCenter

                wrapMode: Text.Wrap
                text: stackView.currentItem != null ? stackView.currentItem.title : ""
            }
        }

        QQC2.StackView {
            id: stackView

            property string currentObjectName: currentItem != null ? currentItem.objectName : ""

            QQLayouts.Layout.fillWidth: true
            QQLayouts.Layout.fillHeight: true
            clip: true
            initialItem: PaymentMethodPage { }
        }

        Row {
            spacing: 8
            QQLayouts.Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            QQC2.CheckBox {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Print receipt")
                visible: stackView.currentObjectName === "paymentSummaryPage"
            }

            QQC2.Button {
                flat: true
                text: qsTr("Cancel")
                onClicked: paymentWizard.reject();
            }

            QQC2.Button {
                visible: stackView.currentObjectName != "paymentFinishPage"
                enabled: stackView.currentObjectName != "paymentCustomerDetailPage" || (stackView.currentObjectName == "paymentCustomerDetailPage" && stackView.currentItem.detailValid)
                text: {
                    if (stackView.currentObjectName == "paymentMethodPage") {
                        switch (stackView.currentItem.selectedOption) {
                        case "cash":
                        case "none":
                            return qsTr("Next");
                        case "card":
                            return qsTr("Pay");
                        }
                    } else if (stackView.currentObjectName == "paymentByCashPage") {
                        return qsTr("Pay");
                    } else if (stackView.currentObjectName == "paymentBalancePage") {
                        return qsTr("Next");
                    } else if (stackView.currentObjectName == "paymentCustomerDetailPage") {
                        return qsTr("Next");
                    } else if (stackView.currentObjectName == "paymentDueDatePage") {
                        return qsTr("Next");
                    }

                    return qsTr("Finish");
                }

                onClicked: {
                    if (stackView.currentObjectName == "paymentMethodPage") {
                        switch (stackView.currentItem.selectedOption) {
                        case "cash":
                            stackView.push(paymentByCashPage);
                            break;
                        case "card":
                            break;
                        default:
                            paymentWizard.amountPaid = 0;
                            paymentWizard.balance = paymentWizard.totalCost;
                            stackView.push(paymentCustomerDetailPage);
                            break;
                        }
                    } else if (stackView.currentObjectName == "paymentByCashPage") {
                        privateProperties.cashPayments++;

                        if (stackView.currentItem.hasDebt || stackView.currentItem.hasCredit) {
                            paymentWizard.amountPaid = stackView.currentItem.amountPaid;
                            paymentWizard.balance = stackView.currentItem.balance;
                            stackView.push(paymentBalancePage, {
                                               "hasDebt": stackView.currentItem.hasDebt,
                                               "hasCredit": stackView.currentItem.hasCredit,
                                               "balance": stackView.currentItem.balance,
                                               "acceptAlternatePaymentMethod": stackView.currentItem.amountPaid > 0,
                                               "isCashPayment": true,
                                           });
                        } else if ((paymentWizard.customerName.trim() === "" || paymentWizard.customerPhoneNumber.trim() === "")
                                   && (stackView.currentItem.hasDebt || stackView.currentItem.hasCredit)) {
                            stackView.push(paymentCustomerDetailPage);
                        } else {
                            stackView.push(paymentSummaryPage);
                        }
                    } else if (stackView.currentObjectName == "paymentByCardPage") {
                        privateProperties.cardPayments++;

                        if (stackView.currentItem.hasDebt || stackView.currentItem.hasCredit) {
                            paymentWizard.balance = stackView.currentItem.balance;
                            stackView.push(paymentBalancePage, {
                                               "hasDebt": stackView.currentItem.hasDebt,
                                               "hasCredit": stackView.currentItem.hasCredit,
                                               "balance": stackView.currentItem.balance,
                                               "acceptAlternatePaymentMethod": stackView.currentItem.amountPaid > 0,
                                               "isCashPayment": false,
                                           });
                        } else if (paymentWizard.customerName.trim() === "" || paymentWizard.customerPhoneNumber.trim() === "") {
                            stackView.push(paymentCustomerDetailPage);
                        } else {
                            stackView.push(paymentSummaryPage);
                        }
                    } else if (stackView.currentObjectName == "paymentBalancePage") {
                        switch (stackView.currentItem.selectedOption) {
                        case "pay_another_way":
                            if (privateProperties.acceptCard || privateProperties.acceptCash) {
                                stackView.push(paymentMethodPage, {
                                                   "acceptCash": privateProperties.acceptCash,
                                                   "acceptCard": privateProperties.acceptCard
                                               });
                            }
                            break;
                        case "overlook_balance":
                            paymentWizard.overlookBalance = true;
                            stackView.push(paymentFinishPage);
                            break;
                        case "give_change":
                            paymentWizard.giveChange = true;
                            stackView.push(paymentChangePage, { "changeDue": stackView.currentItem.balance });
                            break;
                        case "create_debtor":
                            if (paymentWizard.customerName.trim() === "" || paymentWizard.customerPhoneNumber.trim() === "")
                                stackView.push(paymentCustomerDetailPage);
                            else
                                stackView.push(paymentDueDatePage);
                            break;
                        case "create_creditor":
                            if (paymentWizard.customerName.trim() === "" || paymentWizard.customerPhoneNumber.trim() === "")
                                stackView.push(paymentCustomerDetailPage);
                            else
                                stackView.push(paymentDueDatePage);
                            break;
                        }
                    } else if (stackView.currentObjectName == "paymentCustomerDetailPage") {
                        paymentWizard.customerName = stackView.currentItem.customerName;
                        paymentWizard.customerPhoneNumber = stackView.currentItem.customerPhoneNumber;

                        if (paymentWizard.balance != 0)
                            stackView.push(paymentDueDatePage);
                        else
                            stackView.push(paymentSummaryPage);
                    } else if (stackView.currentObjectName == "paymentDueDatePage") {
                        paymentWizard.dueDate = stackView.currentItem.dueDate;
                        stackView.push(paymentSummaryPage);
                    } else if (stackView.currentObjectName == "paymentSummaryPage" || stackView.currentObjectName == "paymentChangePage") {
                        stackView.push(paymentFinishPage);
                        // if (printCheckBox.checked)
                        //     RRWidgets.print(paymentInfo);
                        paymentWizard.finished({ "client_id": -1,
                                                   "customer_name": paymentWizard.customerName,
                                                   "customer_phone_number": paymentWizard.customerPhoneNumber,
                                                   "total_cost": paymentWizard.totalCost,
                                                   "amount_paid": paymentWizard.amountPaid,
                                                   "balance": paymentWizard.balance,
                                                   "due_date": paymentWizard.dueDate,
                                                   "overlook_balance": paymentWizard.overlookBalance,
                                                   "give_change": paymentWizard.giveChange
                                               });
                    } else if (stackView.currentObjectName == "paymentFinishPage") {
                    }
                }
            }
        }
    }

    Component {
        id: paymentMethodPage

        PaymentMethodPage { }
    }

    Component {
        id: paymentByCashPage

        PaymentByCashPage { totalCost: paymentWizard.totalCost }
    }

    Component {
        id: paymentByCardPage

        PaymentByCardPage { }
    }

    Component {
        id: paymentBalancePage

        PaymentBalancePage { totalCost: paymentWizard.totalCost }
    }

    Component {
        id: paymentDebtorPage

        PaymentDebtorPage { }
    }

    Component {
        id: paymentCreditorPage

        PaymentCreditorPage { }
    }

    Component {
        id: paymentCustomerDetailPage

        PaymentCustomerDetailPage { customerName: paymentWizard.customerName; customerPhoneNumber: paymentWizard.customerPhoneNumber }
    }

    Component {
        id: paymentDueDatePage

        PaymentDueDatePage { }
    }

    Component {
        id: paymentSummaryPage

        PaymentSummaryPage {
            customerName: paymentWizard.customerName
            customerPhoneNumber: paymentWizard.customerPhoneNumber
            totalCost: paymentWizard.totalCost
            amountPaid: paymentWizard.amountPaid
        }
    }

    Component {
        id: paymentChangePage

        PaymentChangePage { }
    }

    Component {
        id: paymentFinishPage

        PaymentFinishPage { }
    }

    function displayError(title, message) {

    }
}
