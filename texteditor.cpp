#include <QtGui>
#include "texteditor.h"
#include "highlighter.h"
#include <QFile>
#include <QTextStream>


class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextEditor *editor) : QWidget(editor) {
        textEditor = editor;
    }

    QSize sizeHint() const {
        return QSize(textEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        textEditor->lineNumberAreaPaintEvent(event);
    }
    void mousePressEvent(QMouseEvent *event){
        textEditor->lineNumberAreaMouseEvent(event);
    }
    void mouseMoveEvent(QMouseEvent *event){
        textEditor->lineNumberAreaMouseEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent *event){
        textEditor->lineNumberAreaMouseEvent(event);
    }
    void wheelEvent(QWheelEvent *event) {
        QCoreApplication::sendEvent(textEditor->viewport(), event);
    }

private:
    TextEditor *textEditor;
};

class ColumnNumberArea : public QWidget
{
public:
    ColumnNumberArea(TextEditor *editor) : QWidget(editor) {
        textEditor = editor;
    }

    QSize sizeHint() const {
        return QSize(0, textEditor->columnNumberAreaHeight());
    }

protected:
    void paintEvent(QPaintEvent *event) {
        textEditor->columnNumberAreaPaintEvent(event);
    }
    void mousePressEvent(QMouseEvent *event){
        textEditor->columnNumberAreaMouseEvent(event);
    }
    void mouseMoveEvent(QMouseEvent *event){
        textEditor->columnNumberAreaMouseEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent *event){
        textEditor->columnNumberAreaMouseEvent(event);
    }

private:
    TextEditor *textEditor;
};

TextEditor::TextEditor(QWidget *parent) :
    QPlainTextEdit(parent)
{
    columnNumberArea = new ColumnNumberArea(this);
    lineNumberArea = new LineNumberArea(this);
    highlighter = new Highlighter(document());

    untitled = true;
    keyControl = false;
    new_line_code = NewLineCodeCRLF;
    textCodec = NULL;
    filePath = "";
    readOnlyMode = false;

    setLineWrapMode(QPlainTextEdit::NoWrap);
    setAttribute(Qt::WA_DeleteOnClose);
    setCenterOnScroll(true);
    setReadOnly(false);
    updateConfig(0);
    highlightCurrentLine();
    highlightCurrentColumn();
    changedCursorPosition();
    updateExtraArea();

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateExtraArea()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentColumn()));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateColumnNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(changedCursorPosition()));
    connect(this, SIGNAL(textChanged()), this, SLOT(changedCursorPosition()));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateArea(QRect,int)));
    connect(document(), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));
}

void TextEditor::newFile()
{
    static int sequenceNumber = 1;

    filePath = tr("document%1.txt").arg(sequenceNumber++);

    document()->setModified(false);
    setWindowModified(false);
    setWindowTitle(filePath + "[*]");
}

bool TextEditor::openFile(const QString &filePath)
{
    QFile file(filePath);

    /* ファイル存否確認 */
    if (!file.exists()) {
        QMessageBox::warning(this, tr("ファイル存否確認"),
                             tr("指定のファイルは存在しません: %1")
                             .arg(filePath));
        return false;
    }

    /* ファイル一部読込み(ファイル解析用) */
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("ファイル読込み"),
                             tr("ファイルを読み込めませんでした: %1\n%2.")
                             .arg(filePath)
                             .arg(file.errorString()));
        return false;
    }
    QString analysis_text = QTextStream(&file).read(1000);
    file.close();

    /* 改行コード判定 */
    if (analysis_text.indexOf("\r\n")) {
        new_line_code = NewLineCodeCRLF;
    } else if (analysis_text.indexOf("\n")) {
        new_line_code = NewLineCodeLF;
    } else if (analysis_text.indexOf("\r")) {
        new_line_code = NewLineCodeCR;
    } else {
        new_line_code = config.defNewLineCode;
    }

    /* 文字コード判定 */
    textCodec = QTextCodec::codecForUtfText(analysis_text.toLatin1() ,QTextCodec::codecForName(config.defTextCodecName));

    /* ファイル履歴検索 */
    TextEditor::OpenedData opened_data = openedData(filePath); // カーソルの位置及び文字コードを取得

    /* 前回ファイルを開いた時の文字コードと新しく検出した文字コードが一致するか判定する */
    if (textCodec->name() != opened_data.textCodecForName) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::information(this,
                                       tr("エンコード"),
                                       tr("前回ファイルを開いた時の文字コードと一致しません。\n"
                                          "新しく検出した文字コードで開きますか？\n"
                                          "[Yes]: %1\n"
                                          "[No] : %2")
                                       .arg(QString(textCodec->name()))
                                       .arg(QString(opened_data.textCodecForName)),
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No) {
            textCodec = QTextCodec::codecForName(config.defTextCodecName);
        }
    }

    /* ファイルの読込み */
    if (!loadFile(filePath, textCodec)) {
        return false;
    }

    /* カーソルの位置復元 */
    setCursorForLineNumber(opened_data.row);

    /* 拡張子からキーを取得 */
    QString key = TextEditor::find(filePath);

    /* 拡張子に対応する設定値の読込み */
    updateConfig(key);

    /*  */
    setCurrentFile(filePath);

    return true;
}

bool TextEditor::save()
{
    if (!QFile::exists(filePath)) {
        return saveAs();
    } else {
        return saveFile(filePath);
    }
}

bool TextEditor::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("名前を付けて保存"),
                                                    filePath,
                                                    tr("すべてのファイル (*.*)"));
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

bool TextEditor::saveFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "",
                             tr("ファイルの書き込みに失敗しました。 %1:\n%2.")
                             .arg(filePath)
                             .arg(file.errorString()));
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QTextStream out(&file);
    out.setCodec(textCodec);

    out << toPlainText();
    /*
    const QByteArray newLineCode = newLineCodeText();
    QTextCursor cur = textCursor();
    cur.movePosition(QTextCursor::Start);
    while (!cur.atEnd()) {
        out << qPrintable(cur.block().text()) << newLineCode;
        cur.movePosition(QTextCursor::NextBlock);
    }
    */

    QApplication::restoreOverrideCursor();

    setCurrentFile(filePath);
    return true;
}

bool TextEditor::maybeSave()
{
    if (document()->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, "", tr("'%1' は変更されています。\n"
                                                "変更を保存しますか?").arg(userFriendlyCurrentFile()),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        switch (ret) {
        case QMessageBox::Save:
            return save();
        case QMessageBox::Cancel:
            return false;
        default:
            break;
        }
    }
    return true;
}

QString TextEditor::userFriendlyCurrentFile()
{
    return strippedName(filePath);
}

void TextEditor::setCurrentFile(const QString &fileName)
{
    filePath = QFileInfo(fileName).canonicalFilePath();
    untitled = false;
    document()->setModified(false);
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile() + "[*]");

    QFileSystemModel model;
    setWindowIcon(model.fileIcon(model.index(fileName)));
}

QString TextEditor::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

bool TextEditor::loadFile(const QString &filePath, QTextCodec *textCodec)
{
    QFile file(filePath);

    /* ファイル存否確認 */
    if (!file.exists()) {
        QMessageBox::warning(this, tr("ファイル存否確認"),
                             tr("指定のファイルは存在しません: %1")
                             .arg(filePath));
        return false;
    }

    /* ファイルの読込み */
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("ファイル読込み"),
                             tr("ファイルを読み込めませんでした: %1\n%2.")
                             .arg(filePath)
                             .arg(file.errorString()));
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    clear();
    QTextStream in(&file);
    in.setCodec(textCodec);
    setPlainText(in.readAll());
    file.close();
    QApplication::restoreOverrideCursor();

    /* ファイル書き込み権限チェック */
    if (!QFileInfo(filePath).isWritable()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("書込み禁止"),
                                   tr("指定のファイルは編集することができません。\n"
                                      "読み取り専用モードで開きますか？"),
                                   QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            setReadOnly(true);
        } else {
            return false;
        }
    }

    return true;
}

TextEditor::OpenedData TextEditor::openedData(QString /*filePath*/)
{
    TextEditor::OpenedData opened_data = { "System", 0, 0 };
    return opened_data;
}

void TextEditor::setCursorForLineNumber(int line)
{
    //QTextBlock anchorBlock = document()->findBlockByNumber(line);
    QTextCursor cursor(document());
    cursor.movePosition(QTextCursor::Start);
    for (int i = 1; i < line; ++i) {
        cursor.movePosition(QTextCursor::Down);
    }
    setTextCursor(cursor);
}

int TextEditor::cursorForLineNumber() const
{
    return textCursor().blockNumber() + 1;
}

int TextEditor::cursorForColumnNumber() const
{
    return cursorRect().x() / fontMetrics().width('9');
}

void TextEditor::setTextCodecForName(QString codec)
{
    textCodec = QTextCodec::codecForName(codec.toLatin1());
}

const QString TextEditor::textCodecForName()
{
    if (textCodec)
        return textCodec->name();
    else
        return "Unknown";
}

void TextEditor::setNewLineCode(TextEditor::NewLineCode indent)
{
    new_line_code = indent;
}

TextEditor::NewLineCode TextEditor::newLineCode()
{
    return new_line_code;
}

const QString TextEditor::newLineCodeName()
{
    const char *indention_names[] = {
        "CR+LF",
        "LF",
        "CR"
    };

    return indention_names[new_line_code];
}

const QByteArray TextEditor::newLineCodeText()
{
    switch (new_line_code) {
    case NewLineCodeCRLF:
        return "\r\n";
    case NewLineCodeLF:
        return "\n";
    case NewLineCodeCR:
        return "\r";
    default:
        return "\r\n";
    }
    /*
    switch (new_line_code) {
    case NewLineCodeCRLF:
        return "\2029\2028";
    case NewLineCodeLF:
        return "\2028";
    case NewLineCodeCR:
        return "\2029";
    default:
        return "\2028";
    }
    */
}

QVector<int> TextEditor::findwordMatchLines(int index) const
{
    return highlighter->findwordMatchLines(index);
}

int TextEditor::findwordsCount(int index) const
{
    return highlighter->findwordCount(index);
}
void TextEditor::updateConfig(const int &index)
{
    // 設定値取得
    config = TextEditor::configs(index);
    updateConfig();
}

void TextEditor::updateConfig(const QString &key)
{
    // 設定値取得
    config = TextEditor::configs(key);
    updateConfig();
}

void TextEditor::updateConfig()
{
    setFontFamily(config.fontFamily);
    setFontPointSizeF(config.fontPointSizeF);
    setZoom(config.zoom);
    setLineNumberVisible(config.lineNumberFormat.enabled);
    setTabStopDigits(config.tabStopDigits);
    setCursorWidth(config.cursorWidth);
    setBasicFormat(config.basicFormat);
    setStripeFormat(config.stripeFormat);
    setLineNumberFormat(config.lineNumberFormat);
    setLineNumberCurrentFormat(config.lineNumberCurrentFormat);
    setColumnNumberFormat(config.columnNumberFormat);
    setColumnNumberCurrentFormat(config.columnNumberCurrentFormat);
    setHighlightFormats(config.highlightFormats);
    setFindFormats(config.findFormats);
    setHalfSpaceVisibleFormat(config.halfSpaceVisibleFormat);
    setFullSpaceVisibleFormat(config.fullSpaceVisibleFormat);
    setTabVisibleFormat(config.tabVisibleFormat);
    setEndOfLineFormat(config.endOfLineFormat);
    setEndOfFileFormat(config.endOfFileFormat);
    setCurrentLineFormat(config.currentLineFormat);
    setCurrentColumnFormat(config.currentColumnFormat);
    setSelectionFormat(config.selectionFormat);
    setKeywords(config.keywords);
    setBlockwords(config.blockwords);
}

void TextEditor::setFontFamily(const QString &family)
{
    QFont f = font();
    f.setFamily(family);
    setFont(f);
    setTabStopDigits(config.tabStopDigits);
}

void TextEditor::setFontPointSizeF(double sizeF)
{
    config.fontPointSizeF = sizeF;
    QFont f = font();
    f.setPointSizeF(sizeF * config.zoom);
    setFont(f);
    setTabStopDigits(config.tabStopDigits);
}

void TextEditor::setZoom(double zoom)
{
    config.zoom = zoom;
    setFontPointSizeF(config.fontPointSizeF);
}

void TextEditor::setLineNumberVisible(bool visible)
{
    config.lineNumberFormat.enabled = visible;
    setLineNumberFormat(config.lineNumberFormat);
}

void TextEditor::setColumnNumberVisible(bool visible)
{
    config.columnNumberFormat.enabled = visible;
    setColumnNumberFormat(config.columnNumberFormat);

}

void TextEditor::setTabVisible(bool visible)
{
    config.tabVisibleFormat.enabled = visible;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setTabStopDigits(int digits)
{
    config.tabStopDigits = digits;
    setTabStopWidth(fontMetrics().width('9') * digits);
}

void TextEditor::setTabChar(const QString &text)
{
    config.tabChar = text;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setHalfSpaceVisible(bool visible)
{
    config.halfSpaceVisibleFormat.enabled = visible;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setHalfSpaceChar(const QString &text)
{
    config.halfSpaceChar = text;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setFullSpaceVisible(bool visible)
{
    config.fullSpaceVisibleFormat.enabled = visible;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setFullSpaceChar(const QString &text)
{
    config.fullSpaceChar = text;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setEndOfLineVisible(bool visible)
{
    config.endOfLineFormat.enabled = visible;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setEndOfLineChar(const QString &text)
{
    config.endOfLineChar = text;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setEndOfFileVisible(bool visible)
{
    config.endOfFileFormat.enabled = visible;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setEndOfFileChar(const QString &text)
{
    config.endOfFileChar = text;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setBasicFormat(const TextFormat &format)
{
    config.basicFormat = format;
    QPalette p = palette();

    p.setBrush(QPalette::Text, format.foreground);
    p.setBrush(QPalette::Base, format.background);

    setPalette(p);
}

void TextEditor::setStripeFormat(const TextFormat &format)
{
    config.stripeFormat = format;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setLineNumberFormat(const TextFormat &format)
{
    config.lineNumberFormat = format;
    columnNumberArea->repaint();
    lineNumberArea->repaint();
    updateExtraArea();
}

void TextEditor::setLineNumberCurrentFormat(const TextFormat &format)
{
    config.lineNumberCurrentFormat = format;
    lineNumberArea->repaint();
}

void TextEditor::setColumnNumberFormat(const TextFormat &format)
{
    config.columnNumberFormat = format;
    columnNumberArea->repaint();
    lineNumberArea->repaint();
    updateExtraArea();
}

void TextEditor::setColumnNumberCurrentFormat(const TextFormat &format)
{
    config.columnNumberCurrentFormat = format;
    columnNumberArea->repaint();
}

void TextEditor::setHighlightFormat(const int index, const TextFormat &format)
{
    config.highlightFormats[index] = QVariant::fromValue(format);
    highlighter->updateHighlightFormats(config.highlightFormats);
}

void TextEditor::setHighlightFormats(const QList<QVariant> &formats)
{
    config.highlightFormats = formats;
    highlighter->updateHighlightFormats(config.highlightFormats);
}

void TextEditor::setFindFormat(const int index, const TextFormat &format)
{
    config.findFormats[index] = QVariant::fromValue(format);
    highlighter->updateFindFormats(config.findFormats);
}

void TextEditor::setFindFormats(const QList<QVariant> &formats)
{
    config.findFormats = formats;
    highlighter->updateFindFormats(config.findFormats);
}

void TextEditor::setHalfSpaceVisibleFormat(const TextFormat &format)
{
    config.halfSpaceVisibleFormat = format;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setFullSpaceVisibleFormat(const TextFormat &format)
{
    config.fullSpaceVisibleFormat = format;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setTabVisibleFormat(const TextFormat &format)
{
    config.tabVisibleFormat = format;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setEndOfLineFormat(const TextFormat &format)
{
    config.endOfLineFormat = format;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setEndOfFileFormat(const TextFormat &format)
{
    config.endOfFileFormat = format;
    setUpdatesEnabled(false);
    setUpdatesEnabled(true);
}

void TextEditor::setCurrentLineFormat(const TextFormat &format)
{
    config.currentLineFormat = format;
    QRect cursor_rect = oldCursorRect;
    cursor_rect.setLeft(0);
    cursor_rect.setWidth(width());
    viewport()->update(cursor_rect);
}

void TextEditor::setCurrentColumnFormat(const TextFormat &format)
{
    config.currentColumnFormat = format;
    QRect cursor_rect = oldCursorRect;
    cursor_rect.setTop(0);
    cursor_rect.setHeight(height());
    viewport()->update(cursor_rect);
}

void TextEditor::setSelectionFormat(const TextFormat &format)
{
    config.selectionFormat = format;
    QPalette p = palette();

    p.setBrush(QPalette::HighlightedText, format.foreground);
    p.setBrush(QPalette::Highlight, format.background);

    setPalette(p);
}

void TextEditor::setKeywords(const QList<QVariant> &keywords)
{
    highlighter->updateKeywords(keywords);
}

void TextEditor::setBlockwords(const QList<QVariant> &blockwords)
{
    highlighter->updateBlockwords(blockwords);
}

void TextEditor::setFindword(int index, const TextEditor::KeywordData &data)
{
    findwords[index] = data;
    highlighter->updateFindwords(findwords);
    //lastFindWoard = data;
}

void TextEditor::setFindwords(const TextEditor::KeywordData finds[])
{
    memcpy(findwords, finds, sizeof(findwords));
    highlighter->updateFindwords(findwords);
}

void TextEditor::documentWasModified()
{
    setWindowModified(document()->isModified());
}

void TextEditor::updateExtraArea()
{
    setViewportMargins(lineNumberAreaWidth(), columnNumberAreaHeight(), 0, 0);
}

void TextEditor::changedCursorPosition()
{
    highlightCurrentLine();
    highlightCurrentColumn();
    oldCursorRect = cursorRect();
}

void TextEditor::highlightCurrentLine()
{
    if (oldCursorRect.bottom() != cursorRect().bottom())
    {
        if (config.currentLineFormat.enabled) {
            QRect cursor_rect = cursorRect();
            cursor_rect.setLeft(0);
            cursor_rect.setWidth(width());
            viewport()->update(cursor_rect);

            cursor_rect = oldCursorRect;
            cursor_rect.setLeft(0);
            cursor_rect.setWidth(width());
            viewport()->update(cursor_rect);
        }
    }
}

void TextEditor::highlightCurrentColumn()
{
    if (oldCursorRect.left() != cursorRect().left())
    {
        if (config.columnNumberCurrentFormat.enabled) {
            const int &fontWidth = fontMetrics().width('9');
            columnNumberArea->update(oldCursorRect.left() + lineNumberAreaWidth(), 0, fontWidth, columnNumberAreaHeight());
            columnNumberArea->update(cursorRect().left() + lineNumberAreaWidth(), 0, fontWidth, columnNumberAreaHeight());
        }

        if (config.currentColumnFormat.enabled) {
            QRect cursor_rect = cursorRect();
            cursor_rect.setTop(0);
            cursor_rect.setHeight(height());
            viewport()->update(cursor_rect);

            cursor_rect = oldCursorRect;
            cursor_rect.setTop(0);
            cursor_rect.setHeight(height());
            viewport()->update(cursor_rect);
        }
    }
}

void TextEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y() + columnNumberAreaHeight() , lineNumberAreaWidth(), rect.height() + 5);

    if (rect.contains(viewport()->rect()))
        updateExtraArea();
}

void TextEditor::updateColumnNumberArea(const QRect &rect, int)
{
    columnNumberArea->update(rect.x(), 0, rect.width(), columnNumberAreaHeight());

    if (rect.contains(viewport()->rect()))
        updateExtraArea();
}

void TextEditor::updateArea(const QRect &rect, int)
{
    viewport()->update(rect);

    if (rect.contains(viewport()->rect()))
        changedCursorPosition();
}

void TextEditor::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void TextEditor::mousePressEvent(QMouseEvent *event)
{
    QPlainTextEdit::mousePressEvent(event);
}

void TextEditor::mouseReleaseEvent(QMouseEvent *event)
{
    QPlainTextEdit::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton) {
        emit mouseClickRequest(event);
    }
}

void TextEditor::mouseMoveEvent(QMouseEvent *event)
{
    QPlainTextEdit::mouseMoveEvent(event);
    if (event->modifiers() & Qt::AltModifier) {
    }
}

void TextEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
    QPlainTextEdit::mouseDoubleClickEvent(event);

    if (event->button() == Qt::LeftButton) {
        emit mouseDoubleClickRequest(event);
    }
}

void TextEditor::keyPressEvent(QKeyEvent *event)
{
    if (event == QKeySequence::InsertLineSeparator) {
        textCursor().insertBlock();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Tab) {
        if (textCursor().hasSelection()) {
            QTextCursor cursor(textCursor());
            cursor.beginEditBlock();
            cursor.setPosition(cursor.selectionStart());
            while (cursor.position() < textCursor().selectionEnd()) {
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.insertText("\t");
                cursor.movePosition(QTextCursor::Down);
            }
            cursor.endEditBlock();
            event->accept();
            return;
        }
    } else if (event->key() == Qt::Key_Backtab) {
        if (textCursor().hasSelection()) {
            QTextCursor cursor(textCursor());
            cursor.beginEditBlock();
            cursor.setPosition(cursor.selectionStart());
            while (cursor.position() < textCursor().selectionEnd()) {
                QString text = cursor.block().text();
                int index = QRegExp("^\\t").indexIn(text);
                if (index >= 0) {
                    cursor.movePosition(QTextCursor::StartOfBlock);
                    cursor.deleteChar();
                } else {
                    QRegExp re(QString("^ {1,%1}").arg(config.tabStopDigits));
                    int index = re.indexIn(text);
                    if (index >= 0) {
                        cursor.movePosition(QTextCursor::StartOfBlock);
                        for (int i = 0; i < re.matchedLength(); ++i)
                            cursor.deleteChar();
                    }
                }
                cursor.movePosition(QTextCursor::Down);
            }
            cursor.endEditBlock();
            event->accept();
            return;
        }
    }

    QPlainTextEdit::keyPressEvent(event);
}

void TextEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    const QRect &cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    columnNumberArea->setGeometry(QRect(cr.left(), cr.top(), cr.width(), columnNumberAreaHeight()));
}

void TextEditor::scrollContentsBy(int dx, int dy)
{
    lineNumberArea->scroll(0, dy);
    columnNumberArea->scroll(dx, 0);
    QPlainTextEdit::scrollContentsBy(dx, dy);
}

void TextEditor::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    QPointF offset(contentOffset());

    QTextBlock block = firstVisibleBlock();
    int top = (int) blockBoundingGeometry(block).translated(offset).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    QTextCursor eof_cur = textCursor();
    eof_cur.movePosition(QTextCursor::End);
    const int eof = eof_cur.position();

    painter.setBrushOrigin(offset);
    painter.setClipRect(event->rect());
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            // ストライプ
            if (config.stripeFormat.enabled && block.blockNumber()%2 == 0) {
                painter.fillRect(blockBoundingRect(block).translated(0, top), config.stripeFormat.background);
            }
            // 現在の行
            if (false && config.currentLineFormat.enabled) {
                const int selStart = textCursor().selectionStart();
                const int selEnd = textCursor().selectionEnd();
                bool selected = ((selStart < block.position() + block.length() && selEnd >= block.position() + 1)
                                 || (selStart == selEnd && selStart == block.position()));
                if (selected)
                    painter.fillRect(blockBoundingRect(block).translated(0, top), config.currentLineFormat.background);
            }
            // 改行
            QTextCursor eol_cur(block);
            eol_cur.movePosition(QTextCursor::EndOfBlock);
            if (config.endOfLineFormat.enabled && eol_cur.position() != eof) {
                QRect r = cursorRect(eol_cur);
                QRectF bounding = painter.boundingRect(QRectF(r), config.endOfLineChar);
                painter.fillRect(bounding, config.endOfLineFormat.background);
                painter.setPen(config.endOfLineFormat.foreground);
                painter.drawText(bounding, config.endOfLineChar);
            }

            // 特殊文字
            QTextCursor search_char_cur(block);
            search_char_cur.movePosition(QTextCursor::StartOfBlock);
            do {
                QChar c = document()->characterAt(search_char_cur.position());
                // 半角空白
                if (config.halfSpaceVisibleFormat.enabled && c == ' ') {
                    QRect r = cursorRect(search_char_cur);
                    QRectF bounding = painter.boundingRect(QRectF(r), config.halfSpaceChar);
                    painter.fillRect(bounding, config.halfSpaceVisibleFormat.background);
                    painter.setPen(config.halfSpaceVisibleFormat.foreground);
                    painter.drawText(bounding, config.halfSpaceChar);
                }
                // 全角空白
                if (config.fullSpaceVisibleFormat.enabled && c.unicode() == 12288) {
                    QRect r = cursorRect(search_char_cur);
                    QRectF bounding = painter.boundingRect(QRectF(r), config.fullSpaceChar);
                    painter.fillRect(bounding, config.fullSpaceVisibleFormat.background);
                    painter.setPen(config.fullSpaceVisibleFormat.foreground);
                    painter.drawText(bounding, config.fullSpaceChar);
                }
                // タブ
                if (config.tabVisibleFormat.enabled && c == '\t') {
                    QRect r = cursorRect(search_char_cur);
                    QRectF bounding = painter.boundingRect(QRectF(r), config.tabChar);
                    painter.fillRect(bounding, config.tabVisibleFormat.background);
                    painter.setPen(config.tabVisibleFormat.foreground);
                    painter.drawText(bounding, config.tabChar);
                }
                search_char_cur.movePosition(QTextCursor::NextCharacter);
            } while (!search_char_cur.atBlockEnd());

        }
        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect(block).height();
    }

    // EOF
    if (config.endOfFileFormat.enabled) {
        QRect r = cursorRect(eof_cur);
        QRectF bounding = painter.boundingRect(QRectF(r), config.endOfFileChar);
        painter.fillRect(bounding, config.endOfFileFormat.background);
        painter.setPen(config.endOfFileFormat.foreground);
        painter.drawText(bounding, config.endOfFileChar);
    }

    QPlainTextEdit::paintEvent(event);

    QPainter painter2(viewport());
    if (config.currentLineFormat.enabled) {
        painter2.setPen(config.currentLineFormat.background);
        QRect cr = cursorRect();
        painter2.drawLine(0, cr.bottom(), width(), cr.bottom());
    }

    if (config.currentColumnFormat.enabled) {
        painter2.setPen(config.currentColumnFormat.background);
        QRect cr = cursorRect();
        painter2.drawLine(cr.left(), 0, cr.left(), height());
    }
}

void TextEditor::wheelEvent(QWheelEvent *event)
{
    QCoreApplication::sendEvent(parent(), event);
    if (event->isAccepted()) {
        QPlainTextEdit::wheelEvent(event);
    }
}

int TextEditor::lineNumberAreaWidth()
{
    if (!config.lineNumberFormat.enabled) return 0;

    int digits = 1;
    int max = qMax(10, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * (digits + 4);

    return space;
}

void TextEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.setTransform(painter.combinedTransform().translate(0, columnNumberAreaHeight()));
    QPointF offset(contentOffset());
    painter.fillRect(event->rect(), config.lineNumberFormat.background);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    const int width = lineNumberArea->width() - 15;
    const int height = (int)blockBoundingRect(block).height();
    const int selStart = textCursor().selectionStart();
    const int selEnd = textCursor().selectionEnd();
    int top = (int)blockBoundingGeometry(block).translated(offset).top();
    int bottom = top + height;

    painter.setPen(config.lineNumberFormat.foreground);
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            // 行番号描画
            const QString &number = QString::number(blockNumber + 1);
            bool selected = config.lineNumberCurrentFormat.enabled && ((selStart < block.position() + block.length() && selEnd > block.position())
                             || (selStart == selEnd && selStart == block.position()));
            if(selected) {
                painter.save();
                painter.fillRect(blockBoundingRect(block).translated(0, top), config.lineNumberCurrentFormat.background);
                painter.setPen(config.lineNumberCurrentFormat.foreground);
            }
            painter.drawText(0, top, width, height, Qt::AlignRight, number);
            if (selected)
                painter.restore();
        }

        block = block.next();
        top = bottom;
        bottom = top + height;
        ++blockNumber;
    }
}

void TextEditor::lineNumberAreaMouseEvent(QMouseEvent *event)
{
    static int blockNumber = -1;
    QTextCursor cursor = cursorForPosition(QPoint(0, event->pos().y() - columnNumberAreaHeight()));
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
        if (event->button() == Qt::LeftButton) {
            QTextCursor selection = cursor;
            selection.setVisualNavigation(true);
            blockNumber = selection.blockNumber();
            selection.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            selection.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            setTextCursor(selection);
            emit selectionChanged();
        }
    } else if (blockNumber >= 0) {
        if (event->type() == QEvent::MouseMove) {
            QTextCursor selection = cursor;
            selection.setVisualNavigation(true);
            QTextBlock anchorBlock = document()->findBlockByNumber(blockNumber);
            selection.setPosition(anchorBlock.position());
            if (cursor.blockNumber() < blockNumber) {
                selection.movePosition(QTextCursor::EndOfBlock);
                selection.movePosition(QTextCursor::Right);
            }
            selection.setPosition(cursor.block().position(), QTextCursor::KeepAnchor);
            if (cursor.blockNumber() >= blockNumber) {
                selection.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                selection.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            }
            setTextCursor(selection);
            emit selectionChanged();
        } else {
            blockNumber = -1;
        }
    }
}

int TextEditor::columnNumberAreaHeight()
{
    if (!config.columnNumberFormat.enabled) return 0;

    int space = fontMetrics().height();

    return space;
}

void TextEditor::columnNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(columnNumberArea);
    painter.fillRect(event->rect(), config.columnNumberFormat.background);

    int fontWidth = fontMetrics().width(QLatin1Char('9'));

    painter.setPen(config.columnNumberFormat.foreground);
    const int height = columnNumberAreaHeight();
    const int columnNumberOffset = lineNumberAreaWidth() + contentOffset().x();

    /* 目盛 */
    int left = document()->documentMargin() + columnNumberOffset;
    int right = left + fontWidth;
    while (left <= event->rect().right()) {
        if (right >= event->rect().left()) {
            int columnNumber = (left - columnNumberOffset) / fontWidth;
            if (columnNumber % 10 == 0) {
                painter.drawLine(QPointF(left, height * 0.2f), QPointF(left, height));
            } else if (columnNumber % 5 == 0) {
                painter.drawLine(QPointF(left, height * 0.4f), QPointF(left, height));
            } else {
                painter.drawLine(QPointF(left, height * 0.6f), QPointF(left, height));
            }

        }

        left = right;
        right = left + fontWidth;
    }

    /* 番号 */
    left = document()->documentMargin() + columnNumberOffset;
    right = left + fontWidth;
    while (left <= event->rect().right()) {
        if (right >= event->rect().left()) {
            int columnNumber = (left - columnNumberOffset) / fontWidth;
            if (columnNumber % 10 == 0) {
                QRectF bounding = painter.boundingRect(QRectF(left + fontWidth * 0.5, 0, 0, 0), QString::number(columnNumber));
                painter.fillRect(bounding, config.columnNumberFormat.background);
                painter.drawText(bounding, QString::number(columnNumber));
            }
        }

        left = right;
        right = left + fontWidth;
    }

    if (config.columnNumberCurrentFormat.enabled) {
        const QRect &cr = cursorRect();
        painter.fillRect(cr.left() + lineNumberAreaWidth(), 0, fontWidth, columnNumberArea->height(), config.columnNumberCurrentFormat.background);
    }

    painter.drawLine(QPointF(document()->documentMargin() + columnNumberOffset, height-1), QPointF(width(), height-1));

}

void TextEditor::columnNumberAreaMouseEvent(QMouseEvent *event)
{
    static int columnPosition = -1;
    QTextCursor cursor = cursorForPosition(QPoint(event->pos().x() - lineNumberAreaWidth(), cursorRect().y()));
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
        if (event->button() == Qt::LeftButton) {
            setTextCursor(cursor);
            columnPosition = cursor.position();
        }
    } else if (columnPosition >= 0) {
        if (event->type() == QEvent::MouseMove) {
            QTextCursor selection = cursor;
            selection.setVisualNavigation(true);
            selection.setPosition(columnPosition);
            selection.setPosition(cursor.position(), QTextCursor::KeepAnchor);
            setTextCursor(selection);
            emit selectionChanged();
        } else {
            columnPosition = -1;
        }
    }
}

QString TextEditor::find(const int &index)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");

    /* 拡張子MAPを取得 */
    const QList<QVariant> editorTypes = settings.value("editorTypes").toList();

    /* 拡張子MAPの最大サイズを超えている場合はdefault。 */
    if (editorTypes.size() < index) {
        return "default";
    }

    /* 拡張子MAPのindexに一致するkeyを返す。 */
    for (int i = 0; i < editorTypes.size(); ++i) {
        if (i+1 == index) {
            const ConfigType &config_type = editorTypes.value(i).value<TextEditor::ConfigType>();
            return config_type.key;
        }
    }

    /* 拡張子MAPに一致する値がなかった場合 */
    return "default";
}

QString TextEditor::find(const QString &filePath)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");

    /* ファイルの拡張子を取得 */
    QString complete_suffix = QFileInfo(filePath).completeSuffix();

    /* 拡張子MAPを取得 */
    const QList<QVariant> editorTypes = settings.value("editorTypes").toList();

    /* 拡張子MAPを検索 */
    for (int i = 0; i < editorTypes.size(); ++i) {
        const ConfigType &config_type = editorTypes.value(i).value<TextEditor::ConfigType>();
        QStringList suffix_list = config_type.suffixes.split(QRegExp("[,;]"));
        foreach (const QString &suffix, suffix_list) {
            /* 一致する拡張子が見つかった場合はkeyを返す */
            if (suffix == complete_suffix) {
                return config_type.key;
            }
        }
    }

    /* 一致する拡張子が見つからなかった場合はdefault */
    return "default";
}

TextEditor::Config TextEditor::configs() const
{
    return config;
}

TextEditor::Config TextEditor::configs(const int &index)
{
    // key取得
    QString key = TextEditor::find(index);

    return TextEditor::configs(key);
}

TextEditor::Config TextEditor::configs(const  QString &key)
{
    Config config;

    const char *highlightColor[10] = {
        "#0000ff",
        "#b5a642",
        "#ff0000",
        "#a52a2a",
        "#dc143c",
        "#008b8b",
        "#bdb76b",
        "#aa5500",
        "#008000",
        "#00007f",
    };

    const char *findColor[10] = {
        "#ffff00",
        "#4169e1",
        "#2f4f4f",
        "#ffb6c1",
        "#ff69b4",
        "#4682b4",
        "#daa520",
        "#dc143c",
        "#87cefa",
        "#5f9ea0",
    };

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(key);

    // 設定値取得
    config.type.key = key;
    config.type.name = settings.value("name", "ファイル設定").toString();
    config.type.suffixes = settings.value("suffixes", "").toString();
    config.fontFamily = settings.value("fontFamily", "Consolas").toString();
    config.fontPointSizeF = settings.value("fontPointSizeF", 10).toReal();
    config.zoom = settings.value("zoom", 1).toReal();
    config.tabStopDigits = settings.value("tabStopDigits", 4).toInt();
    config.tabChar = settings.value("tabChar", tr("^")).toString();
    config.halfSpaceChar = settings.value("halfSpaceChar", tr("⋅")).toString();
    config.fullSpaceChar = settings.value("fullSpaceChar", tr("□")).toString();
    config.endOfLineChar = settings.value("endOfLineChar", tr("↓")).toString();
    config.endOfFileChar = settings.value("endOfFileChar", tr("[EOF]")).toString();
    config.cursorWidth = settings.value("cursorWidth", 2).toInt();
    config.defTextCodecName = settings.value("defTextCodecName", "System").toByteArray();
    config.useUtf8Bom = settings.value("useUtf8Bom", true).toBool();
    config.defNewLineCode = settings.value("defNewLineCode", NewLineCodeCRLF).value<NewLineCode>();
    config.basicFormat = settings.value("basicFormat", QVariant::fromValue(TextFormat(tr("基本"), true, "#000000", "#ffffff"))).value<TextFormat>();
    config.stripeFormat = settings.value("stripeFormat", QVariant::fromValue(TextFormat(tr("ストライプ"), false, "transparent", "#f7f7f7"))).value<TextFormat>();
    config.lineNumberFormat = settings.value("lineNumberFormat", QVariant::fromValue(TextFormat(tr("行番号"), true, "#A0A0A0", "#F0F0F0"))).value<TextFormat>();
    config.lineNumberCurrentFormat = settings.value("lineNumberCurrentFormat", QVariant::fromValue(TextFormat(tr("現在の行番号"), true, "#808080", "transparent"))).value<TextFormat>();
    config.columnNumberFormat = settings.value("columnNumberFormat", QVariant::fromValue(TextFormat(tr("列番号"), true, "#808080", "#F0F0F0"))).value<TextFormat>();
    config.columnNumberCurrentFormat = settings.value("columnNumberCurrentFormat", QVariant::fromValue(TextFormat(tr("現在の列番号"), true, "#000000", QColor(0, 0, 0, 100)))).value<TextFormat>();
    QList<QVariant> def_highlight_formats;
    for (int i = 0; i < 10; ++i) {
        def_highlight_formats << QVariant::fromValue(TextFormat(tr("強調文字列%1").arg(i+1), true, highlightColor[i], "transparent"));
    }
    config.highlightFormats = settings.value("highlightFormats", def_highlight_formats).toList();
    QList<QVariant> def_find_formats;
    for (int i = 0; i < 10; ++i) {
        def_find_formats << QVariant::fromValue(TextFormat(tr("検索文字列%1").arg(i+1), true, "#000000", findColor[i]));
    }
    config.findFormats = settings.value("findFormats", def_find_formats).toList();
    config.halfSpaceVisibleFormat = settings.value("halfSpaceVisibleFormat", QVariant::fromValue(TextFormat(tr("半角空白"), false, "#C0C0C0", "transparent"))).value<TextFormat>();
    config.fullSpaceVisibleFormat = settings.value("fullSpaceVisibleFormat", QVariant::fromValue(TextFormat(tr("全角空白"), true, "#C0C0C0", "transparent"))).value<TextFormat>();
    config.tabVisibleFormat = settings.value("tabVisibleFormat", QVariant::fromValue(TextFormat(tr("タブ文字"), true, "#C0C0C0", "transparent"))).value<TextFormat>();
    config.endOfLineFormat = settings.value("endOfLineFormat", QVariant::fromValue(TextFormat(tr("改行コード"), true, "#0080C0", "transparent"))).value<TextFormat>();
    config.endOfFileFormat = settings.value("endOfFileFormat", QVariant::fromValue(TextFormat(tr("EOF"), true, "#0080C0", "transparent"))).value<TextFormat>();
    config.currentLineFormat = settings.value("currentLineFormat", QVariant::fromValue(TextFormat(tr("現在の行"), false, "transparent", "#55aaff"))).value<TextFormat>();
    config.currentColumnFormat = settings.value("currentColumnFormat", QVariant::fromValue(TextFormat(tr("現在の列"), false, "transparent", "#55aaff"))).value<TextFormat>();
    config.selectionFormat = settings.value("selectionFormat", QVariant::fromValue(TextFormat(tr("選択文字列"), true, "#ffffff", "#0064ff"))).value<TextFormat>();
    config.keywords = settings.value("keywords").toList();
    config.blockwords = settings.value("blockwords").toList();

    settings.endGroup();
    return config;
}


QDataStream &operator <<(QDataStream &out, const TextEditor::FormatOption &option)
{
    out << option.bold;
    out << option.italic;
    out << option.underline;
    return out;
}

QDataStream &operator >>(QDataStream &in, TextEditor::FormatOption &option)
{
    in >> option.bold;
    in >> option.italic;
    in >> option.underline;
    return in;
}
QDataStream &operator <<(QDataStream &out, const TextEditor::TextFormat &text_format)
{
    out << text_format.name;
    out << text_format.enabled;
    out << text_format.foreground;
    out << text_format.background;
    out << text_format.option;
    return out;
}

QDataStream &operator >>(QDataStream &in, TextEditor::TextFormat &text_format)
{
    in >> text_format.name;
    in >> text_format.enabled;
    in >> text_format.foreground;
    in >> text_format.background;
    in >> text_format.option;
    return in;
}

QDataStream &operator <<(QDataStream &out, const TextEditor::KeywordOption &option)
{
    out << option.caseSensitive;
    out << option.wholeWords;
    out << option.regularExpression;
    return out;
}

QDataStream &operator >>(QDataStream &in, TextEditor::KeywordOption &option)
{
    in >> option.caseSensitive;
    in >> option.wholeWords;
    in >> option.regularExpression;
    return in;
}

QDataStream &operator <<(QDataStream &out, const TextEditor::KeywordData &keyword_data)
{
    out << keyword_data.text;
    out << keyword_data.option.caseSensitive;
    out << keyword_data.option.wholeWords;
    out << keyword_data.option.regularExpression;
    out << keyword_data.highlightIndex;
    return out;
}

QDataStream &operator >>(QDataStream &in, TextEditor::KeywordData &keyword_data)
{
    in >> keyword_data.text;
    in >> keyword_data.option.caseSensitive;
    in >> keyword_data.option.wholeWords;
    in >> keyword_data.option.regularExpression;
    in >> keyword_data.highlightIndex;
    return in;
}

QDataStream &operator <<(QDataStream &out, const TextEditor::BlockwordData &keyword_data)
{
    out << keyword_data.beginText;
    out << keyword_data.endText;
    out << keyword_data.option.caseSensitive;
    out << keyword_data.option.wholeWords;
    out << keyword_data.option.regularExpression;
    out << keyword_data.highlightIndex;
    return out;
}

QDataStream &operator >>(QDataStream &in, TextEditor::BlockwordData &keyword_data)
{
    in >> keyword_data.beginText;
    in >> keyword_data.endText;
    in >> keyword_data.option.caseSensitive;
    in >> keyword_data.option.wholeWords;
    in >> keyword_data.option.regularExpression;
    in >> keyword_data.highlightIndex;
    return in;
}

QDataStream &operator <<(QDataStream &out, const TextEditor::NewLineCode &new_line_code)
{
    out << static_cast<int>(new_line_code);
    return out;
}

QDataStream &operator >>(QDataStream &in, TextEditor::NewLineCode &new_line_code)
{
    int buf;
    in >> buf;
    new_line_code = static_cast<TextEditor::NewLineCode>(buf);
    return in;
}

QDataStream &operator <<(QDataStream &out, const TextEditor::ConfigType &config_type)
{
    out << config_type.key;
    out << config_type.name;
    out << config_type.suffixes;
    return out;
}

QDataStream &operator >>(QDataStream &in, TextEditor::ConfigType &config_type)
{
    in >> config_type.key;
    in >> config_type.name;
    in >> config_type.suffixes;
    return in;
}
