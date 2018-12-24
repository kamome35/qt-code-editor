#include "grepdialog.h"
#include "ui_grepdialog.h"

GrepDialog::GrepDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GrepDialog)
{
    ui->setupUi(this);

    updateMode(ui->mode->currentIndex());

    connect(ui->mode, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMode(int)));
}

GrepDialog::~GrepDialog()
{
    delete ui;
}

void GrepDialog::updateMode(MODE mode)
{
    switch (mode) {
    case MODE_CURRENT_WINDOW:
        ui->dirsettings->setHidden(true);
        break;
    case MODE_WINDOW:
        ui->dirsettings->setHidden(true);
        break;
    case MODE_DIR:
        ui->dirsettings->setShown(true);
        break;
    default:
        break;
    }

    resize(0, 0);
}
