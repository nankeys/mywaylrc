#pragma once

#include <QString>
#include <QStringList>
#include <QWidget> // 用于弹框 parent

class LyricFinder
{
public:
    void setSearchDirs(const QStringList &dirs);
    QString findLyric(const QString &title, const QString &artist, QWidget *parent = nullptr) const;

private:
    QString normalize(const QString &s) const;
    int scoreFile(const QString &fileName,
                  const QString &title,
                  const QString &artist) const;
    bool isExactTitleArtistMatch(const QString &fileName,
                                 const QString &title,
                                 const QString &artist) const;

private:
    QStringList m_dirs;
};