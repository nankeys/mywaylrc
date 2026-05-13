#include <QApplication>
#include <QDebug>
#include <QShortcut>
#include <QKeySequence>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QCoreApplication>

#include "MprisPlayer.h"
#include "LyricParser.h"
// #include "LyricWindowController.h"
#include "LyricFinder.h"
#include "DesktopLyricWindow.h"
#include "LyricTray.h"


int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", QByteArray("xcb"));
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("LyricPhase");
    QCoreApplication::setApplicationName("DesktopLyric");

    MprisPlayer player;
    LyricParser parser;

    LyricFinder finder;
    finder.loadSettings();

    DesktopLyricWindow lyricWindow;
    lyricWindow.hide();
    // lyricWindow.setHighlightGradient({
    //     QColor("#000000"),
    //     QColor("#f8320b"),
    //     QColor("#000000")
    // });

    // 当前正在显示的歌曲标识
    QString currentSong;
    QString lrcPath;
    bool startupLyricLoaded = false;
    auto firstDisplayLine = [&parser]() -> QString {
        for (const auto &line : parser.lines()) {
            if (!line.text.trimmed().isEmpty())
                return line.text;
        }
        return QString();
    };

    // 封装：只在切歌时弹出一次
    auto reloadLyricOnce = [&]() {
        QString title = player.title();
        QString artist = player.artist();

        if (title.isEmpty()) {
            qDebug() << "skip lyric reload: empty title";
            return;
        }

        QString songId = title + "|" + artist;

        if (songId == currentSong)
            return; // 同一首歌，不重复弹框

        currentSong = songId; // 更新当前歌曲

        lrcPath = finder.findLyric(title, artist, nullptr); // 弹框只会调用一次

        qDebug() << "title:" << title;
        qDebug() << "artist:" << artist;
        qDebug() << "matched lrc:" << lrcPath;

        if (!lrcPath.isEmpty() && lrcPath != QString()) {
            if (parser.loadFromFile(lrcPath)) {
                qDebug() << "load lyric ok:" << parser.lines().size();
                if (!parser.lines().isEmpty()) {
                    lyricWindow.setLyric(firstDisplayLine());
                    lyricWindow.showForLyric();
                    startupLyricLoaded = true;
                } else {
                    lyricWindow.setLyric("");
                    lyricWindow.hideForNoLyric();
                }
            } else {
                qDebug() << "load lyric failed";
                lyricWindow.setLyric("");
                lyricWindow.hideForNoLyric();
            }
        } else {
            parser.loadFromFile(""); // 清空歌词
            lyricWindow.setLyric("");
            lyricWindow.setProgress(0);
            lyricWindow.hideForNoLyric();
        }
    };

    // 启动时主动重试几次，覆盖“程序在歌曲播放中途启动”的场景。
    for (int i = 0; i < 8; ++i) {
        QTimer::singleShot(400 + i * 500, [&]() {
            if (startupLyricLoaded)
                return;

            player.refreshPlayers();
            reloadLyricOnce();
        });
    }

    // 连接 MPRIS metadataChanged 信号
    QObject::connect(&player, &MprisPlayer::metadataChanged, reloadLyricOnce);
    QObject::connect(&player, &MprisPlayer::playerChanged, [&]() {
        startupLyricLoaded = false;
        reloadLyricOnce();
    });
    QObject::connect(&player, &MprisPlayer::playbackStatusChanged, [&](const QString &status) {
        if (status == "Playing")
            reloadLyricOnce();
    });

    // 系统托盘
    LyricTray tray(&lyricWindow, &finder, &parser, &player);
    tray.show();

    // 定时更新歌词高亮
    QTimer timer;

    QObject::connect(&timer, &QTimer::timeout, [&]() {
        qint64 posMs = player.smoothPositionUs() / 1000;
        int lineIndex = parser.currentLineIndex(posMs);

        if (parser.lines().isEmpty()) {
            lyricWindow.hideForNoLyric();
            lyricWindow.setLyric("");
            return;
        }

        if (!lyricWindow.isVisible())
            lyricWindow.showForLyric();

        if (lineIndex < 0)
            return;

        const auto &line = parser.lines()[lineIndex];
        lyricWindow.setLyric(line.text);

        double progress = 0.08;
        if (lineIndex + 1 < parser.lines().size()) {
            qint64 start = line.timeMs;
            qint64 end = parser.lines()[lineIndex + 1].timeMs;
            if (end > start)
                progress = double(posMs - start) / double(end - start);
        }

        lyricWindow.setProgress(progress);
    });

    timer.start(30);

    return app.exec();
}
