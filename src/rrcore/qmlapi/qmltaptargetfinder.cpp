#include "qmltaptargetfinder.h"
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include "qmltaptarget.h"

QMLTapTargetFinder::QMLTapTargetFinder(QObject *parent):
    QObject(parent)
{

}

QQmlListProperty<QMLTapTarget> QMLTapTargetFinder::tapTargets()
{
    return QQmlListProperty<QMLTapTarget>(this, this,
                                          &QMLTapTargetFinder::appendTapTarget,
                                          &QMLTapTargetFinder::tapTargetCount,
                                          &QMLTapTargetFinder::tapTarget,
                                          &QMLTapTargetFinder::clearTapTargets);
}

void QMLTapTargetFinder::appendTapTarget(QMLTapTarget* tapTarget)
{
    QQmlApplicationEngine *engine = static_cast<QQmlApplicationEngine *>(qmlEngine(this));
    if (!engine || engine->rootObjects().isEmpty())
        return;

    QObject *rootObject = engine->rootObjects().first();
    QQuickItem *parent = static_cast<QQuickItem *>(this->parent()->parent());
    QQuickItem *child = rootObject->findChild<QQuickItem *>(tapTarget->targetName());
    if (!parent || !child) {
        qDebug() << "Unable to find child \"" << tapTarget->targetName() << "\"";
    } else {
        const QRectF &mappedRect = child->mapRectToItem(parent, QRectF(QPointF(child->position().x() - 818.0, child->position().y() - 48.0),
                                                                       child->size()));
        qDebug() << "Root? " << parent;
        qDebug() << "Child? " << child << tapTarget->targetName();
        qDebug() << "Child's position? " << child->position() << child->size();
        qDebug() << "Mapping? " << mappedRect;
        qDebug() << "Try? " << child->mapRectFromItem(parent, QRectF(child->position(), child->size()));
        qDebug() << "Parent size? " << parent->position() << parent->size();
        tapTarget->setRect(mappedRect);
        tapTarget->setTarget(child);
        m_tapTargets.append(tapTarget);
        emit found(tapTarget);
    }
}

int QMLTapTargetFinder::tapTargetCount() const
{
    return m_tapTargets.count();
}

QMLTapTarget *QMLTapTargetFinder::tapTarget(int index) const
{
    return m_tapTargets.at(index);
}

void QMLTapTargetFinder::clearTapTargets()
{
    m_tapTargets.clear();
    emit cleared();
}

void QMLTapTargetFinder::classBegin()
{

}

void QMLTapTargetFinder::componentComplete()
{
}

void QMLTapTargetFinder::appendTapTarget(QQmlListProperty<QMLTapTarget>* list, QMLTapTarget* tapTarget)
{
    reinterpret_cast< QMLTapTargetFinder* >(list->data)->appendTapTarget(tapTarget);
}

void QMLTapTargetFinder::clearTapTargets(QQmlListProperty<QMLTapTarget>* list)
{
    reinterpret_cast< QMLTapTargetFinder* >(list->data)->clearTapTargets();
}

QMLTapTarget* QMLTapTargetFinder::tapTarget(QQmlListProperty<QMLTapTarget>* list, int i)
{
    return reinterpret_cast< QMLTapTargetFinder* >(list->data)->tapTarget(i);
}

int QMLTapTargetFinder::tapTargetCount(QQmlListProperty<QMLTapTarget>* list)
{
    return reinterpret_cast< QMLTapTargetFinder* >(list->data)->tapTargetCount();
}
