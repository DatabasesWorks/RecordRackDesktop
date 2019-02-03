#include "qmltaptarget.h"
#include <QQuickItem>

QMLTapTarget::QMLTapTarget(QObject *parent) :
    QObject(parent),
    m_target(nullptr)
{

}

QString QMLTapTarget::targetName() const
{
    return m_targetName;
}

void QMLTapTarget::setTargetName(const QString &targetName)
{
    if (m_targetName == targetName)
        return;

    m_targetName = targetName;
    emit targetNameChanged();
}

QRectF QMLTapTarget::rect() const
{
    return m_rect;
}

void QMLTapTarget::setRect(const QRectF &rect)
{
    if (m_rect == rect)
        return;

    m_rect = rect;
    emit rectChanged();
}

QQuickItem *QMLTapTarget::target() const
{
    return m_target;
}

void QMLTapTarget::setTarget(QQuickItem *target)
{
    if (m_target == target)
        return;

    m_target = target;
    emit targetChanged();
}
