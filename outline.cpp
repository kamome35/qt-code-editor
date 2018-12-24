#include "outline.h"
#include <QTreeWidget>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QTimer>


class TreeWidgetItem : public QTreeWidgetItem
{
public:
    enum COLUMN {
        COLUMN_FILE_LINE,
        COLUMN_FUNC_NAME,
        COLUMN_FUNC_TYPE,
        COLUMN_FILE_NAME,
        COLUMN_FUNC_SCOPE,
        COLUMN_MAX
    };

    TreeWidgetItem(QTreeWidget *parent, int type = Type)
        : QTreeWidgetItem(parent, type)
    {
        bgeinScopeLine = 0;
        endScopeLine = 0;
    }

    void setLineNumberScopeBegin(const int line)
    {
        bgeinScopeLine = line;
    }

    void setLineNumberScopeEnd(const int line)
    {
        endScopeLine = line;
    }

    bool isLineScopeRange(int line) const
    {
        return (bgeinScopeLine <= line) && (endScopeLine > line);
    }

private:
    bool operator< (const QTreeWidgetItem &other) const
    {
        int column = treeWidget()->sortColumn();
        if (column == COLUMN_FILE_LINE) {
            return text(column).toInt() < other.text(column).toInt();
        } else {
            return text(column).toAscii() < other.text(column).toAscii();
        }
    }
private:
    int bgeinScopeLine;
    int endScopeLine;
};

Outline::Outline(QWidget *parent) :
    QDockWidget(parent),
    changedSelectionEnable(true)
{
    setWindowTitle(tr("アウトライン"));

    treeWidget = new QTreeWidget(this);
    treeWidget->setSortingEnabled(true);
    treeWidget->sortItems (TreeWidgetItem::COLUMN_FILE_LINE, Qt::AscendingOrder);
    timer = new QTimer;

    QTreeWidgetItem *treewidgetitem = treeWidget->headerItem();
    treewidgetitem->setText(TreeWidgetItem::COLUMN_FILE_LINE, tr("行番号"));
    treewidgetitem->setText(TreeWidgetItem::COLUMN_FUNC_TYPE, tr("種類"));
    treewidgetitem->setText(TreeWidgetItem::COLUMN_FUNC_NAME, tr("関数名"));

    setWidget(treeWidget);

    connect(timer, SIGNAL(timeout()), this, SLOT(updateOutlineItems()));
    connect(treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(changeCurrentItem(QTreeWidgetItem*,QTreeWidgetItem*)));
}

void Outline::clear()
{
    treeWidget->clear();
}

void Outline::reload(QString fileName)
{
    this->fileName = fileName;
    timer->start(700);
}

void Outline::changeCurrentItem(QTreeWidgetItem *current, QTreeWidgetItem */*previous*/)
{
    if (!current) return;

    int line = current->text(TreeWidgetItem::COLUMN_FILE_LINE).toInt();

    if (changedSelectionEnable)
        emit changedSelection(line);
}

void Outline::selectionLineItem(int line)
{
    changedSelectionEnable = false;
    int size = treeWidget->topLevelItemCount();
    for (int i = 0; i < size; ++i) {
        TreeWidgetItem *item = dynamic_cast<TreeWidgetItem *>(treeWidget->topLevelItem(i));
        if (item && item->isLineScopeRange(line)) {
            treeWidget->setCurrentItem(item);
            break;
        }
    }
    changedSelectionEnable = true;
}

void Outline::updateFindwordMatchLines(const QVector<int> &matchLines)
{
    int size = treeWidget->topLevelItemCount();
    for (int i = 0; i < size; ++i) {
        TreeWidgetItem *item = dynamic_cast<TreeWidgetItem *>(treeWidget->topLevelItem(i));
        if (item) {
            item->setBackgroundColor(TreeWidgetItem::COLUMN_FILE_LINE, "#ffffff");
            item->setBackgroundColor(TreeWidgetItem::COLUMN_FUNC_NAME, "#ffffff");
            item->setBackgroundColor(TreeWidgetItem::COLUMN_FUNC_TYPE, "#ffffff");
            foreach (const int &line, matchLines) {
                if (item->isLineScopeRange(line)) {
                    item->setBackgroundColor(TreeWidgetItem::COLUMN_FILE_LINE, "#ffff00");
                    item->setBackgroundColor(TreeWidgetItem::COLUMN_FUNC_NAME, "#ffff00");
                    item->setBackgroundColor(TreeWidgetItem::COLUMN_FUNC_TYPE, "#ffff00");
                    break;
                }
            }
        }
    }
}

void Outline::updateOutlineItems()
{
    timer->stop();
    clear();

    QProcess proc(this);
    QStringList arguments;

    arguments << "-n";
    arguments << "-f -";
    arguments << "-u";
    arguments << "--languages=C++";
    arguments << "--langmap=C++:(myeditor_outline)";
    arguments << fileName;

    proc.start("ctags", arguments);
    if (!proc.waitForFinished(1000)) {
        return;
    }
    QStringList lines = QString::fromLatin1(proc.readAllStandardOutput()).split("\n");
    lines.removeLast();

    TreeWidgetItem *prev_item = 0;
    foreach (const QString &line, lines) {
        QStringList column = line.split("\t");
        TreeWidgetItem *item = new TreeWidgetItem(treeWidget);
        item->setText(TreeWidgetItem::COLUMN_FILE_LINE, column[TAGS_FILE_LINE].replace(QRegExp("..$"), ""));
        item->setText(TreeWidgetItem::COLUMN_FUNC_NAME, column[TAGS_FUNC_NAME]);
        switch (column[TAGS_FUNC_TYPE].at(0).toAscii()) {
        case 'f':
            item->setText(TreeWidgetItem::COLUMN_FUNC_TYPE, tr("関数"));
            break;
        case 'd':
            item->setText(TreeWidgetItem::COLUMN_FUNC_TYPE, tr("定義"));
            break;
        case 's':
            item->setText(TreeWidgetItem::COLUMN_FUNC_TYPE, tr("構造体"));
            break;
        case 'm':
            item->setText(TreeWidgetItem::COLUMN_FUNC_TYPE, tr("メンバー"));
            break;
        case 'g':
            item->setText(TreeWidgetItem::COLUMN_FUNC_TYPE, tr("列挙体"));
            break;
        case 'e':
            item->setText(TreeWidgetItem::COLUMN_FUNC_TYPE, tr("列挙値"));
            break;
        case 'v':
            item->setText(TreeWidgetItem::COLUMN_FUNC_TYPE, tr("変数"));
            break;
        case 'c':
            item->setText(TreeWidgetItem::COLUMN_FUNC_TYPE, tr("クラス"));
            break;
        case 'n':
            item->setText(TreeWidgetItem::COLUMN_FUNC_TYPE, tr("名前空間"));
            break;
        default:
            item->setText(TreeWidgetItem::COLUMN_FUNC_TYPE, tr("不明"));
            break;
        }
        item->setLineNumberScopeBegin(item->text(TreeWidgetItem::COLUMN_FILE_LINE).toInt());
        if (prev_item)
            prev_item->setLineNumberScopeEnd(item->text(TreeWidgetItem::COLUMN_FILE_LINE).toInt());
        prev_item = item;
    }
    if (prev_item)
        prev_item->setLineNumberScopeEnd(prev_item->text(TreeWidgetItem::COLUMN_FILE_LINE).toInt() + 999);
}

