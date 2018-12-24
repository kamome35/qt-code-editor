#ifndef CONFIGURATIONPAGE_H
#define CONFIGURATIONPAGE_H

#include <QWidget>

namespace Ui {
class ConfigurationPage;
}

class ConfigurationPage : public QWidget
{
    Q_OBJECT
    
public:
    explicit ConfigurationPage(QWidget *parent = 0);
    ~ConfigurationPage();

private slots:
    void setMainWindowPosX(int x);
    void setMainWindowPosY(int y);
    void setMainWindowWidth(int width);
    void setMainWindowHeight(int height);
    void setWindowMaximized(bool maximized);
    
private:
    Ui::ConfigurationPage *ui;
};

#endif // CONFIGURATIONPAGE_H
