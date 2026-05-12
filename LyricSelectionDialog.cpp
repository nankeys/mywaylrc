#include "LyricSelectionDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QDebug>

LyricSelectionDialog::LyricSelectionDialog(const QStringList &files, QWidget *parent)
    : QDialog(parent), m_selected("")
{
    setWindowTitle("Select Lyric File");
    resize(500, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_list = new QListWidget(this);
    m_list->addItems(files);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);

    layout->addWidget(m_list);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_okButton = new QPushButton("OK", this);
    m_cancelButton = new QPushButton("Cancel", this);

    btnLayout->addStretch();
    btnLayout->addWidget(m_okButton);
    btnLayout->addWidget(m_cancelButton);

    layout->addLayout(btnLayout);

    connect(m_okButton, &QPushButton::clicked, this, [this]() {
        if (!m_list->currentItem()) return;
        m_selected = m_list->currentItem()->text();
        accept();
    });

    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // 双击列表项也直接选择
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item){
        m_selected = item->text();
        accept();
    });
}

QString LyricSelectionDialog::selectedFile() const
{
    return m_selected;
}