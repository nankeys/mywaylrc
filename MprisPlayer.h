#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QTimer>
#include <QElapsedTimer>

class MprisPlayer : public QObject
{
    Q_OBJECT

public:
    explicit MprisPlayer(QObject *parent = nullptr);

    QString serviceName() const;
    QString title() const;
    QString artist() const;
    QString album() const;
    QString playbackStatus() const;

    qint64 positionUs() const;
    qint64 lengthUs() const;
    qint64 smoothPositionUs() const;

public slots:
    void refreshPlayers();
    void playPause();
    void play();
    void pause();
    void next();
    void previous();
    void seek(qint64 offsetUs);

signals:
    void playerChanged(const QString &serviceName);
    void metadataChanged();
    void playbackStatusChanged(const QString &status);
    void positionChanged(qint64 positionUs);

private slots:
    void pollPosition();
    void onPropertiesChanged(
        const QString &interfaceName,
        const QVariantMap &changedProperties,
        const QStringList &invalidatedProperties
    );

private:
    void setService(const QString &service);
    void readAllProperties();
    void readMetadata();
    void readPlaybackStatus();
    void readPosition();

private:
    QString m_serviceName;

    QString m_title;
    QString m_artist;
    QString m_album;
    QString m_playbackStatus;

    qint64 m_positionUs = 0;
    qint64 m_lengthUs = 0;

    QTimer m_positionTimer;

    QElapsedTimer m_elapsedTimer;
    qint64 m_basePositionUs = 0;
};