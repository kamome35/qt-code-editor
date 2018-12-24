#include <QtGui>
#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    memset(findwordsCount, 0, sizeof(findwordsCount));
}

void Highlighter::rehighlight()
{
    keywordRules.clear();
    foreach (const QVariant &keyword, keywords) {
        const TextEditor::KeywordData &data = keyword.value<TextEditor::KeywordData>();
        if ((unsigned int)highlightFormats.count() <= (unsigned int)data.highlightIndex) continue;
        const TextEditor::TextFormat &format = highlightFormats[data.highlightIndex].value<TextEditor::TextFormat>();
        if (!format.enabled) continue;
        Highlighter::KeywordRule rule;
        rule.pattern = Highlighter::convertText(data.text, data.option);
        rule.format = Highlighter::convertFormat(format);
        keywordRules.append(rule);
    }

    blockwordRules.clear();
    foreach (const QVariant &keyword, blockwords) {
        const TextEditor::BlockwordData &data = keyword.value<TextEditor::BlockwordData>();
        if ((unsigned int)highlightFormats.count() <= (unsigned int)data.highlightIndex) continue;
        const TextEditor::TextFormat &format = highlightFormats[data.highlightIndex].value<TextEditor::TextFormat>();
        if (!format.enabled) continue;
        Highlighter::BlockwordRule rule;
        rule.beginPattern = Highlighter::convertText(data.beginText, data.option);
        rule.endPattern = Highlighter::convertText(data.endText, data.option);
        rule.format = Highlighter::convertFormat(format);
        blockwordRules.append(rule);
    }

    findRules.clear();
    for (int i = 0; i < 10; ++i) {
        const TextEditor::KeywordData &data = findwords[i];
        if (findFormats.count() <= i) continue;
        const TextEditor::TextFormat &format = findFormats[i].value<TextEditor::TextFormat>();
        if (!format.enabled) continue;
        Highlighter::KeywordRule rule;
        rule.pattern = Highlighter::convertText(data.text, data.option);
        rule.format = Highlighter::convertFormat(format);
        findRules.append(rule);
        findwordsCount[i] = 0;
        findwordsLine[i].clear();
    }

    QSyntaxHighlighter::rehighlight();
}

void Highlighter::updateHighlightFormats(const QList<QVariant> &formats)
{
    highlightFormats = formats;
    rehighlight();
}

void Highlighter::updateFindFormats(const QList<QVariant> &formats)
{
    findFormats = formats;
    rehighlight();
}

void Highlighter::updateKeywords(const QList<QVariant> &keywords)
{
    this->keywords = keywords;
    rehighlight();
}

void Highlighter::updateBlockwords(const QList<QVariant> &blockwords)
{
    this->blockwords = blockwords;
    rehighlight();
}

void Highlighter::updateFindwords(const TextEditor::KeywordData words[])
{
    memcpy(findwords, words, sizeof(findwords));
    rehighlight();
}

QVector<int> Highlighter::findwordMatchLines(int index) const
{
    return findwordsLine[index];
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (const KeywordRule &rule, keywordRules) {
        if (!rule.pattern.isValid()) { continue; }
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            if (!length) break;
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }

    for (int i = 0; i < blockwordRules.size(); ++i)
    {
        if (!blockwordRules[i].beginPattern.isValid()) { continue; }
        if (!blockwordRules[i].endPattern.isValid()) { continue; }
        setCurrentBlockState(0);

        int startIndex = 0;
        int addLen = 0;
        if (previousBlockState() != (i+1))
        {
            startIndex = text.indexOf(blockwordRules[i].beginPattern);
            addLen = blockwordRules[i].beginPattern.matchedLength();
        }

        while (startIndex >= 0)
        {
            int endIndex = text.indexOf(blockwordRules[i].endPattern, startIndex + addLen);
            int length;
            if (endIndex >= addLen)
            {
                setCurrentBlockState(0);
                length = endIndex - startIndex
                        + blockwordRules[i].endPattern.matchedLength();
            }
            else
            {
                setCurrentBlockState(i+1);
                length = text.length() - startIndex;
            }
            if (!length) break;
            setFormat(startIndex, length, blockwordRules[i].format);
            startIndex = text.indexOf(blockwordRules[i].beginPattern,
                                      startIndex + length);
        }
        if (currentBlockState() == (i+1) )
        {
            break;
        }
    }

    int coun_index = -1;
    foreach (const KeywordRule &rule, findRules) {
        ++coun_index;
        if (!rule.pattern.isValid()) { continue; }
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            if (!length) break;
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
            ++findwordsCount[coun_index];
            findwordsLine[coun_index].append(currentBlock().blockNumber() + 1);
        }
    }
}

QRegExp Highlighter::convertText(QString text, const TextEditor::KeywordOption &option)
{
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;

    if (option.caseSensitive) {
        cs = Qt::CaseSensitive;
    }
    if (!option.regularExpression) {
        text = QRegExp::escape(text);
    }
    if (option.wholeWords) {
        text = "\\b" + text + "\\b";
    }
    return QRegExp(text, cs);
}
QTextCharFormat Highlighter::convertFormat(const TextEditor::TextFormat &format)
{
    QTextCharFormat toFormat;
    toFormat.setForeground(format.foreground);
    toFormat.setBackground(format.background);
    toFormat.setFontWeight(format.option.bold ? QFont::Bold : QFont::Normal);
    toFormat.setFontItalic(format.option.italic);
    toFormat.setFontUnderline(format.option.underline);

    return toFormat;
}
