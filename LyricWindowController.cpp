#include "LyricWindowController.h"
#include <LayerShellQt/Window>
#include <QMargins>
#include <QTimer>
#include <QDebug>

LyricWindowController::LyricWindowController(QObject *parent)
    : QObject(parent)
{
    recreateWindow();
}

DesktopLyricWindow* LyricWindowController::window() const { return m_window; }

bool LyricWindowController::isLocked() const { return m_locked; }

void LyricWindowController::setLocked(bool locked)
{
    if (m_locked == locked)
        return;

    // 保存当前窗口位置
    if (m_window)
        m_pos = m_window->frameGeometry().topLeft();

    m_locked = locked;
    recreateWindow();
}

void LyricWindowController::setLyric(const QString &text)
{
    m_text = text;
    if (m_window) m_window->setLyric(text);
}

void LyricWindowController::setProgress(double progress)
{
    m_progress = progress;
    if (m_window) m_window->setProgress(progress);
}

void LyricWindowController::recreateWindow()
{
    if (m_window) {
        m_window->hide();
        m_window->deleteLater();
        m_window = nullptr;
    }

    auto *w = new DesktopLyricWindow;
    m_window = w;

    w->setLyric(m_text);
    w->setProgress(m_progress);
    w->resize(900, 100);

    // 连接拖动结束更新位置
    connect(w, &DesktopLyricWindow::positionChanged, this, [&](const QPoint &p){
        m_pos = p;
        qDebug() << "Updated pos: " << p;
        qDebug() << "Updated m_pos: " << m_pos;
    });

    QTimer *savePosTimer = new QTimer(this);
    connect(savePosTimer, &QTimer::timeout, this, [this]() {
        if (m_window && !m_locked) {
            m_pos = m_window->frameGeometry().topLeft();
        }
    });
    savePosTimer->start(50);

    if (m_locked) {
        qDebug() << "Creating locked window at pos:" << m_pos;
        w->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus | Qt::WindowTransparentForInput);
        w->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        w->winId();

        auto *layer = LayerShellQt::Window::get(w->windowHandle());
        if (layer) {
            layer->setLayer(LayerShellQt::Window::LayerOverlay);
            layer->setExclusiveZone(-1);
            layer->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);

            LayerShellQt::Window::Anchors anchors;
            anchors |= LayerShellQt::Window::AnchorTop;
            anchors |= LayerShellQt::Window::AnchorLeft;
            layer->setAnchors(anchors);

            layer->setMargins(QMargins(m_pos.x(), m_pos.y(), 0, 0));
        }

        w->show();
    } else {
        qDebug() << "Creating unlocked window at pos:" << m_pos;
        w->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
        w->setAttribute(Qt::WA_TransparentForMouseEvents, false);

        w->show();
        w->raise();

        // 延迟处理，确保 Wayland surface 映射完成
        QTimer::singleShot(50, [this, w]() {
            w->move(m_pos);
        });
    }
}