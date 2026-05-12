#pragma once
#include <QObject>
#include <QStringList>

class QSystemTrayIcon;
class QMenu;
class QAction;
class DesktopLyricWindow;
class LyricFinder;
class LyricParser;
class MprisPlayer;

class LyricTray : public QObject
{
    Q_OBJECT
public:
    LyricTray(DesktopLyricWindow *lyricWindow,
              LyricFinder *finder,
              LyricParser *parser,
              MprisPlayer *player,   // 新增
              QObject *parent = nullptr);

    void show();

private:
    QSystemTrayIcon *m_tray;
    QMenu *m_menu;
    QAction *m_lockAction;

    DesktopLyricWindow *m_lyricWindow;
    LyricFinder *m_finder;
    LyricParser *m_parser;
    MprisPlayer *m_player;

    QString m_currentSong; // 保存当前歌曲，避免重复弹框

    void createMenu();
};