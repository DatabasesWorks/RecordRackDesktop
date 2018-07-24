#ifndef DEBTORUTILS_H
#define DEBTORUTILS_H

#include <QString>
#include <QDateTime>

struct DebtPayment {
    double amount = 0.0;
    QString note;
}; Q_DECLARE_TYPEINFO(DebtPayment, Q_PRIMITIVE_TYPE);

struct DebtTransaction {
    double totalDebt = 0.0;
    QDateTime dueDateTime;
    QString note;
    QList<DebtPayment *> debtPayments;
}; Q_DECLARE_TYPEINFO(DebtTransaction, Q_PRIMITIVE_TYPE);

#endif // DEBTORUTILS_H
