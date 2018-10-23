#include "abstracttransactionmodel.h"
#include <QDateTime>

#include "database/queryrequest.h"
#include "database/queryresult.h"

AbstractTransactionModel::AbstractTransactionModel(QObject *parent) :
    AbstractVisualListModel(parent),
    m_transactionId(-1),
    m_keys(None)
{

}

AbstractTransactionModel::AbstractTransactionModel(DatabaseThread &thread) :
    AbstractVisualListModel(thread),
    m_transactionId(-1),
    m_keys(None)
{

}

int AbstractTransactionModel::keys() const
{
    return m_keys;
}

void AbstractTransactionModel::setKeys(int keys)
{
    if (m_keys == keys)
        return;

    m_keys = keys;
    emit keysChanged();
}

QDateTime AbstractTransactionModel::from() const
{
    return m_from;
}

void AbstractTransactionModel::setFrom(const QDateTime &from)
{
    if (m_from == from)
        return;

    m_from = from;
    emit fromChanged();
}

QDateTime AbstractTransactionModel::to() const
{
    return m_to;
}

void AbstractTransactionModel::setTo(const QDateTime &to)
{
    if (m_to == to)
        return;

    m_to = to;
    emit toChanged();
}

void AbstractTransactionModel::componentComplete()
{
    toggleConnections();
    AbstractVisualListModel::componentComplete();
}

void AbstractTransactionModel::toggleConnections()
{
    if (autoQuery()) {
        connect(this, &AbstractTransactionModel::fromChanged, this, &AbstractTransactionModel::tryQuery);
        connect(this, &AbstractTransactionModel::toChanged, this, &AbstractTransactionModel::tryQuery);
    } else {
        disconnect(this, &AbstractTransactionModel::fromChanged, this, &AbstractTransactionModel::tryQuery);
        disconnect(this, &AbstractTransactionModel::toChanged, this, &AbstractTransactionModel::tryQuery);
    }
}
