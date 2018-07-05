#include "qmlsalehomemodel.h"
#include "database/queryrequest.h"
#include "database/queryresult.h"

#include "models/saletotalrevenuemodel.h"
#include "models/salemostsolditemmodel.h"

QMLSaleHomeModel::QMLSaleHomeModel(QObject *parent) :
    AbstractVisualListModel(parent)
{

}

int QMLSaleHomeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_records.count();
}

QVariant QMLSaleHomeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case DataTypeRole:
        return m_records.at(index.row()).toMap().value("data_type").toString();
    case DataModelRole:
        return QVariant::fromValue<QObject *>(m_dataModels.at(index.row()));
    }

    return QVariant();
}

QHash<int, QByteArray> QMLSaleHomeModel::roleNames() const
{
    QHash<int, QByteArray> roles(AbstractVisualListModel::roleNames());
    roles.insert(DataTypeRole, "data_type");
    roles.insert(DataModelRole, "data_model");

    return roles;
}


void QMLSaleHomeModel::tryQuery()
{
    setBusy(true);

    QueryRequest request(this);
    request.setCommand("view_sale_home", QVariantMap(), QueryRequest::Sales);

    emit executeRequest(request);
}

void QMLSaleHomeModel::processResult(const QueryResult result)
{
    if (result.request().receiver() != this)
        return;

    setBusy(false);

    if (result.isSuccessful()) {
        beginResetModel();
        qDeleteAll(m_dataModels);
        m_dataModels.clear();

        m_records = result.outcome().toMap().value("records").toList();

        for (const QVariant &r : m_records) {
            const QVariantMap record = r.toMap();

            if (record.value("data_type").toString() == "total_revenue") {
                SaleTotalRevenueModel *model = new SaleTotalRevenueModel(record.value("data_model").toList(), this);
                m_dataModels.append(model);
            } else if (record.value("data_type").toString() == "most_sold_items") {
                SaleMostSoldItemModel *model = new SaleMostSoldItemModel(record.value("data_model").toList(), this);
                m_dataModels.append(model);
            }
        }

        endResetModel();

        emit success();
    } else {
        emit error();
    }
}