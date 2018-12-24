#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>
#include "texteditor.h"

namespace Ui {
    class ReplaceDialog;
}

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent = 0);
    ~ReplaceDialog();
    void setCurrentTextEditor(TextEditor *textEditor);
    bool replace(const QString &before, const QString &after, TextEditor::KeywordOption option);

public slots:
    void findPrev();
    void findNext();
    void replace();

protected:
    QTextCursor textCursor() const;
    void setTextCursor(const QTextCursor &cursor) const;

private:
    Ui::ReplaceDialog *ui;
    TextEditor *textEditor;
};

#endif // REPLACEDIALOG_H
