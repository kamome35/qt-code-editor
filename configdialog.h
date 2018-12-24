#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QListWidgetItem>
#include <QSettings>
#include <QDialog>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

    QSettings* settings;
public:
    explicit ConfigDialog(QWidget *parent = 0);
    ~ConfigDialog();

public slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);

private:
    void createPages();
    void createConfigurationPage();
    void createConfigEditorPage();

private:
    Ui::ConfigDialog *ui;
};

#endif // CONFIGDIALOG_H
