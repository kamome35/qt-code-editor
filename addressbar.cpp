#include "addressbar.h"
#include <QLineEdit>

AddressBar::AddressBar(QWidget *parent) :
    QToolBar(parent)
{
    setAllowedAreas(Qt::TopToolBarArea);
    setMovable(false);
    setFloatable(false);
    QLineEdit *line = new QLineEdit(this);
    line->setStyleSheet("border: 1px solid gray; border-radius: 3px;");
    addWidget(line);

    setWindowTitle(tr("アドレスバー"));
}
