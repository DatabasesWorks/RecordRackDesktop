#ifndef QMLTAPTARGETFINDER_H
#define QMLTAPTARGETFINDER_H

#include <QObject>
#include <QQmlListProperty>
#include <QQmlParserStatus>

class QMLTapTarget;

class QMLTapTargetFinder : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QMLTapTarget> tapTargets READ tapTargets)
public:
    explicit QMLTapTargetFinder(QObject *parent = nullptr);

    QQmlListProperty<QMLTapTarget> tapTargets();
    void appendTapTarget(QMLTapTarget *);
    int tapTargetCount() const;
    QMLTapTarget* tapTarget(int index) const;
    void clearTapTargets();

    void classBegin() override;
    void componentComplete() override;
signals:
    void found(QMLTapTarget *tapTarget);
    void cleared();
private:
    QList<QMLTapTarget *> m_tapTargets;

    static void appendTapTarget(QQmlListProperty<QMLTapTarget>*, QMLTapTarget*);
    static int tapTargetCount(QQmlListProperty<QMLTapTarget>*);
    static QMLTapTarget* tapTarget(QQmlListProperty<QMLTapTarget>*, int);
    static void clearTapTargets(QQmlListProperty<QMLTapTarget>*);
};

#endif // QMLTAPTARGETFINDER_H
