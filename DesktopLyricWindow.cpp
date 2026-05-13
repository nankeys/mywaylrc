#include "DesktopLyricWindow.h"

#include <QPainter>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QPainterPath>
#include <QScreen>
#include <QMouseEvent>
#include <QWindow>
#include <QMargins>
#include <QWaylandPointer>
// #include <QNativeInterface>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <LayerShellQt/Window>

#include <algorithm>

DesktopLyricWindow::DesktopLyricWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
    );

    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);

    setMouseTracking(true);
    setAutoFillBackground(false);   
    resize(900, 100);
    show();

        // X11 强制置顶 + 隐藏任务栏
#ifdef Q_OS_UNIX
    if (auto *x11App = qGuiApp->nativeInterface<QNativeInterface::QX11Application>()) {
        Display *dpy = x11App->display();
        Window win = this->winId();

        Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
        Atom skip_taskbar = XInternAtom(dpy, "_NET_WM_STATE_SKIP_TASKBAR", False);
        Atom above = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);

        Atom atoms[2] = {skip_taskbar, above};
        XChangeProperty(dpy, win, wm_state, XA_ATOM, 32, PropModeReplace,
                        reinterpret_cast<unsigned char*>(atoms), 2);
        XFlush(dpy);
    }
#endif

    
    connect(&m_scrollTimer, &QTimer::timeout, this, [this]() {
        QFontMetrics fm(QFont("Kaiti", 32, QFont::Bold));
        int textWidth = fm.horizontalAdvance(m_text);
        int maxScroll = qMax(0, textWidth - width());

        // qDebug() << "Scroll timer tick, textWidth:" << textWidth << "scrollOffset:" << m_scrollOffset;

        if (maxScroll == 0) return; // 文本短，不滚动
        m_scrollOffset += 2; // 每次偏移 2 px
        if (m_scrollOffset > maxScroll)
            m_scrollOffset = maxScroll + 10; // 循环滚动，或设置 = maxScroll 停止
        update();
        });

    m_scrollTimer.start(30); // 每 30ms 更新一次

    loadSettings();
}

void DesktopLyricWindow::setLyric(const QString &text)
{
    if (m_text == text)
        return;

    m_text = text;
    m_scrollOffset = 0;

    if (m_text.trimmed().isEmpty()) {
        hide();
        update();
        return;
    }

    if (!isVisible())
        show();

    // m_scrollOffset = 0;

    // // 计算是否需要滚动
    // QFontMetrics fm(font());
    // int textWidth = fm.horizontalAdvance(m_text);

    // if (textWidth > width()) {
    //     m_scrollTimer.start(30); // 开始滚动
    // } else {
    //     m_scrollTimer.stop(); // 不需要滚动
    // }

    update();
}

void DesktopLyricWindow::setProgress(double progress)
{
    progress = std::clamp(progress, 0.0, 1.0);

    if (qAbs(m_progress - progress) < 0.002)
        return;

    m_progress = progress;

    if (m_progress < 0.08)
        m_progress = 0.08;

    update();
}

void DesktopLyricWindow::setLocked(bool locked)
{
    m_locked = locked;
    if (m_locked) {
        setAttribute(Qt::WA_TransparentForMouseEvents, true); // 锁定时无法拖动
    } else {
        setAttribute(Qt::WA_TransparentForMouseEvents, false);
    }
}

bool DesktopLyricWindow::isLocked() const
{
    return m_locked;
}

// void DesktopLyricWindow::setNormalColor(const QColor &color)
// {
//     if (!color.isValid())
//         return;

//     m_normalColor = color;
//     update();
//     saveSettings();
// }

// QColor DesktopLyricWindow::normalColor() const
// {
//     return m_normalColor;
// }

void DesktopLyricWindow::setNormalGradient(const QVector<QColor> &colors)
{
    if (colors.isEmpty())
        return;

    m_normalGradientColors = colors;
    update();
    saveSettings();
}

QVector<QColor> DesktopLyricWindow::normalGradient() const
{
    return m_normalGradientColors;
}

void DesktopLyricWindow::setHighlightGradient(const QVector<QColor> &colors)
{
    if (colors.isEmpty())
        return;

    m_highlightGradientColors = colors;
    update();
    saveSettings();
}

QVector<QColor> DesktopLyricWindow::highlightGradient() const
{
    return m_highlightGradientColors;
}

void DesktopLyricWindow::setOutlineColor(const QColor &color, int width)
{
    if (!color.isValid())
        return;

    m_outlineColor = color;
    m_outlineWidth = qMax(0, width);

    update();
    saveSettings();
}

QColor DesktopLyricWindow::outlineColor() const
{
    return m_outlineColor;
}

int DesktopLyricWindow::outlineWidth() const
{
    return m_outlineWidth;
}

void DesktopLyricWindow::setLyricFont(const QFont &font)
{
    m_lyricFont = font;
    update();
    saveSettings();
}

QFont DesktopLyricWindow::lyricFont() const
{
    return m_lyricFont;
}

void DesktopLyricWindow::loadSettings()
{
    QSettings settings("LyricPhase", "DesktopLyric");

    m_outlineColor = settings.value(
        "style/outlineColor",
        QColor(0, 0, 0, 220)
    ).value<QColor>();

    m_outlineWidth = settings.value(
        "style/outlineWidth",
        6
    ).toInt();

    m_lyricFont = settings.value(
        "style/font",
        QFont("Kaiti", 32, QFont::Bold)
    ).value<QFont>();

    // 读取高亮渐变颜色
    m_highlightGradientColors.clear();

    int count = settings.beginReadArray("style/highlightGradient");
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        QColor c = settings.value("color").value<QColor>();
        if (c.isValid())
            m_highlightGradientColors.append(c);
    }
    settings.endArray();

    if (m_highlightGradientColors.isEmpty()) {
        m_highlightGradientColors = {
            QColor("#000000"),
            QColor("#f8320b"),
            QColor("#000000")
        };
    }

    // 读取普通渐变颜色
    m_normalGradientColors.clear();

    int normalCount = settings.beginReadArray("style/normalGradient");
    for (int i = 0; i < normalCount; ++i) {
        settings.setArrayIndex(i);
        QColor c = settings.value("color").value<QColor>();
        if (c.isValid())
            m_normalGradientColors.append(c);
    }
    settings.endArray();

    if (m_normalGradientColors.isEmpty()) {
        m_normalGradientColors = {
            QColor("#cfcfcf"),
            QColor("#ffffff"),
            QColor("#cfcfcf")
        };
    }

    if (settings.contains("window/pos"))
        move(settings.value("window/pos").toPoint());

    if (settings.contains("window/size"))
        resize(settings.value("window/size").toSize());

    update();
}

void DesktopLyricWindow::saveSettings() const
{
    QSettings settings("LyricPhase", "DesktopLyric");

    // settings.setValue("style/normalColor", m_normalColor);
    settings.setValue("style/outlineColor", m_outlineColor);
    settings.setValue("style/outlineWidth", m_outlineWidth);
    settings.setValue("style/font", m_lyricFont);

    // 保存高亮渐变颜色
    settings.beginWriteArray("style/highlightGradient");
    for (int i = 0; i < m_highlightGradientColors.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("color", m_highlightGradientColors[i]);
    }
    settings.endArray();

    // 保存普通渐变颜色
    settings.beginWriteArray("style/normalGradient");
    for (int i = 0; i < m_normalGradientColors.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("color", m_normalGradientColors[i]);
    }
    settings.endArray();

    settings.setValue("window/pos", pos());
    settings.setValue("window/size", size());

    settings.sync();
}

void DesktopLyricWindow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    QFont font = m_lyricFont;
    p.setFont(font);


    // 构造文本路径
    QPainterPath path;
    path.addText(0, 0, font, m_text);

    QRectF bounds = path.boundingRect();
    qreal textWidth = bounds.width();

    qreal x = 0; // 横坐标
    qreal y = (height() + bounds.height()) / 2 - bounds.bottom();

    // 判断是否需要滚动
    if (textWidth <= width()) {
        // 居中显示
        x = (width() - textWidth) / 2 - bounds.left();
    } else {
        // 滚动显示
        if (!m_scrollTimer.isActive())
            m_scrollTimer.start(30);
        x = -m_scrollOffset; // m_scrollOffset 由定时器更新
        // qDebug() << "Painting with scrollOffset:" << m_scrollOffset << "textWidth:" << textWidth;
    }

    QPainterPath textPath;
    textPath.addText(x, y, font, m_text);

    // 描边
    QPen outlinePen(m_outlineColor, m_outlineWidth);
    p.strokePath(textPath, outlinePen);

    // 未播放文字
    QLinearGradient normalGradient(0, 0, 0, height());

    if (m_normalGradientColors.size() == 1) {
        normalGradient.setColorAt(0.0, m_normalGradientColors[0]);
        normalGradient.setColorAt(1.0, m_normalGradientColors[0]);
    } else {
        for (int i = 0; i < m_normalGradientColors.size(); ++i) {
            double pos = double(i) / double(m_normalGradientColors.size() - 1);
            normalGradient.setColorAt(pos, m_normalGradientColors[i]);
        }
    }

    p.fillPath(textPath, normalGradient);

    // 已播放高亮
    p.save();

    QRectF clipRect(0, 0, width() * m_progress, height());
    p.setClipRect(clipRect);

    QLinearGradient gradient(0, 0, 0, height());
    if (m_highlightGradientColors.size() == 1) {
        gradient.setColorAt(0.0, m_highlightGradientColors[0]);
        gradient.setColorAt(1.0, m_highlightGradientColors[0]);
    } else {
        for (int i = 0; i < m_highlightGradientColors.size(); ++i) {
            double pos = double(i) / double(m_highlightGradientColors.size() - 1);
            gradient.setColorAt(pos, m_highlightGradientColors[i]);
        }
    }

    p.fillPath(textPath, gradient);
    p.restore();
}

void DesktopLyricWindow::mousePressEvent(QMouseEvent *event)
{
    if (!m_locked && event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void DesktopLyricWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_locked && m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragOffset);
        event->accept();
    }
}

void DesktopLyricWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_locked && m_dragging && event->button() == Qt::LeftButton) {
        m_dragging = false;
        emit positionChanged(frameGeometry().topLeft());
        qDebug() << "Drag finished, position:" << frameGeometry().topLeft();
        saveSettings();
        event->accept();
    }
}


void DesktopLyricWindow::onScrollTimeout()
{
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(m_text);

    if (textWidth <= width()) {
        m_scrollOffset = 0;
        return;
    }

    m_scrollOffset += 2; // 每次偏移 2 px
    if (m_scrollOffset > textWidth)
        m_scrollOffset = 0; // 回到开头

    update();
}
