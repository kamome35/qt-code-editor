#include <QDesktopWidget>
#include <QtGui>
#include "configdialog.h"
#include "ui_configdialog.h"
#include "configpages/configurationpage.h"
#include "configpages/configeditorpage.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    this->createPages();

    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "ConfigDialog");

    restoreGeometry(settings->value("geometry").toByteArray());
}

ConfigDialog::~ConfigDialog()
{
    settings->setValue("geometry", saveGeometry());

    delete ui;
}


void ConfigDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    ui->pagesWidget->setCurrentIndex(ui->contentsWidget->row(current));
}

void ConfigDialog::createPages()
{
    createConfigurationPage();
    createConfigEditorPage();
}

void ConfigDialog::createConfigurationPage()
{
    QListWidgetItem *item = new QListWidgetItem(ui->contentsWidget);
    item->setText(tr("ウィンドウ"));
    item->setIcon(QIcon(":/images/application-sidebar-list.png"));
    ConfigurationPage *widget = new ConfigurationPage(this);
    ui->pagesWidget->addWidget(widget);
}

void ConfigDialog::createConfigEditorPage()
{
    QListWidgetItem *item = new QListWidgetItem(ui->contentsWidget);
    item->setText(tr("エディタ"));
    item->setIcon(QIcon(":/images/document-text.png"));
    ConfigEditorPage *widget = new ConfigEditorPage(this);
    ui->pagesWidget->addWidget(widget);
}
