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

    // 封装：只在切歌时弹出一次
    auto reloadLyricOnce = [&]() {
        QString title = player.title();
        QString artist = player.artist();

        if (title.isEmpty())
            return;

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
                if (!parser.lines().isEmpty())
                    lyricWindow.setLyric(parser.lines().first().text);
                else
                    lyricWindow.setLyric("");
            } else {
                qDebug() << "load lyric failed";
                lyricWindow.setLyric("");
            }
        } else {
            parser.loadFromFile(""); // 清空歌词
            lyricWindow.setLyric("");
            lyricWindow.setProgress(0);
        }
    };

    // 启动时检查播放器状态（防止程序启动时歌曲已经在播放）
    qint64 posMs = player.smoothPositionUs() / 1000;
    if (posMs > 0.08) {
        QTimer::singleShot(500, reloadLyricOnce);
    }

    // 连接 MPRIS metadataChanged 信号
    QObject::connect(&player, &MprisPlayer::metadataChanged, reloadLyricOnce);

    // 系统托盘
    LyricTray tray(&lyricWindow, &finder, &parser, &player);
    tray.show();

    // 定时更新歌词高亮
    QTimer timer;

    QObject::connect(&timer, &QTimer::timeout, [&]() {
        qint64 posMs = player.smoothPositionUs() / 1000;
        int lineIndex = parser.currentLineIndex(posMs);

        if (lineIndex < 0) {
            if (parser.lines().isEmpty())
                lyricWindow.setLyric("");
            return;
        }

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
