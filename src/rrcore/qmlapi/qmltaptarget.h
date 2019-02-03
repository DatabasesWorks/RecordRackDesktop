#ifndef QMLTAPTARGET_H
#define QMLTAPTARGET_H

#include <QObject>
#include <QRectF>

class QQuickItem;

class QMLTapTarget : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString targetName READ targetName WRITE setTargetName NOTIFY targetNameChanged)
    Q_PROPERTY(QRectF rect READ rect NOTIFY rectChanged)
    Q_PROPERTY(QQuickItem* target READ target NOTIFY targetChanged)
public:
    explicit QMLTapTarget(QObject *parent = nullptr);

    QString targetName() const;
    void setTargetName(const QString &targetName);

    QRectF rect() const;
    void setRect(const QRectF &rect);

    QQuickItem *target() const;
    void setTarget(QQuickItem *target);
signals:
    void targetNameChanged();
    void rectChanged();
    void targetChanged();
private:
    QString m_targetName;
    QRectF m_rect;
    QQuickItem *m_target;
};

#endif // QMLTAPTARGET_H
