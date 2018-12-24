#include <QApplication>
#include <QCoreApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QDir>
//#include <QtSingleApplication>
#include "mainwindow.h"

MainWindow *mainWindow;

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    qRegisterMetaType<TextEditor::TextFormat>("TextFormat");
    qRegisterMetaTypeStreamOperators<TextEditor::TextFormat>("TextFormat");
    qRegisterMetaType<TextEditor::KeywordData>("KeywordData");
    qRegisterMetaTypeStreamOperators<TextEditor::KeywordData>("KeywordData");
    qRegisterMetaType<TextEditor::BlockwordData>("BlockwordData");
    qRegisterMetaTypeStreamOperators<TextEditor::BlockwordData>("BlockwordData");
    qRegisterMetaType<TextEditor::ConfigType>("ConfigType");
    qRegisterMetaTypeStreamOperators<TextEditor::ConfigType>("ConfigType");

#if 0
    QtSingleApplication app(argc, argv);

    if (app.isRunning()) {
        app.sendMessage("Do I exist?");
        return 0;
    }
#else
    QApplication app(argc, argv);
#endif
    // プラグインディレクトリへのパス設定
    app.addLibraryPath(app.applicationDirPath() + QDir::separator() + "plugins");

    // 翻訳読込
    QString translations = app.applicationDirPath() + QDir::separator() + "translations";
    QTranslator qt_ja;
    qt_ja.load("qt_ja", translations);
    app.installTranslator(&qt_ja);

    MainWindow win;
    mainWindow = &win;
    win.show();
    /*
    QObject::connect(&app, SIGNAL(messageReceived(const QString&)),
                     &app, SLOT(activateWindow()));
                     */
    
    return app.exec();
}
