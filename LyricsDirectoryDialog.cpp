#include "LyricsDirectoryDialog.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QSet>
#include <QVBoxLayout>

LyricsDirectoryDialog::LyricsDirectoryDialog(const QStringList &dirs,
                                             QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("歌词目录设置");
    resize(560, 360);

    auto *mainLayout = new QVBoxLayout(this);

    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(m_list);

    auto *buttonLayout = new QHBoxLayout;
    m_addButton = new QPushButton("添加目录", this);
    m_editButton = new QPushButton("编辑", this);
    m_removeButton = new QPushButton("删除", this);

    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_removeButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    m_buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );
    m_buttonBox->button(QDialogButtonBox::Ok)->setText("保存");
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");
    mainLayout->addWidget(m_buttonBox);

    for (const QString &dir : dirs)
        addDirectoryItem(dir);

    connect(m_addButton, &QPushButton::clicked,
            this, &LyricsDirectoryDialog::addDirectory);
    connect(m_editButton, &QPushButton::clicked,
            this, &LyricsDirectoryDialog::editDirectory);
    connect(m_removeButton, &QPushButton::clicked,
            this, &LyricsDirectoryDialog::removeDirectory);
    connect(m_buttonBox, &QDialogButtonBox::accepted,
            this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);
    connect(m_list, &QListWidget::itemDoubleClicked,
            this, [this]() { editDirectory(); });
}

QStringList LyricsDirectoryDialog::directories() const
{
    QStringList dirs;
    QSet<QString> seen;

    for (int i = 0; i < m_list->count(); ++i) {
        const QString dir = QDir(m_list->item(i)->text()).absolutePath();
        if (!dir.isEmpty() && !seen.contains(dir)) {
            seen.insert(dir);
            dirs << dir;
        }
    }

    return dirs;
}

void LyricsDirectoryDialog::addDirectoryItem(const QString &dir)
{
    const QString cleanDir = QDir(dir).absolutePath();
    if (cleanDir.isEmpty())
        return;

    for (int i = 0; i < m_list->count(); ++i) {
        if (QDir(m_list->item(i)->text()).absolutePath() == cleanDir)
            return;
    }

    m_list->addItem(cleanDir);
}

void LyricsDirectoryDialog::addDirectory()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this,
        "选择歌词目录",
        QDir::homePath()
    );

    if (dir.isEmpty())
        return;

    addDirectoryItem(dir);
}

void LyricsDirectoryDialog::editDirectory()
{
    if (!m_list->currentItem())
        return;

    const QString currentDir = m_list->currentItem()->text();
    const QString dir = QFileDialog::getExistingDirectory(
        this,
        "编辑歌词目录",
        currentDir
    );

    if (dir.isEmpty())
        return;

    m_list->currentItem()->setText(QDir(dir).absolutePath());
}

void LyricsDirectoryDialog::removeDirectory()
{
    delete m_list->takeItem(m_list->currentRow());
}
