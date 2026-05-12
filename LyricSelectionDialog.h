#pragma once
#include <QDialog>
#include <QStringList>

class QListWidget;
class QPushButton;

class LyricSelectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LyricSelectionDialog(const QStringList &files, QWidget *parent = nullptr);

    QString selectedFile() const;

private:
    QListWidget *m_list;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QString m_selected;
};