#include "LyricFinder.h"
#include "LyricSelectionDialog.h"
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

void LyricFinder::setSearchDirs(const QStringList &dirs)
{
    m_dirs = dirs;
}

QString LyricFinder::normalize(const QString &s) const
{
    QString r = s.toLower();
    r.remove(QRegularExpression(R"(\(.*?\))"));
    r.remove(QRegularExpression(R"(\[.*?\])"));
    r.replace("_", " ");
    r.replace("-", " ");
    r.replace(".", " ");
    r.remove(QRegularExpression(R"([^\p{L}\p{N}\s])"));
    r.replace(QRegularExpression(R"(\s+)"), " ");
    return r.trimmed();
}

bool LyricFinder::isExactTitleArtistMatch(const QString &fileName,
                                          const QString &title,
                                          const QString &artist) const
{
    QString f = normalize(fileName);
    QString t = normalize(title);
    QString a = normalize(artist);

    if (t.isEmpty() || a.isEmpty())
        return false;

    QString titleArtist = normalize(t + " " + a);
    QString artistTitle = normalize(a + " " + t);

    return f == titleArtist || f == artistTitle;
}

int LyricFinder::scoreFile(const QString &fileName,
                           const QString &title,
                           const QString &artist) const
{
    QString f = normalize(fileName);
    QString t = normalize(title);
    QString a = normalize(artist);

    int score = 0;

    if (t.isEmpty())
        return 0;

    // 最高优先级：title - artist / artist - title
    if (!a.isEmpty()) {
        QString titleArtist = normalize(t + " " + a);
        QString artistTitle = normalize(a + " " + t);

        if (f == titleArtist || f == artistTitle)
            return 10000;
    }

    // title 完全匹配
    if (f == t)
        score += 500;

    // 文件名包含 title
    if (f.contains(t))
        score += 300;

    // 文件名以 title 开头
    if (f.startsWith(t))
        score += 120;

    // artist 次级匹配
    if (!a.isEmpty()) {
        if (f.contains(a))
            score += 120;

        if (f.startsWith(a))
            score += 60;
    }

    // title 单词匹配
    const QStringList titleWords = t.split(' ', Qt::SkipEmptyParts);
    for (const QString &w : titleWords) {
        if (w.length() >= 2 && f.contains(w))
            score += 15;
    }

    // artist 单词匹配，权重低一点
    const QStringList artistWords = a.split(' ', Qt::SkipEmptyParts);
    for (const QString &w : artistWords) {
        if (w.length() >= 2 && f.contains(w))
            score += 8;
    }

    return score;
}

QString LyricFinder::findLyric(const QString &title,
                               const QString &artist,
                               QWidget *parent) const
{
    struct Candidate {
        QString path;
        QString display;
        int score = 0;
    };

    QVector<Candidate> candidates;

    for (const QString &dirPath : m_dirs) {
        QDir dir(dirPath);
        if (!dir.exists())
            continue;

        QFileInfoList files = dir.entryInfoList(
            QStringList() << "*.lrc" << "*.LRC",
            QDir::Files,
            QDir::Name
        );

        for (const QFileInfo &info : files) {
            QString baseName = info.completeBaseName();

            // 1. title - artist / artist - title 完全匹配，直接返回
            if (isExactTitleArtistMatch(baseName, title, artist)) {
                qDebug() << "Exact title-artist lyric matched:"
                         << info.absoluteFilePath();
                return info.absoluteFilePath();
            }

            int score = scoreFile(baseName, title, artist);

            if (score >= 40) {
                Candidate c;
                c.path = info.absoluteFilePath();
                c.score = score;
                c.display = QString("[%1] %2")
                                .arg(score)
                                .arg(info.absoluteFilePath());

                candidates.append(c);
            }
        }
    }

    if (candidates.isEmpty()) {
        if (parent) {
            QMessageBox::information(
                parent,
                "Lyrics Not Found",
                "没有找到匹配的歌词文件。"
            );
        }
        return QString();
    }

    // 2. 其他候选按 score 从高到低排序
    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate &a, const Candidate &b) {
        if (a.score != b.score)
            return a.score > b.score;

        return a.path < b.path;
    });

    if (candidates.size() == 1) {
        qDebug() << "Found lyric:" << candidates.first().path
                 << "score:" << candidates.first().score;
        return candidates.first().path;
    }

    // 3. 多个候选，弹出选择框
    QStringList items;
    QMap<QString, QString> displayToPath;

    for (const Candidate &c : candidates) {
        items << c.display;
        displayToPath[c.display] = c.path;
    }

    bool ok = false;
    QString selected = QInputDialog::getItem(
        parent,
        "Select Lyric File",
        "找到多个匹配歌词文件，请选择：",
        items,
        0,
        false,
        &ok
    );

    if (ok && !selected.isEmpty()) {
        QString path = displayToPath.value(selected);
        qDebug() << "User selected lyric:" << path;
        return path;
    }

    return QString();
}