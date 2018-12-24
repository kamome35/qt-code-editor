#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include "texteditor.h"

class QSettings;

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    typedef enum {
        FindNext,
        FindPrev,
        FindMark
    } FindType;

    typedef struct {
        int index;
        TextEditor::KeywordData data;
        FindType findType;
        bool warningNavi;
    } FindParam;

public:
    explicit FindDialog(QWidget *parent = 0);
    ~FindDialog();
    void setText(QString text);
    void setFindFormat(const QList<QVariant> &formats);

public slots:
    void findPrev();
    void findNext();
    void findMark();
    void realtimeFind(QString text);
    void updateFindFormat();
    void indexChangefindFormat(int i);

signals:
    void find(FindDialog::FindParam param);

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::FindDialog *ui;
    QList<QVariant> findFormats;
};

#endif // FINDDIALOG_H
