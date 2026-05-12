#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>

struct LyricWord
{
    qint64 timeMs = 0;
    QString text;
};

struct LyricLine
{
    qint64 timeMs = 0;
    QString text;
    QVector<LyricWord> words;
};

class LyricParser
{
public:
    bool loadFromFile(const QString &filePath);
    bool parse(const QString &content);

    const QVector<LyricLine> &lines() const;
    const QMap<QString, QString> &metadata() const;

    int currentLineIndex(qint64 positionMs) const;
    int currentWordIndex(int lineIndex, qint64 positionMs) const;

private:
    qint64 parseTimeMs(const QString &timeText) const;
    QVector<LyricWord> parseEnhancedWords(const QString &text) const;

private:
    QVector<LyricLine> m_lines;
    QMap<QString, QString> m_metadata;
    qint64 m_offsetMs = 0;
};