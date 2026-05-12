#include "MprisPlayer.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDebug>

static constexpr const char *MPRIS_PATH =
    "/org/mpris/MediaPlayer2";

static constexpr const char *PLAYER_IFACE =
    "org.mpris.MediaPlayer2.Player";

static constexpr const char *PROPERTIES_IFACE =
    "org.freedesktop.DBus.Properties";

MprisPlayer::MprisPlayer(QObject *parent)
    : QObject(parent)
{
    connect(&m_positionTimer, &QTimer::timeout,
            this, &MprisPlayer::pollPosition);

    m_positionTimer.start(500);

    refreshPlayers();
}

QString MprisPlayer::serviceName() const
{
    return m_serviceName;
}

QString MprisPlayer::title() const
{
    return m_title;
}

QString MprisPlayer::artist() const
{
    return m_artist;
}

QString MprisPlayer::album() const
{
    return m_album;
}

QString MprisPlayer::playbackStatus() const
{
    return m_playbackStatus;
}

qint64 MprisPlayer::positionUs() const
{
    return m_positionUs;
}

qint64 MprisPlayer::lengthUs() const
{
    return m_lengthUs;
}

qint64 MprisPlayer::smoothPositionUs() const
{
    if (m_playbackStatus == "Playing" && m_elapsedTimer.isValid()) {
        return m_basePositionUs + m_elapsedTimer.elapsed() * 1000;
    }

    return m_positionUs;
}

void MprisPlayer::refreshPlayers()
{
    auto iface = QDBusConnection::sessionBus().interface();

    if (!iface)
        return;

    QDBusReply<QStringList> reply =
        iface->registeredServiceNames();

    if (!reply.isValid())
        return;

    QString fallback;

    for (const QString &name : reply.value()) {

        if (!name.startsWith("org.mpris.MediaPlayer2."))
            continue;

        if (fallback.isEmpty())
            fallback = name;

        QDBusInterface player(
            name,
            MPRIS_PATH,
            PROPERTIES_IFACE,
            QDBusConnection::sessionBus()
        );

        auto statusReply =
            player.call(
                "Get",
                PLAYER_IFACE,
                "PlaybackStatus"
            );

        QString status =
            statusReply.arguments().first().value<QDBusVariant>().variant().toString();

        if (status == "Playing") {
            setService(name);
            return;
        }
    }

    if (!fallback.isEmpty())
        setService(fallback);
}

void MprisPlayer::setService(const QString &service)
{
    if (m_serviceName == service)
        return;

    if (!m_serviceName.isEmpty()) {
        QDBusConnection::sessionBus().disconnect(
            m_serviceName,
            MPRIS_PATH,
            PROPERTIES_IFACE,
            "PropertiesChanged",
            this,
            SLOT(onPropertiesChanged(QString,QVariantMap,QStringList))
        );
    }

    m_serviceName = service;

    if (!m_serviceName.isEmpty()) {
        QDBusConnection::sessionBus().connect(
            m_serviceName,
            MPRIS_PATH,
            PROPERTIES_IFACE,
            "PropertiesChanged",
            this,
            SLOT(onPropertiesChanged(QString,QVariantMap,QStringList))
        );

        readAllProperties();
    }

    emit playerChanged(m_serviceName);
}

void MprisPlayer::readAllProperties()
{
    readMetadata();
    readPlaybackStatus();
    readPosition();
}

void MprisPlayer::readMetadata()
{
    if (m_serviceName.isEmpty())
        return;

    QDBusInterface iface(
        m_serviceName,
        MPRIS_PATH,
        PROPERTIES_IFACE,
        QDBusConnection::sessionBus()
    );

    QDBusReply<QVariant> reply =
        iface.call("Get", PLAYER_IFACE, "Metadata");

    if (!reply.isValid()) {
        qWarning() << "Failed to get metadata:" << reply.error();
        return;
    }

    QVariantMap metadata =
        qdbus_cast<QVariantMap>(reply.value().value<QDBusArgument>());

    m_title = metadata.value("xesam:title").toString();
    m_album = metadata.value("xesam:album").toString();
    m_lengthUs = metadata.value("mpris:length").toLongLong();

    QStringList artists = metadata.value("xesam:artist").toStringList();
    m_artist = artists.join(", ");

    emit metadataChanged();
}

void MprisPlayer::readPlaybackStatus()
{
    if (m_serviceName.isEmpty())
        return;

    QDBusInterface iface(
        m_serviceName,
        MPRIS_PATH,
        PROPERTIES_IFACE,
        QDBusConnection::sessionBus()
    );

    QDBusReply<QVariant> reply =
        iface.call("Get", PLAYER_IFACE, "PlaybackStatus");

    if (!reply.isValid()) {
        qWarning() << "Failed to get playback status:" << reply.error();
        return;
    }

    QString status = reply.value().toString();

    if (m_playbackStatus != status) {
        m_playbackStatus = status;
        emit playbackStatusChanged(status);
    }
}

void MprisPlayer::readPosition()
{
    if (m_serviceName.isEmpty())
        return;

    QDBusInterface iface(
        m_serviceName,
        MPRIS_PATH,
        PROPERTIES_IFACE,
        QDBusConnection::sessionBus()
    );

    QDBusReply<QVariant> reply =
        iface.call("Get", PLAYER_IFACE, "Position");

    if (!reply.isValid())
        return;

    qint64 pos = reply.value().toLongLong();

    m_positionUs = pos;
    m_basePositionUs = pos;
    m_elapsedTimer.restart();

    emit positionChanged(pos);
}

void MprisPlayer::pollPosition()
{
    if (m_playbackStatus == "Playing")
        readPosition();
}

void MprisPlayer::onPropertiesChanged(
    const QString &interfaceName,
    const QVariantMap &changedProperties,
    const QStringList &invalidatedProperties
)
{
    Q_UNUSED(invalidatedProperties)

    if (interfaceName != PLAYER_IFACE)
        return;

    if (changedProperties.contains("Metadata")) {
        QVariantMap metadata =
            qdbus_cast<QVariantMap>(
                changedProperties.value("Metadata").value<QDBusArgument>()
            );

        m_title = metadata.value("xesam:title").toString();
        m_album = metadata.value("xesam:album").toString();
        m_lengthUs = metadata.value("mpris:length").toLongLong();

        QStringList artists = metadata.value("xesam:artist").toStringList();
        m_artist = artists.join(", ");

        emit metadataChanged();
    }

    if (changedProperties.contains("PlaybackStatus")) {
        QString status = changedProperties.value("PlaybackStatus").toString();

        if (m_playbackStatus != status) {
            m_playbackStatus = status;
            emit playbackStatusChanged(status);
        }
    }

    if (changedProperties.contains("Position")) {
        qint64 pos = changedProperties.value("Position").toLongLong();

        if (m_positionUs != pos) {
            m_positionUs = pos;
            emit positionChanged(pos);
        }
    }
}

void MprisPlayer::playPause()
{
    if (m_serviceName.isEmpty())
        return;

    QDBusInterface iface(
        m_serviceName,
        MPRIS_PATH,
        PLAYER_IFACE,
        QDBusConnection::sessionBus()
    );

    iface.call("PlayPause");
}

void MprisPlayer::play()
{
    if (m_serviceName.isEmpty())
        return;

    QDBusInterface iface(
        m_serviceName,
        MPRIS_PATH,
        PLAYER_IFACE,
        QDBusConnection::sessionBus()
    );

    iface.call("Play");
}

void MprisPlayer::pause()
{
    if (m_serviceName.isEmpty())
        return;

    QDBusInterface iface(
        m_serviceName,
        MPRIS_PATH,
        PLAYER_IFACE,
        QDBusConnection::sessionBus()
    );

    iface.call("Pause");
}

void MprisPlayer::next()
{
    if (m_serviceName.isEmpty())
        return;

    QDBusInterface iface(
        m_serviceName,
        MPRIS_PATH,
        PLAYER_IFACE,
        QDBusConnection::sessionBus()
    );

    iface.call("Next");
}

void MprisPlayer::previous()
{
    if (m_serviceName.isEmpty())
        return;

    QDBusInterface iface(
        m_serviceName,
        MPRIS_PATH,
        PLAYER_IFACE,
        QDBusConnection::sessionBus()
    );

    iface.call("Previous");
}

void MprisPlayer::seek(qint64 offsetUs)
{
    if (m_serviceName.isEmpty())
        return;

    QDBusInterface iface(
        m_serviceName,
        MPRIS_PATH,
        PLAYER_IFACE,
        QDBusConnection::sessionBus()
    );

    iface.call("Seek", offsetUs);
}