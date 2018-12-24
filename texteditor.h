#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QCoreApplication>
#include <QPlainTextEdit>

class Highlighter;

class TextEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    typedef enum tagNewLineCode {
        NewLineCodeCRLF,
        NewLineCodeLF,
        NewLineCodeCR,
        NewLineCodeUnknown
    } NewLineCode;

    typedef struct tagOpenedData {
        QByteArray textCodecForName;
        int column;
        int row;
    } OpenedData;

    typedef struct tagFormatOption {
        bool bold;                  // 太字
        bool italic;                // 斜体
        bool underline;             // 下線
    } FormatOption;

    typedef struct tagTextFormat {
        tagTextFormat()
            : name(""), enabled(false), foreground("#000000"), background("#ffffff")
        {
            option.bold = false;
            option.italic = false;
            option.underline = false;
        }
        tagTextFormat(QString name, bool enabled, QColor foreground, QColor background)
            : name(name), enabled(enabled), foreground(foreground), background(background)
        {
            option.bold = false;
            option.italic = false;
            option.underline = false;
        }
        QString name;
        bool enabled;
        QColor foreground;
        QColor background;
        FormatOption option;
    } TextFormat;

    typedef struct tagKeywordOption {
        bool caseSensitive;                 // 大文字小文字区別
        bool wholeWords;                    // 単語単位
        bool regularExpression;             // 正規表現
    } KeywordOption;

    typedef struct tagKeywordData {
        QString text;
        KeywordOption option;
        int highlightIndex;
    } KeywordData;

    typedef struct tagBlocKwordData {
        QString beginText;
        QString endText;
        KeywordOption option;
        int highlightIndex;
    } BlockwordData;

    typedef struct tagConfigType {
        QString key;
        QString name;
        QString suffixes;
    } ConfigType;

    typedef struct tagConfig {
        ConfigType type;                            // キー
        QString fontFamily;                         // フォント名
        qreal fontPointSizeF;                       // フォントサイズ
        qreal zoom;                                 //
        int tabStopDigits;                          // タブ幅
        QString tabChar;                            // タブ文字
        QString halfSpaceChar;                      // 半角文字
        QString fullSpaceChar;                      // 全角文字
        QString endOfLineChar;                      // 改行文字
        QString endOfFileChar;                      // EOF文字
        int cursorWidth;
        QByteArray defTextCodecName;                // デフォルト文字コード
        bool useUtf8Bom;                            // UTF-8 BOM付き
        NewLineCode defNewLineCode;                 // デフォルト改行コード
        TextFormat basicFormat;                     // 基本色
        TextFormat stripeFormat;                    // ストライプ
        TextFormat lineNumberFormat;                // 行番号
        TextFormat lineNumberCurrentFormat;         // 現在の行
        TextFormat columnNumberFormat;              // 列番号
        TextFormat columnNumberCurrentFormat;       // 現在の列
        QList<QVariant> highlightFormats;           // 強調色
        QList<QVariant> findFormats;                // 検索文字列
        TextFormat halfSpaceVisibleFormat;          // 半角空白色
        TextFormat fullSpaceVisibleFormat;          // 全角空白色
        TextFormat tabVisibleFormat;                // タブ色
        TextFormat endOfLineFormat;                 // 改行コード色
        TextFormat endOfFileFormat;                 // EOF色
        TextFormat currentLineFormat;
        TextFormat currentColumnFormat;
        TextFormat selectionFormat;                 // 選択色
        QList<QVariant> keywords;                   // キーワード
        QList<QVariant> blockwords;                 // 複数行キーワード
    } Config;

public:
    explicit TextEditor(QWidget *parent = 0);
    void newFile();
    bool openFile(const QString &fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);
    bool maybeSave();
    QString userFriendlyCurrentFile();
    void setCurrentFile(const QString &fileName);
    QString currentFile() const { return filePath; }
    QString strippedName(const QString &fullFileName);
    bool isUntitled() const { return untitled; }
    bool loadFile(const QString &fileName, QTextCodec *textCodec);
    OpenedData openedData(QString filePath);
    void setCursorForLineNumber(int line);
    int cursorForLineNumber() const;
    int cursorForColumnNumber() const;
    void setTextCodecForName(QString codec);
    const QString textCodecForName();
    void setNewLineCode(NewLineCode indent);
    NewLineCode newLineCode();
    const QString newLineCodeName();
    const QByteArray newLineCodeText();
    QVector<int> findwordMatchLines(int index) const;
    int findwordsCount(int index) const;
    void updateConfig(const int &index);
    void updateConfig(const QString &key);
    void updateConfig();
    void setFontFamily(const QString &family);
    void setFontPointSizeF(double sizeF);
    void setZoom(double zoom);
    void setLineNumberVisible(bool visible);
    void setColumnNumberVisible(bool visible);
    void setTabVisible(bool visible);
    void setTabStopDigits(int digits);
    void setTabChar(const QString &text);
    void setHalfSpaceVisible(bool visible);
    void setHalfSpaceChar(const QString &text);
    void setFullSpaceVisible(bool visible);
    void setFullSpaceChar(const QString &text);
    void setEndOfLineVisible(bool visible);
    void setEndOfLineChar(const QString &text);
    void setEndOfFileVisible(bool visible);
    void setEndOfFileChar(const QString &text);
    void setBasicFormat(const TextFormat &format);
    void setStripeFormat(const TextFormat &format);
    void setLineNumberFormat(const TextFormat &format);
    void setLineNumberCurrentFormat(const TextFormat &format);
    void setColumnNumberFormat(const TextFormat &format);
    void setColumnNumberCurrentFormat(const TextFormat &format);
    void setHighlightFormat(const int index, const TextFormat &format);
    void setHighlightFormats(const QList<QVariant> &formats);
    void setFindFormat(const int index, const TextFormat &format);
    void setFindFormats(const QList<QVariant> &formats);
    void setHalfSpaceVisibleFormat(const TextFormat &format);
    void setFullSpaceVisibleFormat(const TextFormat &format);
    void setTabVisibleFormat(const TextFormat &format);
    void setEndOfLineFormat(const TextFormat &format);
    void setEndOfFileFormat(const TextFormat &format);
    void setCurrentLineFormat(const TextFormat &format);
    void setCurrentColumnFormat(const TextFormat &format);
    void setSelectionFormat(const TextFormat &format);
    void setKeywords(const QList<QVariant> &keywords);
    void setBlockwords(const QList<QVariant> &blockwords);
    void setFindword(int index, const TextEditor::KeywordData &data);
    void setFindwords(const TextEditor::KeywordData finds[]);

public slots:
    void documentWasModified();
    void updateExtraArea();
    void changedCursorPosition();
    void highlightCurrentLine();
    void highlightCurrentColumn();
    void updateLineNumberArea(const QRect &, int);
    void updateColumnNumberArea(const QRect &, int);
    void updateArea(const QRect &rect, int);

protected:
    void closeEvent(QCloseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);
    void scrollContentsBy(int dx, int dy);
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *event);

public:
    /* 行番号 */
    int lineNumberAreaWidth();
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void lineNumberAreaMouseEvent(QMouseEvent *event);
    /* 列番号 */
    int columnNumberAreaHeight();
    void columnNumberAreaPaintEvent(QPaintEvent *event);
    void columnNumberAreaMouseEvent(QMouseEvent *event);

public:
    static QString find(const int &index);
    static QString find(const QString &filePath);
    Config configs() const;
    static Config configs(const int &index);
    static Config configs(const QString &key);

signals:
    void untitledChanged(bool);
    void mouseClickRequest(QMouseEvent *);
    void mouseDoubleClickRequest(QMouseEvent *);


private:
    Config config;
    TextEditor::KeywordData findwords[10];
    QString filePath;
    bool readOnlyMode;
    bool keyControl;
    bool untitled;
    QTextCodec *textCodec;
    QRect oldCursorRect;
    QWidget *lineNumberArea;
    QWidget *columnNumberArea;
    Highlighter *highlighter;
    NewLineCode new_line_code;
};

Q_DECLARE_METATYPE(TextEditor::FormatOption)
QDataStream &operator <<(QDataStream &out, const TextEditor::FormatOption &option);
QDataStream &operator >>(QDataStream &in, TextEditor::FormatOption &option);

Q_DECLARE_METATYPE(TextEditor::TextFormat)
QDataStream &operator <<(QDataStream &out, const TextEditor::TextFormat &text_format);
QDataStream &operator >>(QDataStream &in, TextEditor::TextFormat &text_format);

Q_DECLARE_METATYPE(TextEditor::KeywordOption)
QDataStream &operator <<(QDataStream &out, const TextEditor::KeywordOption &option);
QDataStream &operator >>(QDataStream &in, TextEditor::KeywordOption &option);

Q_DECLARE_METATYPE(TextEditor::KeywordData)
QDataStream &operator >>(QDataStream &in, TextEditor::KeywordData &keyword_data);
QDataStream &operator <<(QDataStream &out, const TextEditor::KeywordData &keyword_data);

Q_DECLARE_METATYPE(TextEditor::BlockwordData)
QDataStream &operator >>(QDataStream &in, TextEditor::BlockwordData &keyword_data);
QDataStream &operator <<(QDataStream &out, const TextEditor::BlockwordData &keyword_data);

Q_DECLARE_METATYPE(TextEditor::NewLineCode)
QDataStream &operator <<(QDataStream &out, const TextEditor::NewLineCode &new_line_code);
QDataStream &operator >>(QDataStream &in, TextEditor::NewLineCode &new_line_code);

Q_DECLARE_METATYPE(TextEditor::ConfigType)
QDataStream &operator <<(QDataStream &out, const TextEditor::ConfigType &config_type);
QDataStream &operator >>(QDataStream &in, TextEditor::ConfigType &config_type);


#endif // TEXTEDITOR_H
