#include <QtGui>
#include "mainwindow.h"
#include "addressbar.h"
#include "texteditor.h"
#include "highlighter.h"
#include "configdialog.h"
#include "outline.h"
#include "tagsmakedialog.h"

#define STATUS_MSG_TIMEOUT  (2000)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "MainWindow");

    mdiArea = new QMdiArea(this);
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setViewMode(QMdiArea::TabbedView);
    mdiArea->setTabsClosable(true);
    mdiArea->setTabsMovable(true);
    mdiArea->setAcceptDrops(true);
    setCentralWidget(mdiArea);
    windowMapper = new QSignalMapper(this);
    textCodecMapper = new QSignalMapper(this);
    newLineCodeMapper = new QSignalMapper(this);
    fileOpenMapper = new QSignalMapper(this);
    dirOpenMapper = new QSignalMapper(this);
    outlineDock = new Outline(this);
    outlineDock->setObjectName("outline");
    outlineDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    outlineDock->setFeatures(QDockWidget::AllDockWidgetFeatures);
    findDialog = new FindDialog(this);
    replaceDialog = new ReplaceDialog(this);
    grepDialog = new GrepDialog(this);

    addDockWidget(Qt::RightDockWidgetArea, outlineDock);

    createActions();
    createToolBar();
    createMenus();
    createStatusBar();
    updateMenus();
    updateShortcut();

    setWindowTitle("MyEditor");
    setWindowIcon(QIcon(":/images/D.png"));

    /* ウィンドウ状態の復元 */
    restoreGeometry(settings->value("regist/geometry").toByteArray());
    restoreState(settings->value("regist/status").toByteArray());
    fileHistory = settings->value("fileHistory").toStringList();
    dirHistory = settings->value("dirHistory").toStringList();
    setAcceptDrops(true);

    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateMenus()));
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateOutline()));
    connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));
    connect(textCodecMapper, SIGNAL(mapped(QString)), this, SLOT(setTextCodec(QString)));
    connect(newLineCodeMapper, SIGNAL(mapped(int)), this, SLOT(setNewLineCode(int)));
    connect(fileOpenMapper, SIGNAL(mapped(QString)), this, SLOT(openFile(QString)));
    connect(dirOpenMapper, SIGNAL(mapped(QString)), this, SLOT(openDir(QString)));
    connect(outlineDock, SIGNAL(changedSelection(int)), this, SLOT(updateEditLine(int)));
    connect(findDialog, SIGNAL(find(FindDialog::FindParam)), SLOT(find(FindDialog::FindParam)));
}

MainWindow::~MainWindow()
{
}

QList<TextEditor *> MainWindow::textEditorList()
{
    QList<TextEditor *> list;
    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        TextEditor *textEdit = qobject_cast<TextEditor *>(window->widget());
        list << textEdit;
    }
    return list;
}

void MainWindow::newFile()
{
    TextEditor *textEdit = createTextEditor();
    textEdit->newFile();
    textEdit->show();

}

void MainWindow::openFile(QString fileName)
{
    if (fileName.isEmpty()) {
        TextEditor *activeEdit = activeMdiChild();
        QString dir_path = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
        if (activeEdit) {
            QDir dir(activeEdit->currentFile());
            dir_path = dir.path();
        }
        fileName = QFileDialog::getOpenFileName(this, tr("ファイルを開く"),
                                                dir_path,
                                                tr("すべてのファイル (*.*)"));
    }

    if (!fileName.isEmpty()) {
        addFileHistory(fileName);
        addDirHistory(fileName);
        QMdiSubWindow *existing = findMdiChild(fileName);
        if (existing) {
            mdiArea->setActiveSubWindow(existing);
            return;
        }

        TextEditor *textEdit = createTextEditor();
        if (textEdit->openFile(fileName)) {
            statusBar()->showMessage(tr("ファイル読込終了"), STATUS_MSG_TIMEOUT);
            textEdit->show();
        } else {
            textEdit->close();
        }

        QMdiSubWindow *active = mdiArea->activeSubWindow();
        if (active) {
            QFileSystemModel model;
            active->setWindowIcon(model.fileIcon(model.index(fileName)));
        }
    }
}

void MainWindow::openDir(QString path)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("ファイルを開く"),
                                                path,
                                                tr("すべてのファイル (*.*)"));
    openFile(fileName);
}

void MainWindow::save()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit && activeEdit->save())
        statusBar()->showMessage(tr("ファイル保存終了"), STATUS_MSG_TIMEOUT);
}

void MainWindow::saveAll()
{
    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        TextEditor *textEdit = qobject_cast<TextEditor *>(window->widget());
        if (textEdit && textEdit->document()->isModified())
            textEdit->save();
    }
}

void MainWindow::saveAs()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit && activeEdit->saveAs())
        statusBar()->showMessage(tr("ファイル保存終了"), STATUS_MSG_TIMEOUT);
}

void MainWindow::revert()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit->document()->isModified() && !activeEdit->isUntitled()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, "", tr("'%1' は変更されています。\n"
                                                "変更は破棄されますが継続しますか?").arg(activeEdit->userFriendlyCurrentFile()),
                                   QMessageBox::Ok | QMessageBox::Cancel);
        switch (ret) {
            case QMessageBox::Ok:
                if (activeEdit && activeEdit->openFile(activeEdit->currentFile())) {
                    statusBar()->showMessage(tr("再読み込みしました"), STATUS_MSG_TIMEOUT);
                    updateMenus();
                }
                break;
            case QMessageBox::Cancel:
                statusBar()->showMessage(tr("再読み込みをキャンセルしました"), STATUS_MSG_TIMEOUT);
                break;
            default:
                break;
        }
    } else {
        if (activeEdit && activeEdit->openFile(activeEdit->currentFile())) {
            statusBar()->showMessage(tr("再読み込みしました"), STATUS_MSG_TIMEOUT);
            updateMenus();
        }
    }
}

void MainWindow::undo()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit)
        activeEdit->undo();
}

void MainWindow::redo()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit)
        activeEdit->redo();
}

void MainWindow::copy()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit)
        activeEdit->copy();
}

void MainWindow::cut()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit)
        activeEdit->cut();
}

void MainWindow::paste()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit)
        activeEdit->paste();
}

void MainWindow::del()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit)
        activeEdit->cut();
}

void MainWindow::selAll()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit)
        activeEdit->selectAll();
}

void MainWindow::zoomIn()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit) {
        qreal zoom = activeEdit->configs().zoom;
        zoom += 0.1;
        activeEdit->setZoom(zoom);
    }
}

void MainWindow::zoomOut()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit) {
        qreal zoom = activeEdit->configs().zoom;
        zoom -= 0.1;
        activeEdit->setZoom(zoom);
    }
}

void MainWindow::zoomChanged(int zoom)
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit) {
        zoom = zoom <= 0 ? 1 : zoom;
        activeEdit->setZoom(zoom / 100.0);
    }
}

void MainWindow::lowercase()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit && activeEdit->textCursor().hasSelection()) {
        QString text = activeEdit->textCursor().selectedText().toLower();
        activeEdit->textCursor().insertText(text);
    }
}

void MainWindow::uppercase()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit && activeEdit->textCursor().hasSelection()) {
        QString text = activeEdit->textCursor().selectedText().toUpper();
        activeEdit->textCursor().insertText(text);
    }
}

void MainWindow::find()
{
    TextEditor *activeEdit = activeMdiChild();
    if (!activeEdit)
        return;

    findDialog->setFindFormat(activeEdit->configs().findFormats);
    findDialog->setText(activeEdit->textCursor().selectedText());

    if (findDialog->isHidden()) {
        findDialog->show();
        findDialog->exec();
        findDialog->hide();
    } else {
        findDialog->activateWindow();
    }
}

void MainWindow::find(FindDialog::FindParam param)
{
    TextEditor *activeEdit = activeMdiChild();
    if (!activeEdit)
        return;
    marking(param.index, param.data);
    qDebug("findwordsCount=%d\n", activeEdit->findwordsCount(param.index));

    if (param.data.text.isEmpty())
        return;

    lastSearch = param;

    if (lastSearch.findType == FindDialog::FindPrev) {
        findPrev();
    } else if (lastSearch.findType == FindDialog::FindNext){
        findNext();
    }
}

void MainWindow::marking(int index, const TextEditor::KeywordData &keyword)
{
    TextEditor *activeEdit = activeMdiChild();
    if (!activeEdit)
        return;
    activeEdit->setFindword(index, keyword);
    QVector<int> matchLines = activeEdit->findwordMatchLines(index);
    outlineDock->updateFindwordMatchLines(matchLines);
}

void MainWindow::findNext()
{
    TextEditor *activeEdit = activeMdiChild();
    if (!activeEdit)
        return;

    QRegExp pattern(Highlighter::convertText(lastSearch.data.text, lastSearch.data.option));
    QTextCursor cursor = activeEdit->document()->find(pattern, activeEdit->textCursor());
    if (!cursor.isNull()) {
        activeEdit->setTextCursor(cursor);
    } else {
        do {
            cursor = activeEdit->textCursor();
            statusBar()->showMessage(tr("再検索"), STATUS_MSG_TIMEOUT);
            cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
            cursor = activeEdit->document()->find(pattern, cursor);
            if (!cursor.isNull()) {
                activeEdit->setTextCursor(cursor);
                break;
            }

            if (lastSearch.warningNavi) {
                QMessageBox::warning(this, tr("後方検索"), tr("'%1'は見つかりませんでした。").arg(lastSearch.data.text));
            } else {
                statusBar()->showMessage(tr("\"%1\" は見つかりませんでした").arg(lastSearch.data.text), STATUS_MSG_TIMEOUT);
            }
            QTextCursor cr = activeEdit->textCursor();
            cr.clearSelection();
            activeEdit->setTextCursor(cr);
        } while (0);
    }
}

void MainWindow::findPrev()
{
    TextEditor *activeEdit = activeMdiChild();
    if (!activeEdit)
        return;

    QRegExp pattern(Highlighter::convertText(lastSearch.data.text, lastSearch.data.option));
    QTextCursor cursor = activeEdit->document()->find(pattern, activeEdit->textCursor(), QTextDocument::FindBackward);
    if (!cursor.isNull()) {
        activeEdit->setTextCursor(cursor);
    } else {
        do {
            cursor = activeEdit->textCursor();
            statusBar()->showMessage(tr("再検索"), STATUS_MSG_TIMEOUT);
            cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
            cursor = activeEdit->document()->find(pattern, cursor, QTextDocument::FindBackward);
            if (!cursor.isNull()) {
                activeEdit->setTextCursor(cursor);
                break;
            }

            if (lastSearch.warningNavi) {
                QMessageBox::warning(this, tr("前方検索"), tr("'%1'は見つかりませんでした。").arg(lastSearch.data.text));
            } else {
                statusBar()->showMessage(tr("\"%1\" は見つかりませんでした").arg(lastSearch.data.text), STATUS_MSG_TIMEOUT);
            }
            QTextCursor cr = activeEdit->textCursor();
            cr.clearSelection();
            activeEdit->setTextCursor(cr);
        } while (0);
    }
}

void MainWindow::replace()
{
    TextEditor *activeEdit = activeMdiChild();
    if (!activeEdit)
        return;

    replaceDialog->setCurrentTextEditor(activeEdit);

    if (replaceDialog->isHidden()) {
        replaceDialog->show();
        replaceDialog->exec();
        replaceDialog->hide();
    } else {
        replaceDialog->activateWindow();
    }
}

void MainWindow::grep()
{
    if (grepDialog->isHidden()) {
        grepDialog->show();
        grepDialog->exec();
        grepDialog->hide();
    } else {
        grepDialog->activateWindow();
    }
}

void MainWindow::ctagsMake()
{
    TagsMakeDialog d(this);

    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit)
        d.setDirPath(QFileInfo(activeEdit->currentFile()).dir().path());

    d.exec();
}

void MainWindow::ctagsJump()
{
    TextEditor *activeEdit = activeMdiChild();
    if (!activeEdit)
        return;
    QDir dir = QFileInfo(activeEdit->currentFile()).dir();

    while (!QFile::exists(dir.path() + QDir::separator() + "tags")) {
        if (!dir.cdUp()) {
            QMessageBox::StandardButton ret;
            ret = QMessageBox::warning(this, "", tr("タグファイルが見つかりません作成しますか？"), QMessageBox::Ok | QMessageBox::Cancel);
            if (ret == QMessageBox::Ok)
                tagsMakeAct->trigger();
            return;
        }
    }
    QString tagsFile = dir.path() + QDir::separator() + "tags";
    QTextCursor textCursor = activeEdit->textCursor();
    textCursor.select(QTextCursor::WordUnderCursor);
    QString searchWord = textCursor.selectedText();
    QFile file(tagsFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    QVector<QStringList> matchs;
    while (!in.atEnd()) {
        QStringList column = in.readLine().split("\t");
        if (column.at(0).at(0) == QChar('!'))
            continue;
        if (column[0] == searchWord)
            matchs.append(column);
    }
    if (matchs.isEmpty()) {
        statusBar()->showMessage(tr("タグは見つかりませんでした"));
        return;
    }
    if (matchs.count() > 1) {
        return;
    }
    TagsJumpStack stack;
    stack.filePath = activeEdit->currentFile();
    stack.lineNumber = activeEdit->cursorForLineNumber();

    QString openFileName = QFileInfo(matchs.at(0).at(1)).canonicalFilePath();
    QString line_str = matchs.at(0).at(2);
    int line = line_str.replace(QRegExp("..$"), "").toInt();
    openFile(openFileName);
    TextEditor *openEdior = activeMdiChild();
    if (openEdior) {
        openEdior->setCursorForLineNumber(line);
        tagsJumpStacks.prepend(stack);
    }
    /*
    QMdiSubWindow *window = findMdiChild(openFileName);
    if (window) {
        TextEditor *editor = qobject_cast<TextEditor *>(window->widget());
        editor->setLinePos(line);
    }
    */
}

void MainWindow::ctagsJumpBack()
{
    if (tagsJumpStacks.isEmpty())
        return;
    TagsJumpStack stack = tagsJumpStacks.value(0);
    qDebug() << stack.filePath;
    qDebug() << stack.lineNumber;
    openFile(stack.filePath);
    TextEditor *openEdior = activeMdiChild();
    if (openEdior) {
        openEdior->setCursorForLineNumber(stack.lineNumber);
    }
    tagsJumpStacks.remove(0);
}

void MainWindow::slotGoLine()
{
    TextEditor *activeEdit = activeMdiChild();
    if (!activeEdit)
        return;
    bool ok;
    int line = QInputDialog::getInt(this, tr("指定行へジャンプ"),
                                          tr("行番号:"), activeEdit->cursorForLineNumber(),
                                          1, 2147483647, 1, &ok);
    if (ok) {
        activeEdit->setCursorForLineNumber(line);
        activeEdit->setFocus();
    }
}

void MainWindow::markAllClear()
{
    TextEditor::KeywordData data;
    data.text = "";
    data.option.caseSensitive = false;
    data.option.wholeWords = true;
    data.option.regularExpression = false;
    for (int i = 0; i < 10; ++i) {
        marking(i, data);
    }
}

void MainWindow::wordmark()
{
    const QString &text = activeMdiChild()->textCursor().selectedText();
    TextEditor::KeywordData data;
    data.text = text;
    data.option.caseSensitive = true;
    data.option.wholeWords = true;
    data.option.regularExpression = false;
    marking(0, data);

    lastSearch.index = 0;
    lastSearch.data = data;
}

void MainWindow::option()
{
    ConfigDialog d(this);
    d.exec();
}

void MainWindow::about()
{
    QMessageBox::information(this, tr("未実装"), tr("coming soon..."));
}

TextEditor *MainWindow::createTextEditor()
{
    TextEditor *textEdit = new TextEditor(this);
    textEdit->setContextMenuPolicy(Qt::ActionsContextMenu);
    QMenu *editMenu = textEdit->createStandardContextMenu();
    editMenu->clear();
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addAction(delAct);
    editMenu->addSeparator();
    editMenu->addAction(selAllAct);
    editMenu->addSeparator();
    editMenu->addAction(markAllClearAct);
    textEdit->addActions(editMenu->actions());

    QMdiSubWindow *subwin = mdiArea->addSubWindow(textEdit);
    subwin->setWindowIcon(QIcon(":/images/document--pencil.png"));
    subwin->systemMenu()->clear();
    subwin->systemMenu()->addAction(closeAct);
    subwin->systemMenu()->addSeparator();
    subwin->systemMenu()->addAction(saveAct);
    subwin->systemMenu()->addAction(saveAsAct);
    subwin->systemMenu()->addSeparator();
    subwin->systemMenu()->addAction(revertAct);

    connect(textEdit->document(), SIGNAL(modificationChanged(bool)), saveAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(untitledChanged(bool)), revertAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(undoAvailable(bool)), undoAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(redoAvailable(bool)), redoAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)), cutAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)), delAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)), lowercaseAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)), uppercaseAct, SLOT(setEnabled(bool)));
    //connect(textEdit, SIGNAL(textChanged()), this, SLOT(updateOutline()));
    connect(textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(updateOutlineCurrent()));
    connect(textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(updateCurrentCharCode()));
    connect(textEdit, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
    connect(textEdit, SIGNAL(mouseClickRequest(QMouseEvent*)), mouseClickAct, SLOT(trigger()));
    connect(textEdit, SIGNAL(mouseDoubleClickRequest(QMouseEvent*)), mouseDoubleClickAct, SLOT(trigger()));

    return textEdit;
}

void MainWindow::updateMenus()
{
    TextEditor *activeEdit = activeMdiChild();

    bool hasMdiChild = (activeEdit != 0);
    saveAllAct->setEnabled(hasMdiChild);
    saveAsAct->setEnabled(hasMdiChild);
    pasteAct->setEnabled(hasMdiChild);
    selAllAct->setEnabled(hasMdiChild);
    findAct->setEnabled(hasMdiChild);
    findNextAct->setEnabled(hasMdiChild);
    findPrevAct->setEnabled(hasMdiChild);
    replaceAct->setEnabled(hasMdiChild);
    goLineAct->setEnabled(hasMdiChild);
    closeAct->setEnabled(hasMdiChild);
    closeAllAct->setEnabled(hasMdiChild);
    nextSubWinAct->setEnabled(hasMdiChild);
    prevSubWinAct->setEnabled(hasMdiChild);

    bool isModified = (activeEdit && activeEdit->document()->isModified());
    saveAct->setEnabled(isModified);

    bool isUntitled = (activeEdit && !activeEdit->isUntitled());
    revertAct->setEnabled(isUntitled);

    bool isUndoAvailable = (activeEdit && activeEdit->document()->isUndoAvailable());
    undoAct->setEnabled(isUndoAvailable);
    bool isRedoAvailable = (activeEdit && activeEdit->document()->isRedoAvailable());
    redoAct->setEnabled(isRedoAvailable);

    bool hasSelection = (activeEdit && activeEdit->textCursor().hasSelection());
    delAct->setEnabled(hasSelection);
    cutAct->setEnabled(hasSelection);
    copyAct->setEnabled(hasSelection);
    lowercaseAct->setEnabled(hasSelection);
    uppercaseAct->setEnabled(hasSelection);

    if (activeEdit) {
        codecInfo->setText(activeEdit->textCodecForName());
        newLineCodeInfo->setText(activeEdit->newLineCodeName());
        zoomInfo->setValue(activeEdit->configs().zoom * 100.0);
    } else {
        codecInfo->setText("Unknown");
        newLineCodeInfo->setText("Unknown");
        zoomInfo->setValue(0);
    }

    updateOutlineCurrent();
    updateCurrentCharCode();
}

void MainWindow::updateOutline()
{
    TextEditor *activeEdit = activeMdiChild();
    if (!activeEdit) {
        outlineDock->clear();
        return;
    }

    QString fileName = QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/myeditor_outline";

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return;

    QTextStream out(&file);
    out << activeEdit->toPlainText();
    file.close();
    outlineDock->reload(fileName);
}

void MainWindow::updateOutlineCurrent()
{
    TextEditor *activeEdit = activeMdiChild();
    if (!activeEdit) {
        return;
    }
    int line = activeEdit->cursorForLineNumber();
    outlineDock->selectionLineItem(line);
}

void MainWindow::updateCurrentCharCode()
{
    TextEditor *activeEdit = activeMdiChild();
    if (!activeEdit) {
        lineNumberInfo->setText(tr("%1行 %2桁").arg(0).arg(0));
        charCodeInfo->setText("Unicode:");
        return;
    }
    int lineNumber = activeEdit->cursorForLineNumber();
    int columnNumber = activeEdit->cursorForColumnNumber();
    int pos = activeEdit->textCursor().position();
    QChar c = activeEdit->document()->characterAt(pos);
    charCodeInfo->setText(QString("Unicode:%1").arg(c.unicode()));
    lineNumberInfo->setText(tr("%1行 %2桁").arg(lineNumber).arg(columnNumber));
}

void MainWindow::updateSelection()
{
    TextEditor *activeEdit = activeMdiChild();
    if (activeEdit && activeEdit->textCursor().hasSelection()) {
        const QString &selectedText = activeEdit->textCursor().selectedText();
        statusBar()->showMessage(tr("選択文字数:%1 (行:%2)").arg(selectedText.count()).arg(selectedText.count(QChar::ParagraphSeparator) + 1));
    } else {
        statusBar()->clearMessage();
    }
}

void MainWindow::updateOpenedFileMenu()
{
    openedFileMenu->clear();
    foreach (const QString &path, fileHistory) {
        QMdiSubWindow *window = findMdiChild(path);
        QAction *action  = openedFileMenu->addAction(path);
        action->setCheckable(true);
        action->setChecked(window != NULL);
        connect(action, SIGNAL(triggered()), fileOpenMapper, SLOT(map()));
        fileOpenMapper->setMapping(action, path);
    }
}

void MainWindow::updateOpenedDirMenu()
{
    openedDirMenu->clear();
    foreach (const QString &path, dirHistory) {
        QAction *action  = openedDirMenu->addAction(path);
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), dirOpenMapper, SLOT(map()));
        dirOpenMapper->setMapping(action, path);
    }
}

void MainWindow::updateWindowMenu()
{
    windowMenu->clear();
    windowMenu->addAction(closeAct);
    windowMenu->addAction(closeAllAct);
    windowMenu->addSeparator();
    windowMenu->addAction(nextSubWinAct);
    windowMenu->addAction(prevSubWinAct);

    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    if (!windows.isEmpty()) windowMenu->addSeparator();

    TextEditor *activeEdit = activeMdiChild();
    for (int i = 0; i < windows.size(); ++i) {
        TextEditor *child = qobject_cast<TextEditor *>(windows.at(i)->widget());

        QString text;
        if (i < 9) {
            text = tr("&%1 %2").arg(i + 1)
                               .arg(child->userFriendlyCurrentFile());
        } else {
            text = tr("%1 %2").arg(i + 1)
                              .arg(child->userFriendlyCurrentFile());
        }
        QAction *action  = windowMenu->addAction(text);
        action->setCheckable(true);
        action->setChecked(child == activeEdit);
        //action->setIcon(windows.at(i)->windowIcon());
        connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
        windowMapper->setMapping(action, windows.at(i));
    }
}

void MainWindow::updateShortcut()
{
    newFileAct->setShortcut(QKeySequence::New);
    openFileAct->setShortcut(QKeySequence::Open);
    saveAct->setShortcut(QKeySequence::Save);
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    closeAct->setShortcut(QKeySequence::Close);
    //closeAllAct->setShortcut(QKeySequence::Close);
    exitAct->setShortcut(QKeySequence::Quit);
    undoAct->setShortcut(QKeySequence::Undo);
    redoAct->setShortcut(QKeySequence::Redo);
    cutAct->setShortcut(QKeySequence::Cut);
    copyAct->setShortcut(QKeySequence::Copy);
    pasteAct->setShortcut(QKeySequence::Paste);
    delAct->setShortcut(QKeySequence::Delete);
    selAllAct->setShortcut(QKeySequence::SelectAll);
    zoomInAct->setShortcut(tr("Ctrl+Wheel"));
    //zoomOutAct->setShortcut(tr("Ctrl+Wheel"));
    findAct->setShortcut(QKeySequence::Find);
    findNextAct->setShortcut(QKeySequence::FindNext);
    findPrevAct->setShortcut(QKeySequence::FindPrevious);
    replaceAct->setShortcut(QKeySequence::Replace);
    grepAct->setShortcut(tr("Ctrl+Shift+F", "Grep"));
    tagsJumpAct->setShortcut(tr("F12"));
    tagsJumpBackAct->setShortcut(tr("Shift+F12"));
    goLineAct->setShortcut(tr("Ctrl+G", "Go"));
    //statusBarToggleAct->setShortcut(QKeySequence::New);
    nextSubWinAct->setShortcut(QKeySequence::NextChild);
    prevSubWinAct->setShortcut(QKeySequence::PreviousChild);
    //optionAct->setShortcut(QKeySequence::New);
    //aboutAct->setShortcut(QKeySequence::New);
    //aboutQtAct->setShortcut(QKeySequence::New);
}

void MainWindow::updateStatusbarTextCodec()
{
    const char *codec_names[] = {
        "System",
    //    "Apple Roman",
    //    "Big5",
    //    "Big5-HKSCS",
    //    "CP949",
        "EUC-JP",
    //    "EUC-KR",
    //    "GB18030-0",
    //    "IBM 850",
    //    "IBM 866",
    //    "IBM 874",
    //    "ISO 2022-JP",
    //    "ISO 8859-1 to 10",
    //    "ISO 8859-13 to 16",
    //    "Iscii-Bng",
    //    "JIS X 0201",
    //    "JIS X 0208",
    //    "KOI8-R",
    //    "KOI8-U",
    //    "MuleLao-1",
    //    "ROMAN8",
        "Shift_JIS",
    //    "TIS-620",
    //    "TSCII",
        "UTF-8",
        "UTF-16",
        "UTF-16BE",
        "UTF-16LE",
        "UTF-32",
        "UTF-32BE",
        "UTF-32LE",
    //    "Windows-1250 to 1258",
    //    "WINSAMI2"
    };

    TextEditor *activeEdit = activeMdiChild();
    QString textCodecName = "";
    if (activeEdit)
        textCodecName = activeEdit->textCodecForName();
    codecInfo->menu()->clear();
    int size = sizeof(codec_names) / sizeof(codec_names[0]);
    for (int i = 0; i < size; ++i) {
        QAction *action  = codecInfo->menu()->addAction(codec_names[i]);
        action->setCheckable(true);
        action->setChecked(codec_names[i] == textCodecName);
        connect(action, SIGNAL(triggered()), textCodecMapper, SLOT(map()));
        textCodecMapper->setMapping(action, codec_names[i]);
    }
}

void MainWindow::updateStatusbarIndention()
{
    const char *indention_names[] = {
        "CR+LF",
        "LF",
        "CR"
    };

    TextEditor *activeEdit = activeMdiChild();
    TextEditor::NewLineCode new_line_code = TextEditor::NewLineCodeUnknown;
    if (activeEdit)
        new_line_code = activeEdit->newLineCode();
    newLineCodeInfo->menu()->clear();
    for (int i = 0; i < TextEditor::NewLineCodeUnknown; ++i) {
        QAction *action  = newLineCodeInfo->menu()->addAction(indention_names[i]);
        action->setCheckable(true);
        action->setChecked(static_cast<TextEditor::NewLineCode>(i) == new_line_code);
        connect(action, SIGNAL(triggered()), newLineCodeMapper, SLOT(map()));
        newLineCodeMapper->setMapping(action, i);
    }
}

/**
 * ステータスバー表示変更
 */
void MainWindow::changeStatusBarVisible(bool visible)
{
    // メニューバーアクション状態変更
    statusBar()->setVisible(visible);
}

void MainWindow::setActiveSubWindow(QWidget *window)
{
    if (!window)
        return;
    mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void MainWindow::setTextCodec(QString codec)
{
    TextEditor *activeEdit = activeMdiChild();
    if (!activeEdit) {
        return;
    }
    activeEdit->setTextCodecForName(codec);
    codecInfo->setText(activeEdit->textCodecForName());
    if (!activeEdit->isUntitled()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, "", tr("指定の文字コードで再読み込みお行いますか？"), QMessageBox::Ok | QMessageBox::Cancel);
        if (ret == QMessageBox::Ok) {
            QString fileName = activeEdit->currentFile();
            activeEdit->openFile(fileName);
        }
    }
}

void MainWindow::setNewLineCode(int index)
{
    TextEditor *activeEdit = activeMdiChild();
    if (!activeEdit) {
        return;
    }
    activeEdit->setNewLineCode(static_cast<TextEditor::NewLineCode>(index));
    newLineCodeInfo->setText(activeEdit->newLineCodeName());
}

void MainWindow::updateEditLine(int line)
{
    TextEditor *activeEdit = activeMdiChild();
    if (!activeEdit) {
        return;
    }

    activeEdit->setCursorForLineNumber(line);

    activeEdit->setFocus();
}

void MainWindow::addFileHistory(QString path)
{
    path = QFile(path).fileName();
    fileHistory.removeOne(path);
    fileHistory.insert(0, path);
    for (int i = 10; i < fileHistory.size(); ++i)
    {
        fileHistory.removeLast();
    }
}

void MainWindow::addDirHistory(QString path)
{
    path = QFileInfo(path).absoluteDir().path();
    dirHistory.removeOne(path);
    dirHistory.insert(0, path);
    for (int i = 10; i < dirHistory.size(); ++i)
    {
        dirHistory.removeLast();
    }
}

void MainWindow::createActions()
{
    separator = new QAction(this);
    separator->setSeparator(true);

    newFileAct = new QAction(tr("新規作成(&N)"), this);
    newFileAct->setIcon(QIcon(":/images/document.png"));
    newFileAct->setStatusTip(tr("ファイルの新規作成"));
    connect(newFileAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openFileAct = new QAction(tr("ファイルを開く(&O)..."), this);
    openFileAct->setIcon(QIcon(":/images/folder-horizontal-open.png"));
    openFileAct->setStatusTip(tr("ファイルを開く"));
    connect(openFileAct, SIGNAL(triggered()), this, SLOT(openFile()));

    saveAct = new QAction(tr("上書き保存(&O)"), this);
    saveAct->setIcon(QIcon(":/images/disk-return-black.png"));
    saveAct->setStatusTip(tr("上書き保存する"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAllAct = new QAction(tr("すべて保存(&L)"), this);
    saveAllAct->setIcon(QIcon(":/images/disks-black.png"));
    saveAllAct->setStatusTip(tr("すべて保存"));
    connect(saveAllAct, SIGNAL(triggered()), this, SLOT(saveAll()));

    saveAsAct = new QAction(tr("名前を付けて保存(&S)..."), this);
    saveAsAct->setIcon(QIcon(":/images/folder-open-document.png"));
    saveAsAct->setStatusTip(tr("名前を付けて保存する"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    closeAct = new QAction(tr("閉じる(&C)"), this);
    closeAct->setIcon(QIcon(":/images/cross-button.png"));
    closeAct->setStatusTip(tr("タブを閉じる"));
    connect(closeAct, SIGNAL(triggered()), mdiArea, SLOT(closeActiveSubWindow()));

    closeAllAct = new QAction(tr("すべて閉じる"), this);
    closeAllAct->setIcon(QIcon(":/images/cross-white.png"));
    closeAllAct->setStatusTip(tr("すべてのタブを閉じる"));
    connect(closeAllAct, SIGNAL(triggered()), mdiArea, SLOT(closeAllSubWindows()));

    revertAct = new QAction(tr("再読込"), this);
    revertAct->setIcon(QIcon(":/images/arrow-circle-double-135.png"));
    revertAct->setStatusTip(tr("保存時の状態に戻す"));
    connect(revertAct, SIGNAL(triggered()), this, SLOT(revert()));

    exitAct = new QAction(tr("終了"), this);
    exitAct->setIcon(QIcon(":/images/door-open-in.png"));
    exitAct->setStatusTip(tr("アプリケーションを終了する"));
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    undoAct = new QAction(tr("元に戻す(&U)"), this);
    undoAct->setIcon(QIcon(":/images/arrow-curve-180-left.png"));
    undoAct->setStatusTip(tr("元に戻す"));
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));

    redoAct = new QAction(tr("やり直す(&R)"), this);
    redoAct->setIcon(QIcon(":/images/arrow-curve-000-left"));
    redoAct->setStatusTip(tr("やり直す"));
    connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

    cutAct = new QAction(tr("切り取り(&T)"), this);
    cutAct->setIcon(QIcon(":/images/scissors.png"));
    cutAct->setStatusTip(tr("切り取り"));
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

    copyAct = new QAction(tr("コピー(&C)"), this);
    copyAct->setIcon(QIcon(":/images/blue-documents.png"));
    copyAct->setStatusTip(tr("コピー"));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAct = new QAction(tr("貼り付け(&P)"), this);
    pasteAct->setIcon(QIcon(":/images/clipboard-paste.png"));
    pasteAct->setStatusTip(tr("貼り付け"));
    connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));

    delAct = new QAction(tr("削除(&D)"), this);
    delAct->setIcon(QIcon(":/images/cross-script.png"));
    delAct->setStatusTip(tr("削除"));
    connect(delAct, SIGNAL(triggered()), this, SLOT(del()));

    selAllAct = new QAction(tr("全て選択(&A)"), this);
    selAllAct->setIcon(QIcon(":/images/selection-select-input.png"));
    selAllAct->setStatusTip(tr("全て選択"));
    connect(selAllAct, SIGNAL(triggered()), this, SLOT(selAll()));

    zoomInAct = new QAction(tr("ズームイン"), this);
    zoomInAct->setStatusTip(tr("ズームイン"));
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(tr("ズームアウト"), this);
    zoomOutAct->setStatusTip(tr("ズームアウト"));
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    lowercaseAct = new QAction(tr("小文字(&L)"), this);
    lowercaseAct->setStatusTip(tr("小文字"));
    connect(lowercaseAct, SIGNAL(triggered()), this, SLOT(lowercase()));

    uppercaseAct = new QAction(tr("大文字(&U)"), this);
    uppercaseAct->setStatusTip(tr("大文字"));
    connect(uppercaseAct, SIGNAL(triggered()), this, SLOT(uppercase()));

    findAct = new QAction(tr("検索(&F)..."), this);
    findAct->setIcon(QIcon(":/images/magnifier.png"));
    findAct->setStatusTip(tr("検索"));
    connect(findAct, SIGNAL(triggered()), this, SLOT(find()));

    findNextAct = new QAction(tr("次を検索"), this);
    findNextAct->setIcon(QIcon(":/images/arrow-skip-270.png"));
    findNextAct->setStatusTip(tr("次を検索する"));
    connect(findNextAct, SIGNAL(triggered()), this, SLOT(findNext()));

    findPrevAct = new QAction(tr("前を検索"), this);
    findPrevAct->setIcon(QIcon(":/images/arrow-skip-090.png"));
    findPrevAct->setStatusTip(tr("前を検索する"));
    connect(findPrevAct, SIGNAL(triggered()), this, SLOT(findPrev()));

    replaceAct = new QAction(tr("置換"), this);
    replaceAct->setIcon(QIcon(":/images/magnifier--pencil.png"));
    replaceAct->setStatusTip(tr("置換する"));
    connect(replaceAct, SIGNAL(triggered()), this, SLOT(replace()));

    grepAct = new QAction(tr("Grep"), this);
    grepAct->setIcon(QIcon(":/images/folder-search-result.png"));
    grepAct->setStatusTip(tr("Grep"));
    connect(grepAct, SIGNAL(triggered()), this, SLOT(grep()));

    tagsMakeAct = new QAction(tr("タグファイル作成"), this);
    tagsMakeAct->setIcon(QIcon(":/images/tag--pencil.png"));
    tagsMakeAct->setStatusTip(tr("tagsファイル作成"));
    connect(tagsMakeAct, SIGNAL(triggered()), this, SLOT(ctagsMake()));

    tagsJumpAct = new QAction(tr("タグジャンプ"), this);
    tagsJumpAct->setIcon(QIcon(":/images/tag--arrow.png"));
    tagsJumpAct->setStatusTip(tr("タグジャンプ"));
    connect(tagsJumpAct, SIGNAL(triggered()), this, SLOT(ctagsJump()));

    tagsJumpBackAct = new QAction(tr("タグジャンプバック"), this);
    tagsJumpBackAct->setIcon(QIcon(":/images/tag--minus.png"));
    tagsJumpBackAct->setStatusTip(tr("タグジャンプバック"));
    connect(tagsJumpBackAct, SIGNAL(triggered()), this, SLOT(ctagsJumpBack()));

    goLineAct = new QAction(tr("行指定"), this);
    goLineAct->setIcon(QIcon(":/images/arrow-step.png"));
    goLineAct->setStatusTip(tr("指定行へジャンプ"));
    connect(goLineAct, SIGNAL(triggered()), this, SLOT(slotGoLine()));

    markAllClearAct = new QAction(tr("マーク全解除"), this);
    markAllClearAct->setIcon(QIcon(":/images/edit-color.png"));
    markAllClearAct->setStatusTip(tr("マーク全解除"));
    connect(markAllClearAct, SIGNAL(triggered()), this, SLOT(markAllClear()));

    statusBarToggleAct = new QAction(tr("ステータスバー"), this);
    statusBarToggleAct->setCheckable(true);
    statusBarToggleAct->setChecked(!statusBar()->isVisible());
    connect(statusBarToggleAct, SIGNAL(toggled(bool)), this, SLOT(changeStatusBarVisible(bool)));

    nextSubWinAct = new QAction(tr("次のウィドウ(&N)"), this);
    nextSubWinAct->setIcon(QIcon(":/images/arrow-stop.png"));
    nextSubWinAct->setStatusTip(tr("次のウィンドウ"));
    connect(nextSubWinAct, SIGNAL(triggered()), mdiArea, SLOT(activateNextSubWindow()));

    prevSubWinAct = new QAction(tr("前のウィンドウ(&P)"), this);
    prevSubWinAct->setIcon(QIcon(":/images/arrow-stop-180.png"));
    prevSubWinAct->setStatusTip(tr("前のウィンドウ"));
    connect(prevSubWinAct, SIGNAL(triggered()), mdiArea, SLOT(activatePreviousSubWindow()));

    optionAct = new QAction(tr("オプション(&0)..."), this);
    optionAct->setIcon(QIcon(":/images/gear.png"));
    optionAct->setStatusTip(tr("オプション"));
    connect(optionAct, SIGNAL(triggered()), this, SLOT(option()));

    aboutAct = new QAction(tr("MyEditorについて..."), this);
    aboutAct->setStatusTip(tr("MyEditorについて"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("Qtについて..."), this);
    aboutQtAct->setStatusTip(tr("Qtについて"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    mouseClickAct = new QAction(tr("クリック"), this);

    mouseDoubleClickAct = new QAction(tr("ダブルクリック"), this);
    connect(mouseDoubleClickAct, SIGNAL(triggered()), this, SLOT(wordmark()));

}

void MainWindow::createMenus()
{
    openedFileMenu = new QMenu(tr("最近使ったファイル"), this);
    openedFileMenu->setIcon(QIcon(":/images/document-clock.png"));
    updateOpenedFileMenu();
    connect(openedFileMenu, SIGNAL(aboutToShow()), this, SLOT(updateOpenedFileMenu()));

    openedDirMenu = new QMenu(tr("最近使ったフォルダ"), this);
    openedDirMenu->setIcon(QIcon(":/images/clock.png"));
    updateOpenedDirMenu();
    connect(openedDirMenu, SIGNAL(aboutToShow()), this, SLOT(updateOpenedDirMenu()));

    fileMenu = menuBar()->addMenu(tr("ファイル(&F)"));
    fileMenu->addAction(newFileAct);
    fileMenu->addAction(openFileAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAllAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(revertAct);
    fileMenu->addSeparator();
    fileMenu->addMenu(openedFileMenu);
    fileMenu->addMenu(openedDirMenu);
    fileMenu->addSeparator();
    fileMenu->addAction(closeAct);
    fileMenu->addAction(closeAllAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("編集(&E)"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addAction(delAct);
    editMenu->addSeparator();
    editMenu->addAction(selAllAct);
    editMenu->addSeparator();
    editMenu->addAction(lowercaseAct);
    editMenu->addAction(uppercaseAct);

    searchMenu = menuBar()->addMenu(tr("検索(&S)"));
    searchMenu->addAction(findAct);
    searchMenu->addAction(findNextAct);
    searchMenu->addAction(findPrevAct);
    searchMenu->addSeparator();
    searchMenu->addAction(replaceAct);
    searchMenu->addAction(grepAct);
    searchMenu->addSeparator();
    searchMenu->addAction(tagsMakeAct);
    searchMenu->addAction(tagsJumpAct);
    searchMenu->addAction(tagsJumpBackAct);
    searchMenu->addSeparator();
    searchMenu->addAction(goLineAct);

    viewMenu = menuBar()->addMenu(tr("表示(&V)"));
    viewMenu->addAction(statusBarToggleAct);
    viewMenu->addAction(outlineDock->toggleViewAction());
    viewMenu->addAction(toolBar->toggleViewAction());

    windowMenu = menuBar()->addMenu(tr("ウィンドウ(&W)"));
    updateWindowMenu();
    connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));

    toolMenu = menuBar()->addMenu(tr("ツール(&T)"));
    toolMenu->addAction(optionAct);

    helpMenu = menuBar()->addMenu(tr("ヘルプ(&H)"));
    //helpMenu->addAction(aboutAct);
    //helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBar()
{
    toolBar = new QToolBar(tr("ツールバー"), this);
    toolBar->setObjectName("toolbar");
    toolBar->setAllowedAreas(Qt::TopToolBarArea);
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setIconSize(QSize(16, 16));
    this->addToolBar(Qt::TopToolBarArea, toolBar);

    // ToDo
    toolBar->addAction(newFileAct);
    toolBar->addAction(openFileAct);
    toolBar->addAction(saveAct);
    toolBar->addSeparator();
    toolBar->addAction(undoAct);
    toolBar->addAction(redoAct);
    toolBar->addSeparator();
    toolBar->addAction(cutAct);
    toolBar->addAction(copyAct);
    toolBar->addAction(pasteAct);
    toolBar->addSeparator();
    toolBar->addAction(prevSubWinAct);
    toolBar->addAction(nextSubWinAct);
    toolBar->addSeparator();
    toolBar->addAction(optionAct);
    toolBar->addSeparator();
    toolBar->addAction(exitAct);
    toolBar->addSeparator();
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("レディ"));
    charCodeInfo = new QLabel(tr("Unicode:"), this);
    charCodeInfo->setContentsMargins(10, 0, 10, 0);
    statusBar()->addPermanentWidget(charCodeInfo);

    lineNumberInfo = new QPushButton(tr("0行 0桁"), this);
    lineNumberInfo->setFlat(true);
    connect(lineNumberInfo, SIGNAL(clicked()), goLineAct, SLOT(trigger()));
    statusBar()->addPermanentWidget(lineNumberInfo);

    codecInfo = new QPushButton("Unknown", this);
    codecInfo->setFlat(true);
    codecInfo->setMenu(new QMenu(this));
    connect(codecInfo->menu(), SIGNAL(aboutToShow()), this, SLOT(updateStatusbarTextCodec()));
    statusBar()->addPermanentWidget(codecInfo);

    newLineCodeInfo = new QPushButton("Unknown", this);
    newLineCodeInfo->setFlat(true);
    newLineCodeInfo->setMenu(new QMenu(this));
    connect(newLineCodeInfo->menu(), SIGNAL(aboutToShow()), this, SLOT(updateStatusbarIndention()));
    statusBar()->addPermanentWidget(newLineCodeInfo);

    zoomInfo = new QSpinBox(this);
    zoomInfo->setSuffix("%");
    zoomInfo->setMinimum(0);
    zoomInfo->setMaximum(500);
    zoomInfo->setSingleStep(5);
    connect(zoomInfo, SIGNAL(valueChanged(int)), this, SLOT(zoomChanged(int)));
    statusBar()->addPermanentWidget(zoomInfo);
}

TextEditor *MainWindow::activeMdiChild()
{
    if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow())
        return qobject_cast<TextEditor *>(activeSubWindow->widget());
    return 0;
}

QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName)
{
    //QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    QString canonicalFilePath = fileName;

    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        TextEditor *textEdit = qobject_cast<TextEditor *>(window->widget());
        if (textEdit->currentFile() == canonicalFilePath)
            return window;
    }
    return 0;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    mdiArea->closeAllSubWindows();
    if (mdiArea->currentSubWindow()) {
        event->ignore();
    } else {
        settings->setValue("fileHistory", fileHistory);
        settings->setValue("dirHistory", dirHistory);
        settings->setValue("regist/geometry", saveGeometry());
        settings->setValue("regist/status", saveState());
        event->accept();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QMessageBox::StandardButton ret = QMessageBox::Yes;
    if (event->mimeData()->urls().size() > 20) {
        ret = QMessageBox::warning(this, "", tr("複数のファイルを開こうとしています。処理を継続しますか？"),
                                   QMessageBox::Yes | QMessageBox::Cancel);
    }
    if (ret == QMessageBox::Yes) {
        foreach (QUrl url, event->mimeData()->urls()) {
            openFile(url.toLocalFile());
        }
    }
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        const int delta = event->delta();
        if (delta < 0)
            zoomInfo->setValue(zoomInfo->value() - zoomInfo->singleStep());
        else if (delta > 0)
            zoomInfo->setValue(zoomInfo->value() + zoomInfo->singleStep());
        event->ignore();
    }
}
