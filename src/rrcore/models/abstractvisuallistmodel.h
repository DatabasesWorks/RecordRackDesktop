#ifndef ABSTRACTVISUALLISTMODEL_H
#define ABSTRACTVISUALLISTMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <QVariant>
#include "database/queryrequest.h"
#include "database/queryresult.h"

class QueryRequest;
class QueryResult;
class DatabaseThread;

class AbstractVisualListModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(bool autoQuery READ autoQuery WRITE setAutoQuery NOTIFY autoQueryChanged)
    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(int filterColumn READ filterColumn WRITE setFilterColumn NOTIFY filterColumnChanged)
public:
    explicit AbstractVisualListModel(QObject *parent = nullptr);
    explicit AbstractVisualListModel(DatabaseThread &thread);
    virtual ~AbstractVisualListModel() override;

    bool autoQuery() const;
    void setAutoQuery(bool);

    bool isBusy() const;

    QString filterText() const;
    void setFilterText(const QString &filterText);

    int filterColumn() const;
    void setFilterColumn(int filterColumn);

    Q_INVOKABLE QVariant get(int row) const;

    void classBegin() override;
    void componentComplete() override;
public slots:
    virtual void undoLastCommit();
    virtual void refresh();
protected:
    virtual void tryQuery() = 0;
    virtual void processResult(const QueryResult result) = 0;
    virtual void filter();
    void setBusy(bool);

    void setLastRequest(const QueryRequest &lastRequest);
    QueryRequest lastRequest() const;
signals:
    void executeRequest(const QueryRequest request);
    void autoQueryChanged();
    void busyChanged();

    void filterTextChanged();
    void filterColumnChanged();

    void success(int successCode = -1);
    void error(int errorCode = -1);
private:
    bool m_autoQuery;
    bool m_busy;
    QString m_filterText;
    int m_filterColumn;
    QueryRequest m_lastRequest;

    void saveRequest(const QueryResult &result);
};

#endif // ABSTRACTVISUALLISTMODEL_H
