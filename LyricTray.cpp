#include "LyricTray.h"
#include "DesktopLyricWindow.h"
#include "LyricFinder.h"
#include "LyricParser.h"
#include "MprisPlayer.h"
#include "LyricsDirectoryDialog.h"
#include "LyricsStyleDialog.h"

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QDebug>
#include <QApplication>

LyricTray::LyricTray(DesktopLyricWindow *lyricWindow,
                     LyricFinder *finder,
                     LyricParser *parser,
                     MprisPlayer *player,   // 新增
                     QObject *parent)
    : QObject(parent)
    , m_lyricWindow(lyricWindow)
    , m_finder(finder)
    , m_parser(parser)
    , m_player(player)       // 保存
{
    m_tray = new QSystemTrayIcon(this);
    m_tray->setIcon(QIcon::fromTheme("media-playback-start"));

    m_menu = new QMenu;
    createMenu();

    m_tray->setContextMenu(m_menu);
}

void LyricTray::createMenu()
{
    // 锁定/解锁
    m_lockAction = m_menu->addAction("锁定歌词");
    m_lockAction->setCheckable(true);
    m_lockAction->setChecked(false);

    connect(m_lockAction, &QAction::toggled, [this](bool checked){
        m_lyricWindow->setLocked(checked);
        m_lockAction->setText(checked ? "解锁歌词" : "锁定歌词");
    });

    m_menu->addAction("歌词目录设置...", [this]() {
        LyricsDirectoryDialog dlg(m_finder->searchDirs());

        if (dlg.exec() != QDialog::Accepted)
            return;

        m_finder->setSearchDirs(dlg.directories());
        m_finder->saveSettings();
    });

    // 浏览歌词
    m_menu->addAction("浏览歌词", [this](){
        QString filePath = QFileDialog::getOpenFileName(
            nullptr,
            "Select Lyric File",
            QDir::homePath(),
            "Lyric Files (*.lrc);;All Files (*)"
        );

        if (!filePath.isEmpty()) {
            qDebug() << "User selected lyric file:" << filePath;
            if (m_parser->loadFromFile(filePath)) {
                m_lyricWindow->setLyric(
                    m_parser->lines().isEmpty() ? "" : m_parser->lines().first().text
                );
                m_currentSong = "MANUAL|" + filePath;
            } else {
                m_lyricWindow->setLyric("");
            }
        }
    });

    // 重载歌词
    m_menu->addAction("重新载入歌词", [this]() {
        if (!m_player) return;

        QString title = m_player->title();
        QString artist = m_player->artist();

        if (title.isEmpty())
            return;

        QString songId = title + "|" + artist;
        if (songId != m_currentSong) {
            m_currentSong = songId; // 更新当前歌曲标识
        }

        QString lrcPath = m_finder->findLyric(title, artist, nullptr);
        if (!lrcPath.isEmpty()) {
            if (m_parser->loadFromFile(lrcPath)) {
                QString firstLine = m_parser->lines().isEmpty() ? "" : m_parser->lines().first().text;
                m_lyricWindow->setLyric(firstLine);
                qDebug() << "Reload lyric from:" << lrcPath;
            } else {
                m_lyricWindow->setLyric("");
            }
        } else {
            m_parser->loadFromFile("");
            m_lyricWindow->setLyric("");
        }
    });

    m_menu->addAction("歌词样式设置...", [this]() {
        LyricsStyleDialog dlg(m_lyricWindow);
        dlg.exec();
    });

    m_menu->addAction("退出", qApp, &QApplication::quit);
}

void LyricTray::show()
{
    m_tray->show();
}
