#pragma once

#include <QDialog>
#include <QStringList>

class QListWidget;
class QPushButton;
class QDialogButtonBox;

class LyricsDirectoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LyricsDirectoryDialog(const QStringList &dirs,
                                   QWidget *parent = nullptr);

    QStringList directories() const;

private:
    void addDirectoryItem(const QString &dir);

private slots:
    void addDirectory();
    void editDirectory();
    void removeDirectory();

private:
    QListWidget *m_list = nullptr;
    QPushButton *m_addButton = nullptr;
    QPushButton *m_editButton = nullptr;
    QPushButton *m_removeButton = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
};
