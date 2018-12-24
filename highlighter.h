#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include "texteditor.h"

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    typedef struct tagKeywordRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    } KeywordRule;

    typedef struct tagBlockwordRule
    {
        int index;
        int depth;
        QRegExp beginPattern;
        QRegExp endPattern;
        QTextCharFormat format;
    } BlockwordRule;
    explicit Highlighter(QTextDocument *parent = 0);
    void rehighlight();
    void updateHighlightFormats(const QList<QVariant> &formats);
    void updateFindFormats(const QList<QVariant> &formats);
    void updateKeywords(const QList<QVariant> &words);
    void updateBlockwords(const QList<QVariant> &words);
    void updateFindwords(const TextEditor::KeywordData words[]);
    QVector<int> findwordMatchLines(int index) const;
    inline int findwordCount(int index) const
    {
        return findwordsCount[index];
    }

protected:
    void highlightBlock(const QString &text);

public:
    static QRegExp convertText(QString text, const TextEditor::KeywordOption &option);
    static QTextCharFormat convertFormat(const TextEditor::TextFormat &format);

private:
    QList<QVariant> highlightFormats;
    QList<QVariant> findFormats;
    QList<QVariant> keywords;
    QList<QVariant> blockwords;
    TextEditor::KeywordData findwords[10];
    QVector<KeywordRule> keywordRules;
    QVector<BlockwordRule> blockwordRules;
    QVector<KeywordRule> findRules;
    int findwordsCount[10];
    QVector<int> findwordsLine[10];
};

#endif // HIGHLIGHTER_H
