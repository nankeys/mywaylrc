#include <QApplication>
#include <QWidget>
#include <QMouseEvent>
#include <QWindow>
#include <QDebug>

class DragTestWindow : public QWidget
{
    Q_OBJECT
public:
    DragTestWindow() {
        setWindowTitle("Wayland Drag Test");
        resize(400, 200);
        setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
        // setAttribute(Qt::WA_TranslucentBackground, true);
        // setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_TransparentForMouseEvents, false);

        // 允许鼠标移动事件在没有按键按下时也触发（可选）
        // setMouseTracking(true);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            m_dragging = true;

            if (windowHandle())
                windowHandle()->startSystemMove();

            m_dragStartPos = event->globalPos() - frameGeometry().topLeft();
        }
        qDebug() << "[Press ] Mouse pressed at:" << event->pos();
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (m_dragging && (event->buttons() & Qt::LeftButton)) {
            move(event->globalPos() - m_dragStartPos);
            QPoint delta = event->pos() - m_dragStartPos;
            qDebug() << "[Move  ] Mouse moved at:" << event->pos()
                      << ", delta:" << delta;
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton)
            m_dragging = false;
            qDebug() << "[Release] Mouse released at:" << event->pos();
    }
    // void mousePressEvent(QMouseEvent *event) override {
    //     if (event->button() == Qt::LeftButton) {
    //         m_dragging = true;
    //         m_dragStartPos = event->pos();
    //         qDebug() << "[Press ] Mouse pressed at:" << event->pos();
    //     }
    // }

    // void mouseMoveEvent(QMouseEvent *event) override {
    //     if (m_dragging && (event->buttons() & Qt::LeftButton)) {
    //         QPoint delta = event->pos() - m_dragStartPos;
    //         qDebug() << "[Move  ] Mouse moved at:" << event->pos()
    //                  << ", delta:" << delta;
    //     }
    // }

    // void mouseReleaseEvent(QMouseEvent *event) override {
    //     if (m_dragging && event->button() == Qt::LeftButton) {
    //         m_dragging = false;
    //         qDebug() << "[Release] Mouse released at:" << event->pos();
    //     }
    // }

private:
    bool m_dragging = false;
    QPoint m_dragStartPos;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DragTestWindow window;
    window.show();

    return app.exec();
}

#include "player.moc"
