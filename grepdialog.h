#ifndef GREPDIALOG_H
#define GREPDIALOG_H

#include <QDialog>

namespace Ui {
    class GrepDialog;
}

class GrepDialog : public QDialog
{
    Q_OBJECT
public:
    enum MODE {
        MODE_CURRENT_WINDOW,
        MODE_WINDOW,
        MODE_DIR
    };

public:
    explicit GrepDialog(QWidget *parent = 0);
    ~GrepDialog();
    void updateMode(MODE mode);

public slots:
    void updateMode(int mode)
    {
        updateMode(static_cast<MODE>(mode));
    }

private:
    Ui::GrepDialog *ui;
};

#endif // GREPDIALOG_H
