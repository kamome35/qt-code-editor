#include "replacedialog.h"
#include "ui_replacedialog.h"
#include <QTextCursor>
#include "highlighter.h"

ReplaceDialog::ReplaceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReplaceDialog),
    textEditor(0)
{
    ui->setupUi(this);

    connect(ui->findPrev, SIGNAL(released()), this, SLOT(findPrev()));
    connect(ui->findNext, SIGNAL(released()), this, SLOT(findNext()));
    connect(ui->replace, SIGNAL(released()), this, SLOT(replace()));
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

void ReplaceDialog::setCurrentTextEditor(TextEditor *textEditor)
{
    this->textEditor = textEditor;
    ui->before->setEditText(textEditor->textCursor().selectedText());
}

bool ReplaceDialog::replace(const QString &before, const QString &after,
                           TextEditor::KeywordOption option)
{
    QTextCursor cursor = textCursor();
    QRegExp regexp = Highlighter::convertText(before, option);

    if (regexp.exactMatch(cursor.selectedText())) {
        cursor.insertText(after);
        return true;
    }

    return false;
}

void ReplaceDialog::findPrev()
{
    if (!textEditor)
        return;

    TextEditor::KeywordOption option;
    option.caseSensitive = ui->caseSensitively->isChecked();
    option.wholeWords = ui->wholeWords->isChecked();
    option.regularExpression = ui->regularExpression->isChecked();

    QRegExp regexp = Highlighter::convertText(ui->before->currentText(), option);

    QTextCursor cursor = textEditor->document()->find(regexp, textEditor->textCursor(), QTextDocument::FindBackward);
    if (!cursor.isNull()) {
        textEditor->setTextCursor(cursor);
    } else {
        do {
            cursor = textEditor->textCursor();
            cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
            cursor = textEditor->document()->find(regexp, cursor, QTextDocument::FindBackward);
            if (!cursor.isNull()) {
                textEditor->setTextCursor(cursor);
                break;
            }

            QTextCursor cr = textEditor->textCursor();
            cr.clearSelection();
            textEditor->setTextCursor(cr);
        } while (0);
    }
}

void ReplaceDialog::findNext()
{
    if (!textEditor)
        return;

    TextEditor::KeywordOption option;
    option.caseSensitive = ui->caseSensitively->isChecked();
    option.wholeWords = ui->wholeWords->isChecked();
    option.regularExpression = ui->regularExpression->isChecked();

    QRegExp regexp = Highlighter::convertText(ui->before->currentText(), option);

    QTextCursor cursor = textEditor->document()->find(regexp, textEditor->textCursor());
    if (!cursor.isNull()) {
        textEditor->setTextCursor(cursor);
    } else {
        do {
            cursor = textEditor->textCursor();
            cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
            cursor = textEditor->document()->find(regexp, cursor);
            if (!cursor.isNull()) {
                textEditor->setTextCursor(cursor);
                break;
            }

            QTextCursor cr = textEditor->textCursor();
            cr.clearSelection();
            textEditor->setTextCursor(cr);
        } while (0);
    }
}

void ReplaceDialog::replace()
{
    qDebug("ReplaceDialog::replace");
    TextEditor::KeywordOption option;
    option.caseSensitive = ui->caseSensitively->isChecked();
    option.wholeWords = ui->wholeWords->isChecked();
    option.regularExpression = ui->regularExpression->isChecked();

    if (replace(ui->before->currentText(), ui->after->currentText(), option) && ui->nextSearch->isChecked()) {
        findNext();
    }
}

QTextCursor ReplaceDialog::textCursor() const
{
    return textEditor->textCursor();
}

void ReplaceDialog::setTextCursor(const QTextCursor &cursor) const
{
    textEditor->setTextCursor(cursor);
}
