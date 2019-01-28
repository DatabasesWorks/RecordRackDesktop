#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT
public:
    static Settings &instance();

    Settings(Settings const &) = delete;
    void operator=(Settings const &) = delete;

    bool darkModeActive() const;
    void setDarkModeActive(bool darkModeActive);
signals:
    void darkModeActiveChanged();
private:
    explicit Settings(QObject *parent = nullptr);
    QSettings m_settings;
    bool m_darkModeActive;
};

#endif // SETTINGS_H