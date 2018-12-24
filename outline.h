#ifndef OUTLINE_H
#define OUTLINE_H

#include <QDockWidget>

class QTreeWidget;
class QTreeWidgetItem;
class QTimer;

class Outline : public QDockWidget
{
    Q_OBJECT
    enum TAGS_COLUMN {
        TAGS_FUNC_NAME,
        TAGS_FILE_NAME,
        TAGS_FILE_LINE,
        TAGS_FUNC_TYPE,
        TAGS_FUNC_SCOPE,
        TAGS_MAX
    };

public:
    explicit Outline(QWidget *parent = 0);
    void clear();
    void reload(QString fileName);
    void selectionLineItem(int line);
    void updateFindwordMatchLines(const QVector<int> &matchLines);
signals:
    void changedSelection(int);

public slots:
    void changeCurrentItem(QTreeWidgetItem * current, QTreeWidgetItem * previous);
    void updateOutlineItems();

private:
    QTreeWidget *treeWidget;
    QTimer *timer;
    QString fileName;
    bool changedSelectionEnable;

};

#endif // OUTLINE_H
