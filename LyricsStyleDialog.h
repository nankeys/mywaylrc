#pragma once

#include <QDialog>
#include <QVector>
#include <QColor>
#include <QFont>

class DesktopLyricWindow;
class QListWidget;
class QPushButton;
class QSpinBox;
class QLabel;
class QDialogButtonBox;

class LyricsStyleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LyricsStyleDialog(DesktopLyricWindow *lyricWindow,
                               QWidget *parent = nullptr);

private slots:
    // 普通渐变颜色选择
    void addNormalGradientPoint();
    void editNormalGradientPoint();
    void removeNormalGradientPoint();

    // 高亮渐变颜色选择
    void addGradientPoint();
    void editGradientPoint();
    void removeGradientPoint();

    
    void chooseFont();
    void chooseOutlineColor();
    void outlineWidthChanged(int width);

    void applyAndAccept();

private:
    void refreshGradientList();
    void refreshNormalGradientList();
    void updateButtonColors();
    void updatePreviewText();

private:
    DesktopLyricWindow *m_lyricWindow = nullptr;

    QVector<QColor> m_normalGradientColors;
    QVector<QColor> m_gradientColors;
    QFont m_font;
    QColor m_outlineColor;
    int m_outlineWidth = 6;

    QPushButton *m_normalBtn = nullptr;

    QListWidget *m_gradientList = nullptr;
    QPushButton *m_addBtn = nullptr;
    QPushButton *m_editBtn = nullptr;
    QPushButton *m_removeBtn = nullptr;

    QListWidget *m_normalGradientList = nullptr;
    QPushButton *m_normalAddBtn = nullptr;
    QPushButton *m_normalEditBtn = nullptr;
    QPushButton *m_normalRemoveBtn = nullptr;

    QPushButton *m_fontBtn = nullptr;
    QPushButton *m_outlineColorBtn = nullptr;
    QSpinBox *m_outlineWidthSpin = nullptr;

    QLabel *m_previewLabel = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
};