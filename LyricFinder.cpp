#include "LyricFinder.h"
#include "LyricSelectionDialog.h"
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include <QRegularExpression>

void LyricFinder::setSearchDirs(const QStringList &dirs)
{
    m_dirs = dirs;
}

QStringList LyricFinder::searchDirs() const
{
    return m_dirs;
}

void LyricFinder::loadSettings()
{
    QSettings settings("LyricPhase", "DesktopLyric");
    const QStringList dirs = settings.value("lyrics/searchDirs").toStringList();

    if (dirs.isEmpty()) {
        m_dirs = defaultSearchDirs();
        saveSettings();
        return;
    }

    m_dirs = dirs;
}

void LyricFinder::saveSettings() const
{
    QSettings settings("LyricPhase", "DesktopLyric");
    settings.setValue("lyrics/searchDirs", m_dirs);
    settings.sync();
}

QStringList LyricFinder::defaultSearchDirs() const
{
    return {
        "/mnt/zspace/myLyrics",
        QDir::home().filePath(".lyrics"),
        QDir::home().filePath(".local/share/lyrics")
    };
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

QStringList LyricFinder::splitWords(const QString &s) const
{
    return normalize(s).split(' ', Qt::SkipEmptyParts);
}

bool LyricFinder::containsAllWords(const QString &text,
                                   const QStringList &words) const
{
    if (words.isEmpty())
        return false;

    for (const QString &word : words) {
        if (word.length() >= 2 && !text.contains(word))
            return false;
    }

    return true;
}

int LyricFinder::countMatchedWords(const QString &text,
                                   const QStringList &words) const
{
    int count = 0;

    for (const QString &word : words) {
        if (word.length() >= 2 && text.contains(word))
            ++count;
    }

    return count;
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
    const QStringList titleWords = splitWords(title);
    const QStringList artistWords = splitWords(artist);

    int score = 0;

    if (t.isEmpty())
        return 0;

    const bool hasArtist = !a.isEmpty();
    const QString titleArtist = hasArtist ? normalize(t + " " + a) : QString();
    const QString artistTitle = hasArtist ? normalize(a + " " + t) : QString();
    const bool titleExact = (f == t);
    const bool titleContains = f.contains(t);
    const bool titleStarts = f.startsWith(t);
    const bool artistContains = hasArtist && f.contains(a);
    const bool artistStarts = hasArtist && f.startsWith(a);
    const bool allTitleWordsMatched = containsAllWords(f, titleWords);
    const bool allArtistWordsMatched = hasArtist && containsAllWords(f, artistWords);
    const int matchedTitleWords = countMatchedWords(f, titleWords);
    const int matchedArtistWords = hasArtist ? countMatchedWords(f, artistWords) : 0;

    // 最高优先级：title - artist / artist - title
    if (hasArtist) {
        if (f == titleArtist || f == artistTitle)
            return 20000;

        if (f.contains(titleArtist) || f.contains(artistTitle))
            score += 10000;

        if (allTitleWordsMatched && allArtistWordsMatched)
            score += 4500;

        if (titleContains && artistContains)
            score += 2200;
    }

    // title 完全匹配
    if (titleExact)
        score += 900;

    // 文件名包含 title
    if (titleContains)
        score += 420;

    // 文件名以 title 开头
    if (titleStarts)
        score += 180;

    // artist 次级匹配，但和 title 组合时权重更高
    if (hasArtist) {
        if (artistContains)
            score += 280;

        if (artistStarts)
            score += 100;
    }

    // title 单词匹配
    score += matchedTitleWords * 25;

    // artist 单词匹配，权重低一点
    score += matchedArtistWords * 16;

    if (allTitleWordsMatched)
        score += 140;

    if (hasArtist && allArtistWordsMatched)
        score += 120;

    // 有 artist 信息时，优先 title + artist 的组合；
    // 仅命中 title 的文件保留为兜底，但明显降权。
    if (hasArtist && !artistContains && matchedArtistWords == 0) {
        if (!titleExact)
            score -= 320;
        else
            score -= 120;
    }

    if (hasArtist && matchedArtistWords > 0 && matchedTitleWords == 0)
        score -= 200;

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
