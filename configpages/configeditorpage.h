#ifndef CONFIGEDITORPAGE_H
#define CONFIGEDITORPAGE_H

#include <QWidget>
#include <QObject>
#include <QListWidgetItem>
#include "texteditor.h"

class QTreeWidgetItem;
class QSignalMapper;

namespace Ui {
class ConfigEditorPage;
}

class ConfigEditorPage : public QWidget
{
    Q_OBJECT
    typedef enum tagKeywordItem {
        KeywordItemText,
        KeywordItemOption,
        KeywordItemColor,
        KeywordItemUnknown
    } KeywordItem;

    typedef enum tagBlockwordItem {
        BlockwordItemBeginText,
        BlockwordItemEndText,
        BlockwordItemOption,
        BlockwordItemColor,
        BlockwordItemUnknown
    } BlockwordItem;
public:
    explicit ConfigEditorPage(QWidget *parent = 0);
    ~ConfigEditorPage();

private slots:
    void tabChanged(int index);
    void currentFileNameChanged(TextEditor *textEdit);
    int findEditorType(const QString &key);
    void currentEditorTypeChanged(QString text);
    void currentEditorTypeChanged(TextEditor *textEdit);
    void editorTypeChanged(int index);
    void editorTypeNameChanged(QString);
    void suffixesChanged(QString suffixes);
    void addEditorType();
    void removeEditorType();
    void fontFamilyChanged(QString family);
    void fontPointSizeFChanged(double size);
    void zoomChanged(int zoom);
    void lineNumberVisibleChanged(bool visible);
    void columnNumberVisibleChanged(bool visible);
    void tabVisibleChanged(bool visible);
    void tabCharChanged(const QString &text);
    void tabStopDigitsChanged(int digits);
    void halfSpaceVisibleChanged(bool visible);
    void halfSpaceCharChanged(const QString &text);
    void fullSpaceVisibleChanged(bool visible);
    void fullSpaceCharChanged(const QString &text);
    void endOfLineVisibleChanged(bool visible);
    void endOfLineCharChanged(const QString &text);
    void endOfFileVisibleChanged(bool visible);
    void endOfFileCharChanged(const QString &text);
    void defTextCodecNameChanged(QString textCodecName);
    void textFormatItemChanged(QListWidgetItem *item);
    void textFormatEnabledChanged();
    void textFormatNameChanged();
    void textFormatForegroundChanged();
    void textFormatBackgroundChanged();
    void toggleTextFormatBold(bool enabled);
    void toggleTextFormatItalic(bool enabled);
    void toggleTextFormatUnderline(bool enabled);
    void currentKeywordItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void keywordChanged(QTreeWidgetItem *item, int column);
    void addKeyword();
    void removeKeyword();
    void keywordTextChanged(QString text);
    void changedKeywordHighlightFormats(int index);
    void currentBlockwordItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void blockwordChanged(QTreeWidgetItem *item, int column);
    void addBlockword();
    void removeBlockword();
    void blockwordBeginTextChanged(QString text);
    void blockwordEndTextChanged(QString text);
    void blockwordHighlightFormatsChanged(int index);
    void toggleKeywordCaseSensitive(bool enabled);
    void toggleKeywordWholeWords(bool enabled);
    void toggleKeywordRegularExpression(bool enabled);
    void toggleBlockwordCaseSensitive(bool enabled);
    void toggleBlockwordWholeWords(bool enabled);
    void toggleBlockwordRegularExpression(bool enabled);
    void updateBasicFormat();                     // 基本色
    void updateStripeFormat();                    // ストライプ
    void updateLineNumberFormat();                // 行番号
    void updateLineNumberCurrentFormat();         // 現在の行
    void updateColumnNumberFormat();              // 列番号
    void updateColumnNumberCurrentFormat();       // 現在の列
    void updateHighlightFormat(const int index);
    void updateFindFormat(const int index);
    void updateHalfSpaceVisibleFormat();          // 半角空白色
    void updateFullSpaceVisibleFormat();          // 全角空白色
    void updateTabVisibleFormat();                // タブ色
    void updateEndOfLineFormat();                 // 改行コード色
    void updateEndOfFileFormat();                 // EOF色
    void updateCurrentLineFormat();
    void updateCurrentColumnFormat();
    void updateSelectionFormat();                 // 選択色
    void keywordFilterChanged();
    void blockwordFilterChanged();

private:
    void updateEditorType();
    void updateConfigKeyword();
    void updateConfigBlockkeyword();
    void setKeywordOption(QTreeWidgetItem *item, const TextEditor::KeywordOption &option);
    void setBlockwordOption(QTreeWidgetItem *item, const TextEditor::KeywordOption &option);
    QTreeWidgetItem *createKeyword(const TextEditor::KeywordData &data);
    QTreeWidgetItem *createBlockword(const TextEditor::BlockwordData &data);
    void updateTextEditorConfig();
    void updateLayout();
    void updateBehavior();
    void updatedTextFormats();
    void updatedKeywords();
    void updatedBlockKeywords();
    TextEditor::ConfigType currentConfigType() const;
    TextEditor::TextFormat currentFormat() const;
    TextEditor *currentTextEditor() const;

private:
    Ui::ConfigEditorPage *ui;
    QSignalMapper *highlightFormatMapper;
    QSignalMapper *findFormatMapper;
    QList<QVariant> editorTypes;
    TextEditor::Config configs;
    QMap<QString, QVariant> keywordMap;
    QMap<QString, QVariant> blokwordMap;
};



class EditorFormatItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    enum VisableItemFlag {
        ItemEnableVisable     = 1 << 1,
        ItemNameVisable       = 1 << 2,
        ItemForegroundVisable = 1 << 3,
        ItemBackgroundVisable = 1 << 4,
        ItemBoldVisable       = 1 << 5,
        ItemItalicVisable     = 1 << 6,
        ItemUnderlineVisable  = 1 << 7,
        ItemFontStyleVisable  = ItemBoldVisable | ItemItalicVisable | ItemUnderlineVisable,
        ItemVisableAll        = -1
    };
    Q_DECLARE_FLAGS(VisableItemFlags, VisableItemFlag)
    enum EnableItemFlag {
        ItemEnableEnable     = 1 << 1,
        ItemNameEnable       = 1 << 2,
        ItemForegroundEnable = 1 << 3,
        ItemBackgroundEnable = 1 << 4,
        ItemBoldEnable       = 1 << 5,
        ItemItalicEnable     = 1 << 6,
        ItemUnderlineEnable  = 1 << 7,
        ItemFontStyleEnable  = ItemBoldEnable | ItemItalicEnable | ItemUnderlineEnable,
        ItemEnableAll        = -1
    };
    Q_DECLARE_FLAGS(EnableItemFlags, EnableItemFlag)
public:
    EditorFormatItem() : QListWidgetItem(), _visableFlags(-1), _enableItemFlags(-1) {
    }

    void emitUpdateFormat() {
        emit updatedFormat();
    }
    inline void setChecked(bool checked) {
        Qt::CheckState check_state = checked ? Qt::Checked : Qt::Unchecked;
        setCheckState(check_state);
        _format.enabled = checked;
    }

    inline bool isChecked() const {
        return checkState() != Qt::Unchecked;
        //return _format.enabled;
    }

    inline void setForeground(const QBrush &brush) {
        _format.foreground = brush.color();
        QListWidgetItem::setForeground(brush);
    }

    inline void setBackground(const QBrush &brush) {
        _format.background = brush.color();
        QListWidgetItem::setBackground(brush);
    }

    inline void setTextFormat(const TextEditor::TextFormat &format) {
        setText(format.name);
        setChecked(format.enabled);
        setForeground(format.foreground.alpha() ? format.foreground : "#000000");
        setBackground(format.background);
        setTextFormatBold(format.option.bold);
        setTextFormatItalic(format.option.italic);
        setTextFormatUnderline(format.option.underline);
        _format = format;
    }
    inline TextEditor::TextFormat textFormat() {
        //TextEditor::TextFormat format;
        _format.name = text();
        _format.enabled = checkState() != Qt::Unchecked;
        //_format.foreground = foreground().color();
        //_format.background = background().color();
        _format.option.bold = font().bold();
        _format.option.italic = font().italic();
        _format.option.underline = font().underline();

        return _format;
    }
    inline void setTextFormatBold(bool enabled) {
        QFont f = font();
        f.setBold(enabled);
        setFont(f);
    }
    inline void setTextFormatItalic(bool enabled) {
        QFont f = font();
        f.setItalic(enabled);
        setFont(f);
    }
    inline void setTextFormatUnderline(bool enabled) {
        QFont f = font();
        f.setUnderline(enabled);
        setFont(f);
    }

    inline void setVisableFlags(EditorFormatItem::VisableItemFlags flags)
    {
        _visableFlags = flags;
    }

    inline EditorFormatItem::VisableItemFlags visableFlags() const
    {
        return _visableFlags;
    }

    inline void setEnableItemFlags(EditorFormatItem::EnableItemFlags flags)
    {
        _enableItemFlags = flags;
    }

    inline EditorFormatItem::EnableItemFlags enableItemFlags() const
    {
        return _enableItemFlags;
    }

private:
    TextEditor::TextFormat _format;
    EditorFormatItem::VisableItemFlags _visableFlags;
    EditorFormatItem::EnableItemFlags _enableItemFlags;

signals:
    void updatedFormat();
};


Q_DECLARE_OPERATORS_FOR_FLAGS(EditorFormatItem::VisableItemFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(EditorFormatItem::EnableItemFlags)


#endif // CONFIGEDITORPAGE_H
