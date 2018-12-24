#include <QtGui>
#include "finddialog.h"
#include "ui_finddialog.h"
#include "texteditor.h"

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Geometry");
    settings.beginGroup("FindDialog");

    restoreGeometry(settings.value("FindDialog").toByteArray());
    ui->findText->addItems(settings.value("findText").toStringList());
    ui->caseSensitively->setChecked(settings.value("caseSensitively", false).toBool());
    ui->wholeWords->setChecked(settings.value("wholeWords", false).toBool());
    ui->regularExpression->setChecked(settings.value("regularExpression", false).toBool());
    ui->warningNavi->setChecked(settings.value("warningNavi", true).toBool());
    ui->findkeep->setChecked(settings.value("findkeep", true).toBool());
    ui->realtime->setChecked(settings.value("realtime", true).toBool());

    connect(ui->findText, SIGNAL(editTextChanged(QString)), this, SLOT(realtimeFind(QString)));
    connect(ui->findFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(indexChangefindFormat(int)));
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::findPrev()
{
    FindParam param;
    param.index = ui->findFormat->currentIndex();
    param.data.text = ui->findText->currentText();
    param.data.option.caseSensitive = ui->caseSensitively->isChecked();
    param.data.option.wholeWords = ui->wholeWords->isChecked();
    param.data.option.regularExpression = ui->wholeWords->isChecked();
    param.findType = FindPrev;
    param.warningNavi = ui->warningNavi->isChecked();

    emit find(param);

    if (ui->findkeep->isChecked() && !ui->realtime->isChecked()) {
        close();
    }
}

void FindDialog::findNext()
{
    FindParam param;
    param.index = ui->findFormat->currentIndex();
    param.data.text = ui->findText->currentText();
    param.data.option.caseSensitive = ui->caseSensitively->isChecked();
    param.data.option.wholeWords = ui->wholeWords->isChecked();
    param.data.option.regularExpression = ui->wholeWords->isChecked();
    param.findType = FindNext;
    param.warningNavi = ui->warningNavi->isChecked();

    emit find(param);

    if (ui->findkeep->isChecked() && !ui->realtime->isChecked()) {
        close();
    }
}

void FindDialog::findMark()
{
    FindParam param;
    param.index = ui->findFormat->currentIndex();
    param.data.text = ui->findText->currentText();
    param.data.option.caseSensitive = ui->caseSensitively->isChecked();
    param.data.option.wholeWords = ui->wholeWords->isChecked();
    param.data.option.regularExpression = ui->wholeWords->isChecked();
    param.findType = FindMark;
    param.warningNavi = ui->warningNavi->isChecked();

    emit find(param);

    if (ui->findkeep->isChecked() && !ui->realtime->isChecked()) {
        close();
    }
}

void FindDialog::setText(QString text)
{
    ui->findText->setEditText(text);
}

void FindDialog::realtimeFind(QString text)
{
    if (!ui->realtime->isChecked()) return;

    FindParam param;
    param.index = ui->findFormat->currentIndex();
    param.data.text = text;
    param.data.option.caseSensitive = ui->caseSensitively->isChecked();
    param.data.option.wholeWords = ui->wholeWords->isChecked();
    param.data.option.regularExpression = ui->wholeWords->isChecked();
    param.findType = FindMark;
    param.warningNavi = false;

    emit find(param);
}

void FindDialog::setFindFormat(const QList<QVariant> &formats)
{
    findFormats = formats;
    updateFindFormat();
}

void FindDialog::updateFindFormat()
{
    ui->findFormat->clear();
    for (int i = 0; i < findFormats.count(); ++i) {
        const TextEditor::TextFormat &format = findFormats[i].value<TextEditor::TextFormat>();
        ui->findFormat->insertItem(i, format.name);
        ui->findFormat->setItemData(i, format.foreground, Qt::ForegroundRole);
        ui->findFormat->setItemData(i, format.background, Qt::DecorationRole);
        ui->findFormat->setItemData(i, format.background, Qt::BackgroundRole);
    }

    indexChangefindFormat(ui->findFormat->currentIndex());
}

void FindDialog::indexChangefindFormat(int i)
{
    /*
    ui->findFormat->setBackgroundRole(QPalette::Base);
    ui->findFormat->setAutoFillBackground(true);
    */
    QPalette p = ui->findFormat->palette();

    p.setBrush(QPalette::Text, ui->findFormat->itemData(i, Qt::ForegroundRole).value<QBrush>());
    p.setBrush(QPalette::Base, ui->findFormat->itemData(i, Qt::BackgroundRole).value<QBrush>());

    ui->findFormat->setPalette(p);
}

void FindDialog::closeEvent(QCloseEvent *event)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Geometry");
    settings.beginGroup("FindDialog");
    settings.setValue("caseSensitively", ui->caseSensitively->isChecked());
    settings.setValue("wholeWords", ui->wholeWords->isChecked());
    settings.setValue("regularExpression", ui->regularExpression->isChecked());
    settings.setValue("warningNavi", ui->warningNavi->isChecked());
    settings.setValue("findkeep", ui->findkeep->isChecked());
    settings.setValue("realtime", ui->realtime->isChecked());
    QStringList findText;
    for (int i = 0; i < ui->findText->count(); ++i) {
        findText << ui->findText->itemText(i);
    }
    settings.setValue("findText", findText);
    settings.setValue("FindDialog", saveGeometry());

    QDialog::closeEvent(event);
}
