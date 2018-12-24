#include "tagsmakedialog.h"
#include "ui_tagsmakedialog.h"
#include <QMenu>
#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QDesktopServices>

TagsMakeDialog::TagsMakeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TagsMakeDialog)
{
    ui->setupUi(this);

    ui->lineEdit->setText(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));

    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(dirSelection()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(upperDirectory()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(makeTags()));
}

TagsMakeDialog::~TagsMakeDialog()
{
    delete ui;
}

void TagsMakeDialog::dirSelection()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("ディレクトリ選択"),
                                            ui->lineEdit->text(),
                                            QFileDialog::ShowDirsOnly);
    setDirPath(dirPath);
}

void TagsMakeDialog::setDirPath(QString dirPath)
{
    QDir dir(dirPath);
    if (!dirPath.isEmpty() && dir.exists())
        ui->lineEdit->setText(dirPath);
}

void TagsMakeDialog::upperDirectory()
{
    QDir dir(ui->lineEdit->text());
    if (dir.cdUp())
        ui->lineEdit->setText(dir.path());
}

void TagsMakeDialog::makeTags()
{
    QDir dir(ui->lineEdit->text());
    QProcess proc(this);
    QStringList arguments;

    if (!dir.exists()) {
        QMessageBox::warning(this, "", tr("対象のディレクトリが存在しません: %1").arg(dir.path()));
        return;
    }
    if (QFile::exists(dir.path() + QDir::separator() + "tags")) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, "",
                             tr("タグファイルはすでに存在します。:%1\n上書きしますか？").arg(dir.path() + QDir::separator() + "tags"),
                             QMessageBox::Ok | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel)
            return;
    }

    arguments << "-n";
    if (ui->checkBox->isChecked())
        arguments << "-R";
    if (ui->languageType->currentIndex())
        arguments << "--languages=" + ui->languageType->currentText();
    if (!ui->options->text().isEmpty())
        arguments << ui->options->text();

    arguments << "-o " + dir.path() + QDir::separator() + "tags";

    foreach (const QString &fileName, dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
        arguments << dir.path() + QDir::separator() + fileName;
    }

    proc.start("ctags", arguments);
    if (!proc.waitForFinished()) {
        return;
    }
}
