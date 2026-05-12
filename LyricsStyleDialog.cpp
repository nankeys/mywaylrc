#include "LyricsStyleDialog.h"
#include "DesktopLyricWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QColorDialog>
#include <QFontDialog>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QListWidgetItem>

LyricsStyleDialog::LyricsStyleDialog(DesktopLyricWindow *lyricWindow,
                                     QWidget *parent)
    : QDialog(parent)
    , m_lyricWindow(lyricWindow)
{
    setWindowTitle("歌词样式设置");
    resize(520, 420);

    // 读取当前样式到临时变量
    m_normalGradientColors = m_lyricWindow->normalGradient();
    m_gradientColors = m_lyricWindow->highlightGradient();

    m_font = m_lyricWindow->lyricFont();
    m_outlineColor = m_lyricWindow->outlineColor();
    m_outlineWidth = m_lyricWindow->outlineWidth();

    if (m_normalGradientColors.isEmpty()) {
        m_normalGradientColors = {
            QColor("#cfcfcf"),
            QColor("#ffffff"),
            QColor("#cfcfcf")
        };
    }

    if (m_gradientColors.isEmpty()) {
        m_gradientColors = {
            QColor("#000000"),
            QColor("#f8320b"),
            QColor("#000000")
        };
    }

    auto *mainLayout = new QVBoxLayout(this);

    // 预览
    m_previewLabel = new QLabel("歌词样式预览 Preview", this);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setMinimumHeight(70);
    mainLayout->addWidget(m_previewLabel);

    // 未播放歌词渐变
    {
        mainLayout->addWidget(new QLabel("未播放歌词颜色:", this));

        m_normalGradientList = new QListWidget(this);
        m_normalGradientList->setMinimumHeight(100);
        mainLayout->addWidget(m_normalGradientList);

        auto *btnLayout = new QHBoxLayout;

        m_normalAddBtn = new QPushButton("添加", this);
        m_normalEditBtn = new QPushButton("编辑", this);
        m_normalRemoveBtn = new QPushButton("删除", this);

        btnLayout->addWidget(m_normalAddBtn);
        btnLayout->addWidget(m_normalEditBtn);
        btnLayout->addWidget(m_normalRemoveBtn);

        mainLayout->addLayout(btnLayout);

        connect(m_normalAddBtn, &QPushButton::clicked,
                this, &LyricsStyleDialog::addNormalGradientPoint);

        connect(m_normalEditBtn, &QPushButton::clicked,
                this, &LyricsStyleDialog::editNormalGradientPoint);

        connect(m_normalRemoveBtn, &QPushButton::clicked,
                this, &LyricsStyleDialog::removeNormalGradientPoint);
    }

    // 高亮渐变
    {
        mainLayout->addWidget(new QLabel("已播放歌词颜色:", this));

        m_gradientList = new QListWidget(this);
        m_gradientList->setMinimumHeight(120);
        mainLayout->addWidget(m_gradientList);

        auto *btnLayout = new QHBoxLayout;

        m_addBtn = new QPushButton("添加", this);
        m_editBtn = new QPushButton("编辑", this);
        m_removeBtn = new QPushButton("删除", this);

        btnLayout->addWidget(m_addBtn);
        btnLayout->addWidget(m_editBtn);
        btnLayout->addWidget(m_removeBtn);

        mainLayout->addLayout(btnLayout);

        connect(m_addBtn, &QPushButton::clicked,
                this, &LyricsStyleDialog::addGradientPoint);
        connect(m_editBtn, &QPushButton::clicked,
                this, &LyricsStyleDialog::editGradientPoint);
        connect(m_removeBtn, &QPushButton::clicked,
                this, &LyricsStyleDialog::removeGradientPoint);
    }

    // 字体
    {
        auto *layout = new QHBoxLayout;
        layout->addWidget(new QLabel("字体:", this));

        m_fontBtn = new QPushButton(this);
        layout->addWidget(m_fontBtn);

        mainLayout->addLayout(layout);

        connect(m_fontBtn, &QPushButton::clicked,
                this, &LyricsStyleDialog::chooseFont);
    }

    // 描边
    {
        auto *layout = new QHBoxLayout;
        layout->addWidget(new QLabel("轮廓颜色:", this));

        m_outlineColorBtn = new QPushButton(this);
        m_outlineColorBtn->setMinimumWidth(120);
        layout->addWidget(m_outlineColorBtn);

        layout->addWidget(new QLabel("轮廓粗细:", this));

        m_outlineWidthSpin = new QSpinBox(this);
        m_outlineWidthSpin->setRange(0, 30);
        m_outlineWidthSpin->setValue(m_outlineWidth);
        layout->addWidget(m_outlineWidthSpin);

        mainLayout->addLayout(layout);

        connect(m_outlineColorBtn, &QPushButton::clicked,
                this, &LyricsStyleDialog::chooseOutlineColor);

        connect(m_outlineWidthSpin,
                qOverload<int>(&QSpinBox::valueChanged),
                this,
                &LyricsStyleDialog::outlineWidthChanged);
    }

    // 确定 / 取消
    m_buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );

    m_buttonBox->button(QDialogButtonBox::Ok)->setText("确定");
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");

    mainLayout->addWidget(m_buttonBox);

    connect(m_buttonBox, &QDialogButtonBox::accepted,
            this, &LyricsStyleDialog::applyAndAccept);

    connect(m_buttonBox, &QDialogButtonBox::rejected,
            this, &LyricsStyleDialog::reject);

    refreshNormalGradientList();
    refreshGradientList();
    updateButtonColors();
    updatePreviewText();
}

void LyricsStyleDialog::addNormalGradientPoint()
{
    QColor color = QColorDialog::getColor(
        QColor("#ffffff"),
        this,
        "添加未播放歌词颜色"
    );

    if (!color.isValid())
        return;

    m_normalGradientColors.append(color);

    refreshNormalGradientList();
    updatePreviewText();
}

void LyricsStyleDialog::editNormalGradientPoint()
{
    int row = m_normalGradientList->currentRow();

    if (row < 0 || row >= m_normalGradientColors.size())
        return;

    QColor color = QColorDialog::getColor(
        m_normalGradientColors[row],
        this,
        "编辑未播放歌词颜色"
    );

    if (!color.isValid())
        return;

    m_normalGradientColors[row] = color;

    refreshNormalGradientList();
    updatePreviewText();
}

void LyricsStyleDialog::removeNormalGradientPoint()
{
    int row = m_normalGradientList->currentRow();

    if (row < 0 || row >= m_normalGradientColors.size())
        return;

    if (m_normalGradientColors.size() <= 1)
        return;

    m_normalGradientColors.removeAt(row);

    refreshNormalGradientList();
    updatePreviewText();
}

void LyricsStyleDialog::addGradientPoint()
{
    QColor color = QColorDialog::getColor(
        Qt::yellow,
        this,
        "添加已播放歌词颜色"
    );

    if (!color.isValid())
        return;

    m_gradientColors.append(color);

    refreshGradientList();
    updatePreviewText();
}

void LyricsStyleDialog::editGradientPoint()
{
    int row = m_gradientList->currentRow();
    if (row < 0 || row >= m_gradientColors.size())
        return;

    QColor color = QColorDialog::getColor(
        m_gradientColors[row],
        this,
        "编辑已播放歌词颜色"
    );

    if (!color.isValid())
        return;

    m_gradientColors[row] = color;

    refreshGradientList();
    updatePreviewText();
}

void LyricsStyleDialog::removeGradientPoint()
{
    int row = m_gradientList->currentRow();
    if (row < 0 || row >= m_gradientColors.size())
        return;

    if (m_gradientColors.size() <= 1)
        return;

    m_gradientColors.removeAt(row);

    refreshGradientList();
    updatePreviewText();
}

void LyricsStyleDialog::chooseFont()
{
    bool ok = false;

    QFont font = QFontDialog::getFont(
        &ok,
        m_font,
        this,
        "选择歌词字体"
    );

    if (!ok)
        return;

    m_font = font;

    updateButtonColors();
    updatePreviewText();
}

void LyricsStyleDialog::chooseOutlineColor()
{
    QColor color = QColorDialog::getColor(
        m_outlineColor,
        this,
        "选择轮廓颜色"
    );

    if (!color.isValid())
        return;

    m_outlineColor = color;

    updateButtonColors();
    updatePreviewText();
}

void LyricsStyleDialog::outlineWidthChanged(int width)
{
    m_outlineWidth = width;
    updatePreviewText();
}

void LyricsStyleDialog::applyAndAccept()
{
    if (!m_lyricWindow)
        return;

    m_lyricWindow->setNormalGradient(m_normalGradientColors);
    m_lyricWindow->setHighlightGradient(m_gradientColors);
    m_lyricWindow->setLyricFont(m_font);
    m_lyricWindow->setOutlineColor(m_outlineColor, m_outlineWidth);

    // 如果你的 setter 已经自动 saveSettings()，这里可以不写。
    // 如果 setter 没有自动保存，就保留这一句。
    m_lyricWindow->saveSettings();

    accept();
}

void LyricsStyleDialog::refreshNormalGradientList()
{
    m_normalGradientList->clear();

    for (int i = 0; i < m_normalGradientColors.size(); ++i) {
        const QColor &c = m_normalGradientColors[i];

        auto *item = new QListWidgetItem(
            QString("未播放渐变点 %1: %2")
                .arg(i + 1)
                .arg(c.name(QColor::HexArgb))
        );

        item->setBackground(c);
        item->setForeground(c.lightness() < 128 ? Qt::white : Qt::black);

        m_normalGradientList->addItem(item);
    }
}

void LyricsStyleDialog::refreshGradientList()
{
    m_gradientList->clear();

    for (int i = 0; i < m_gradientColors.size(); ++i) {
        const QColor &c = m_gradientColors[i];

        auto *item = new QListWidgetItem(
            QString("已播放渐变点 %1: %2").arg(i + 1).arg(c.name(QColor::HexArgb))
        );

        item->setBackground(c);
        item->setForeground(c.lightness() < 128 ? Qt::white : Qt::black);

        m_gradientList->addItem(item);
    }
}

void LyricsStyleDialog::updateButtonColors()
{
    // m_normalBtn->setStyleSheet(
    //     QString("background-color: %1;")
    //         .arg(m_normalColor.name(QColor::HexArgb))
    // );

    m_outlineColorBtn->setStyleSheet(
        QString("background-color: %1;")
            .arg(m_outlineColor.name(QColor::HexArgb))
    );

    m_fontBtn->setText(
        QString("%1, %2pt")
            .arg(m_font.family())
            .arg(m_font.pointSize())
    );
}

void LyricsStyleDialog::updatePreviewText()
{
    QString gradientCss;

    if (m_gradientColors.size() == 1) {
        gradientCss = m_gradientColors.first().name();
    } else {
        // QLabel 不支持真正多点文字渐变，这里只做文字说明预览。
        gradientCss = m_gradientColors.first().name();
    }

    QColor previewNormal =
    m_normalGradientColors.isEmpty()
        ? QColor("#ffffff")
        : m_normalGradientColors.first();

    m_previewLabel->setFont(m_font);

    m_previewLabel->setStyleSheet(QString(
        "QLabel {"
        "color: %1;"
        "background: rgba(30, 30, 30, 180);"
        "border: %2px solid %3;"
        "border-radius: 8px;"
        "padding: 8px;"
        "}"
    )
        .arg(previewNormal.name(QColor::HexArgb))
        .arg(qMax(1, m_outlineWidth))
        .arg(m_outlineColor.name(QColor::HexArgb))
    );

    m_previewLabel->setText("歌词样式预览 Preview");
}