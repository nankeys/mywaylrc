#pragma once

#include <QObject>
#include <QPointer>
#include <QPoint>
#include <QString>

#include "DesktopLyricWindow.h"

class LyricWindowController : public QObject
{
    Q_OBJECT

public:
    explicit LyricWindowController(QObject *parent = nullptr);

    DesktopLyricWindow *window() const;

    void setLocked(bool locked);
    bool isLocked() const;

    void setLyric(const QString &text);
    void setProgress(double progress);

private:
    void recreateWindow();

private:
    QPointer<DesktopLyricWindow> m_window;

    bool m_locked = false;
    QPoint m_pos = QPoint(500, 800);

    QString m_text;
    double m_progress = 0.0;
};