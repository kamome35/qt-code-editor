#ifndef TAGSMAKEDIALOG_H
#define TAGSMAKEDIALOG_H

#include <QDialog>

namespace Ui {
    class TagsMakeDialog;
}

class TagsMakeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TagsMakeDialog(QWidget *parent = 0);
    ~TagsMakeDialog();

public slots:
    void dirSelection();
    void setDirPath(QString dirPath);
    void upperDirectory();
    void makeTags();

private:
    Ui::TagsMakeDialog *ui;
};

#endif // TAGSMAKEDIALOG_H
