#include "LyricParser.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <algorithm>

bool LyricParser::loadFromFile(const QString &filePath)
{
    if(filePath.isEmpty()) {
        m_lines.clear();
        m_metadata.clear();
        m_offsetMs = 0;
        return false;
    }

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    return parse(in.readAll());
}

bool LyricParser::parse(const QString &content)
{
    m_lines.clear();
    m_metadata.clear();
    m_offsetMs = 0;

    const QStringList rawLines = content.split('\n');

    QRegularExpression metaRe(
        R"(\[([a-zA-Z]+):(.*)\])"
    );

    QRegularExpression timeRe(
        R"(\[(\d{1,2}:\d{2}(?:\.\d{1,3})?)\])"
    );

    for (QString raw : rawLines) {
        raw = raw.trimmed();

        if (raw.isEmpty())
            continue;

        auto metaMatch = metaRe.match(raw);
        if (metaMatch.hasMatch() && !raw.contains(QRegularExpression(R"(\[\d{1,2}:\d{2})"))) {
            QString key = metaMatch.captured(1);
            QString value = metaMatch.captured(2).trimmed();

            m_metadata[key] = value;

            if (key == "offset")
                m_offsetMs = value.toLongLong();

            continue;
        }

        QVector<qint64> times;

        auto it = timeRe.globalMatch(raw);
        while (it.hasNext()) {
            auto match = it.next();
            times.append(parseTimeMs(match.captured(1)));
        }

        if (times.isEmpty())
            continue;

        QString lyricText = raw;
        lyricText.remove(timeRe);

        QVector<LyricWord> words = parseEnhancedWords(lyricText);

        QString plainText = lyricText;
        plainText.remove(QRegularExpression(R"(<\d{1,2}:\d{2}(?:\.\d{1,3})?>)"));

        for (qint64 t : times) {
            LyricLine line;
            line.timeMs = t + m_offsetMs;
            line.text = plainText;
            line.words = words;

            m_lines.append(line);
        }
    }

    std::sort(m_lines.begin(), m_lines.end(),
              [](const LyricLine &a, const LyricLine &b) {
        return a.timeMs < b.timeMs;
    });

    return !m_lines.isEmpty();
}

qint64 LyricParser::parseTimeMs(const QString &timeText) const
{
    QStringList parts = timeText.split(':');

    if (parts.size() != 2)
        return 0;

    qint64 minute = parts[0].toLongLong();
    double second = parts[1].toDouble();

    return minute * 60 * 1000 + static_cast<qint64>(second * 1000.0);
}

QVector<LyricWord> LyricParser::parseEnhancedWords(const QString &text) const
{
    QVector<LyricWord> words;

    QRegularExpression wordRe(
        R"(<(\d{1,2}:\d{2}(?:\.\d{1,3})?)>([^<]*))"
    );

    auto it = wordRe.globalMatch(text);

    while (it.hasNext()) {
        auto match = it.next();

        LyricWord word;
        word.timeMs = parseTimeMs(match.captured(1));
        word.text = match.captured(2);

        if (!word.text.isEmpty())
            words.append(word);
    }

    return words;
}

const QVector<LyricLine> &LyricParser::lines() const
{
    return m_lines;
}

const QMap<QString, QString> &LyricParser::metadata() const
{
    return m_metadata;
}

int LyricParser::currentLineIndex(qint64 positionMs) const
{
    if (m_lines.isEmpty())
        return -1;

    int left = 0;
    int right = m_lines.size() - 1;
    int result = -1;

    while (left <= right) {
        int mid = (left + right) / 2;

        if (m_lines[mid].timeMs <= positionMs) {
            result = mid;
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return result;
}

int LyricParser::currentWordIndex(int lineIndex, qint64 positionMs) const
{
    if (lineIndex < 0 || lineIndex >= m_lines.size())
        return -1;

    const auto &words = m_lines[lineIndex].words;

    if (words.isEmpty())
        return -1;

    int result = -1;

    for (int i = 0; i < words.size(); ++i) {
        if (words[i].timeMs <= positionMs)
            result = i;
        else
            break;
    }

    return result;
}