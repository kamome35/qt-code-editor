#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "finddialog.h"
#include "replacedialog.h"
#include "grepdialog.h"
#include "texteditor.h"

class Outline;
class TagsMakeDialog;
class QMdiArea;
class QMdiSubWindow;
class QSettings;
class QSignalMapper;
class QLabel;
class QSpinBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QList<TextEditor *> textEditorList();

public slots:
    void newFile();
    void openFile(QString fileName = "");
    void openDir(QString path = "");
    void save();
    void saveAll();
    void saveAs();
    void revert();
    void undo();
    void redo();
    void copy();
    void cut();
    void paste();
    void del();
    void selAll();
    void zoomIn();
    void zoomOut();
    void zoomChanged(int zoom);
    void lowercase();
    void uppercase();
    void find();
    void find(FindDialog::FindParam param);
    void marking(int index, const TextEditor::KeywordData &keyword);
    void findNext();
    void findPrev();
    void replace();
    void grep();
    void ctagsMake();
    void ctagsJump();
    void ctagsJumpBack();
    void slotGoLine();
    void markAllClear();
    void wordmark();
    void option();
    void about();
    TextEditor *createTextEditor();
    void updateMenus();
    void updateOutline();
    void updateOutlineCurrent();
    void updateCurrentCharCode();
    void updateSelection();
    void updateOpenedFileMenu();
    void updateOpenedDirMenu();
    void updateWindowMenu();
    void updateShortcut();
    void updateStatusbarTextCodec();
    void updateStatusbarIndention();
    void changeStatusBarVisible(bool visible);
    void setActiveSubWindow(QWidget *window);
    void setTextCodec(QString codec);
    void setNewLineCode(int index);
    void updateEditLine(int line);
    void addFileHistory(QString path);
    void addDirHistory(QString path);

public:
    void createActions();
    void createMenus();
    void createToolBar();
    void createStatusBar();
    TextEditor *activeMdiChild();
    QMdiSubWindow *findMdiChild(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent * event);
    void wheelEvent(QWheelEvent *event);

private:
    QSettings *settings;            // 設定
    QMdiArea *mdiArea;              // MDI
    QSignalMapper *windowMapper;    //
    QSignalMapper *textCodecMapper; //
    QSignalMapper *newLineCodeMapper; //
    QSignalMapper *fileOpenMapper;  //
    QSignalMapper *dirOpenMapper;   //
    Outline *outlineDock;           // アウトライン
    QMenu *fileMenu;                // ファイル
    QMenu *editMenu;                // 編集
    QMenu *searchMenu;              // 検索
    QMenu *viewMenu;                // 表示
    QMenu *windowMenu;              // ウィンドウ
    QMenu *toolMenu;                // ツール
    QMenu *helpMenu;                // ヘルプ
    QMenu *openedFileMenu;          // 最近開いたファイル
    QMenu *openedDirMenu;           // 最近開いたフォルダ
    QAction *separator;             //
    QAction *newFileAct;            // 新規
    QAction *openFileAct;           // 開く
    QAction *saveAct;               // 上書き保存
    QAction *saveAllAct;            // すべて保存
    QAction *saveAsAct;             // 名前を付けて保存
    QAction *closeAct;              // 閉じる
    QAction *closeAllAct;           // すべて閉じる
    QAction *revertAct;             // 保存時の状態に戻す
    QAction *exitAct;               // アプリ終了
    QAction *undoAct;               // 元に戻す
    QAction *redoAct;               // やり直す
    QAction *cutAct;                // 切り取り
    QAction *copyAct;               // コピー
    QAction *pasteAct;              // 貼り付け
    QAction *delAct;                // 削除
    QAction *selAllAct;             // 全て選択
    QAction *zoomInAct;             //
    QAction *zoomOutAct;            //
    QAction *lowercaseAct;          // 小文字
    QAction *uppercaseAct;          // 大文字
    QAction *findAct;               // 検索
    QAction *findNextAct;           // 次を検索
    QAction *findPrevAct;           // 前を検索
    QAction *replaceAct;            // 置換
    QAction *grepAct;               // Grep
    QAction *tagsMakeAct;           // tags作成
    QAction *tagsJumpAct;           // タグジャンプ
    QAction *tagsJumpBackAct;       // タグジャンプバック
    QAction *goLineAct;             // 指定行へジャンプ
    QAction *markAllClearAct;       // マークをすべて解除
    QAction *statusBarToggleAct;    // ステータスバー
    QAction *nextSubWinAct;         // 次のウィンドウ
    QAction *prevSubWinAct;         // 前のウィンドウ
    QAction *optionAct;             // オプション
    QAction *aboutAct;              //
    QAction *aboutQtAct;            // Qtについて
    QAction *mouseClickAct;         // クリック
    QAction *mouseDoubleClickAct;   // ダブルクリック
    QToolBar *toolBar;              // ツールバー
    QToolBar *urlBar;               // ツールバー
    QPushButton *lineNumberInfo;
    QLabel *charCodeInfo;
    QPushButton *codecInfo;
    QPushButton *newLineCodeInfo;
    QSpinBox *zoomInfo;
    QStringList fileHistory;
    QStringList dirHistory;

private:
    FindDialog *findDialog;
    FindDialog::FindParam lastSearch;
    ReplaceDialog *replaceDialog;
    GrepDialog *grepDialog;
    int markIndex;

    struct TagsJumpStack {
        QString filePath;
        int lineNumber;
    };

    QVector<TagsJumpStack> tagsJumpStacks;
};

#endif // MAINWINDOW_H
