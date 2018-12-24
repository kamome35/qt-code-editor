#include "configurationpage.h"
#include "ui_configurationpage.h"
#include "mainwindow.h"

extern MainWindow *mainWindow;

ConfigurationPage::ConfigurationPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigurationPage)
{
    ui->setupUi(this);

    ui->pos_x->setValue(mainWindow->x());
    ui->pos_y->setValue(mainWindow->y());
    ui->window_width->setValue(mainWindow->width());
    ui->window_height->setValue(mainWindow->height());
    ui->maximized->setChecked(mainWindow->isMaximized());
}

ConfigurationPage::~ConfigurationPage()
{
    delete ui;
}

void ConfigurationPage::setMainWindowPosX(int x)
{
    mainWindow->move(x, mainWindow->y());
}

void ConfigurationPage::setMainWindowPosY(int y)
{
    mainWindow->move(mainWindow->x(), y);
}

void ConfigurationPage::setMainWindowWidth(int width)
{
    mainWindow->resize(width, mainWindow->height());
}

void ConfigurationPage::setMainWindowHeight(int height)
{
    mainWindow->resize(mainWindow->width(), height);
}

void ConfigurationPage::setWindowMaximized(bool maximized)
{
    if (maximized)
        mainWindow->setWindowState(mainWindow->windowState() | Qt::WindowMaximized);
    else
        mainWindow->setWindowState(mainWindow->windowState() ^ Qt::WindowMaximized);
}
