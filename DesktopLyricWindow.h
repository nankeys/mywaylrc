#pragma once

#include <QWidget>
#include <QString>
#include <QGradient>
#include <QVector>
#include <QColor>
#include <QPoint>
#include <QTimer>
#include <QSettings>
#include <QFont>
#include <QVector>

class DesktopLyricWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DesktopLyricWindow(QWidget *parent = nullptr);

    void setLyric(const QString &text);
    void showForLyric();
    void hideForNoLyric();
    void setProgress(double progress); // 0.0 ~ 1.0
    void setLocked(bool locked);
    bool isLocked() const;
    // 高亮渐变颜色（已存在）
    void setHighlightGradient(const QVector<QColor> &colors);
    QVector<QColor> highlightGradient() const;
    // 普通文字颜色
    // void setNormalColor(const QColor &color);
    // QColor normalColor() const;
    void setNormalGradient(const QVector<QColor> &colors);
    QVector<QColor> normalGradient() const;
    // 描边颜色和宽度
    void setOutlineColor(const QColor &color, int width);
    QColor outlineColor() const;
    int outlineWidth() const;
    // 字体设置
    void setLyricFont(const QFont &font);
    QFont lyricFont() const;
    // 配置文件读写
    void loadSettings();
    void saveSettings() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    
signals:
    void positionChanged(const QPoint &pos);
    
private slots:
    void onScrollTimeout(); // 滚动定时器槽

private:
    void ensureVisibleOnScreen();
    QString m_text;
    int m_scrollOffset = 0;
    QTimer m_scrollTimer;

    QPoint m_savedPos;
    double m_progress = 0.08;
    bool m_locked = false;
    bool m_dragging = false;
    QPoint m_dragOffset;
    QColor m_outlineColor = QColor(0, 0, 0, 220);
    int m_outlineWidth = 6;
    QVector<QColor> m_highlightGradientColors = {
        QColor("#000000"),
        QColor("#f8320b"),
        QColor("#000000")
    };
    QVector<QColor> m_normalGradientColors = {
        QColor("#cfcfcf"),
        QColor("#ffffff"),
        QColor("#cfcfcf")
    };
    // QColor m_normalColor = Qt::white; // 默认白色
    QFont m_lyricFont = QFont("Kaiti", 32, QFont::Bold);
};
