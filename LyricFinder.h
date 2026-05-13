#pragma once

#include <QString>
#include <QStringList>
#include <QWidget> // 用于弹框 parent

class LyricFinder
{
public:
    void setSearchDirs(const QStringList &dirs);
    QStringList searchDirs() const;
    void loadSettings();
    void saveSettings() const;
    QString findLyric(const QString &title, const QString &artist, QWidget *parent = nullptr) const;

private:
    QStringList defaultSearchDirs() const;
    QString normalize(const QString &s) const;
    QStringList splitWords(const QString &s) const;
    bool containsAllWords(const QString &text, const QStringList &words) const;
    int countMatchedWords(const QString &text, const QStringList &words) const;
    int scoreFile(const QString &fileName,
                  const QString &title,
                  const QString &artist) const;
    bool isExactTitleArtistMatch(const QString &fileName,
                                 const QString &title,
                                 const QString &artist) const;

private:
    QStringList m_dirs;
};
