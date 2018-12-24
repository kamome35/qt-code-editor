#include <QtGui>
#include <QComboBox>
#include <QSignalMapper>
#include "configeditorpage.h"
#include "ui_configeditorpage.h"
#include "mainwindow.h"
#include "texteditor.h"

extern MainWindow *mainWindow;

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

ConfigEditorPage::ConfigEditorPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigEditorPage)
{
    ui->setupUi(this);
    highlightFormatMapper = new QSignalMapper(this);
    connect(highlightFormatMapper, SIGNAL(mapped(int)), this, SLOT(updateHighlightFormat(int)));
    findFormatMapper = new QSignalMapper(this);
    connect(findFormatMapper, SIGNAL(mapped(int)), this, SLOT(updateFindFormat(int)));

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");

    // ファイル種別設定値一覧取得
    editorTypes = settings.value("editorTypes").toList();
    foreach (const QVariant &config_type, editorTypes) {
        ui->editorType->addItem(config_type.value<TextEditor::ConfigType>().name, config_type);
    }

    QList<TextEditor *> list = mainWindow->textEditorList();
    foreach (TextEditor *textEdit, list) {
        QString text = textEdit->currentFile();
        ui->fileName->addItem(text);
    }

    int size = sizeof(codec_names) / sizeof(codec_names[0]);
    for (int i = 0; i < size; ++i) {
        ui->defTextCodecName->addItem(codec_names[i]);
    }

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    connect(ui->fileName, SIGNAL(currentIndexChanged(QString)), this, SLOT(currentEditorTypeChanged(QString)));
    connect(ui->editorType, SIGNAL(currentIndexChanged(int)), this, SLOT(editorTypeChanged(int)));
    connect(ui->editorType, SIGNAL(editTextChanged(QString)), this, SLOT(editorTypeNameChanged(QString)));
    connect(ui->suffixes, SIGNAL(textChanged(QString)), this, SLOT(suffixesChanged(QString)));
    connect(ui->addEditorType, SIGNAL(clicked()), this, SLOT(addEditorType()));
    connect(ui->removeEditorType, SIGNAL(clicked()), this, SLOT(removeEditorType()));
    connect(ui->fontFamily, SIGNAL(currentIndexChanged(QString)), this, SLOT(fontFamilyChanged(QString)));
    connect(ui->fontPointSizeF, SIGNAL(valueChanged(double)), this, SLOT(fontPointSizeFChanged(double)));
    connect(ui->zoom, SIGNAL(valueChanged(int)), this, SLOT(zoomChanged(int)));
    connect(ui->lineNumberVisible, SIGNAL(clicked(bool)), this, SLOT(lineNumberVisibleChanged(bool)));
    connect(ui->columnNumberVisible, SIGNAL(clicked(bool)), this, SLOT(columnNumberVisibleChanged(bool)));
    connect(ui->tabVisible, SIGNAL(clicked(bool)), this, SLOT(tabVisibleChanged(bool)));
    connect(ui->tabChar, SIGNAL(textChanged(QString)), this, SLOT(tabCharChanged(QString)));
    connect(ui->tabStopDigits, SIGNAL(valueChanged(int)), this, SLOT(tabStopDigitsChanged(int)));
    connect(ui->halfSpaceVisible, SIGNAL(clicked(bool)), this, SLOT(halfSpaceVisibleChanged(bool)));
    connect(ui->halfSpaceChar, SIGNAL(textChanged(QString)), this, SLOT(halfSpaceCharChanged(QString)));
    connect(ui->fullSpaceVisible, SIGNAL(clicked(bool)), this, SLOT(fullSpaceVisibleChanged(bool)));
    connect(ui->fullSpaceChar, SIGNAL(textChanged(QString)), this, SLOT(fullSpaceCharChanged(QString)));
    connect(ui->endOfLineVisible, SIGNAL(clicked(bool)), this, SLOT(endOfLineVisibleChanged(bool)));
    connect(ui->endOfLineChar, SIGNAL(textChanged(QString)), this, SLOT(endOfLineCharChanged(QString)));
    connect(ui->endOfFileVisible, SIGNAL(clicked(bool)), this, SLOT(endOfFileVisibleChanged(bool)));
    connect(ui->endOfFileChar, SIGNAL(textChanged(QString)), this, SLOT(endOfFileCharChanged(QString)));
    connect(ui->defTextCodecName, SIGNAL(currentIndexChanged(QString)), this, SLOT(defTextCodecNameChanged(QString)));
    connect(ui->textFormats, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(textFormatItemChanged(QListWidgetItem*)));
    connect(ui->textFormats, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(textFormatItemChanged(QListWidgetItem*)));
    connect(ui->textFormatEnabled, SIGNAL(toggled(bool)), this, SLOT(textFormatEnabledChanged()));
    connect(ui->textFormatName, SIGNAL(textChanged(QString)), this, SLOT(textFormatNameChanged()));
    connect(ui->textFormatForeground, SIGNAL(clicked()), this, SLOT(textFormatForegroundChanged()));
    connect(ui->textFormatBackground, SIGNAL(clicked()), this, SLOT(textFormatBackgroundChanged()));
    connect(ui->textFormatBold, SIGNAL(toggled(bool)), this, SLOT(toggleTextFormatBold(bool)));
    connect(ui->textFormatItalic, SIGNAL(toggled(bool)), this, SLOT(toggleTextFormatItalic(bool)));
    connect(ui->textFormatUnderline, SIGNAL(toggled(bool)), this, SLOT(toggleTextFormatUnderline(bool)));
    connect(ui->keywords, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(currentKeywordItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(ui->keywords, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(keywordChanged(QTreeWidgetItem*,int)));
    connect(ui->keywordFilterIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(keywordFilterChanged()));
    connect(ui->keywordFilter, SIGNAL(textChanged(QString)), this, SLOT(keywordFilterChanged()));
    connect(ui->addKeyword, SIGNAL(clicked()), this, SLOT(addKeyword()));
    connect(ui->removeKeyword, SIGNAL(clicked()), this, SLOT(removeKeyword()));
    connect(ui->keywordText, SIGNAL(textChanged(QString)), this, SLOT(keywordTextChanged(QString)));
    connect(ui->keywordCaseSensitive, SIGNAL(toggled(bool)), this, SLOT(toggleKeywordCaseSensitive(bool)));
    connect(ui->keywordWholeWords, SIGNAL(toggled(bool)), this, SLOT(toggleKeywordWholeWords(bool)));
    connect(ui->keywordRegularExpression, SIGNAL(toggled(bool)), this, SLOT(toggleKeywordRegularExpression(bool)));
    connect(ui->keywordHighlightFormats, SIGNAL(currentIndexChanged(int)), this, SLOT(changedKeywordHighlightFormats(int)));
    connect(ui->blockwords, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(currentBlockwordItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(ui->blockwords, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(blockwordChanged(QTreeWidgetItem*,int)));
    connect(ui->blockwordFilterIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(blockwordFilterChanged()));
    connect(ui->blockwordFilter, SIGNAL(textChanged(QString)), this, SLOT(blockwordFilterChanged()));
    connect(ui->addBlockword, SIGNAL(clicked()), this, SLOT(addBlockword()));
    connect(ui->removeBolockword, SIGNAL(clicked()), this, SLOT(removeBlockword()));
    connect(ui->blockKeywordBeginText, SIGNAL(textChanged(QString)), this, SLOT(blockwordBeginTextChanged(QString)));
    connect(ui->blockKeywordEndText, SIGNAL(textChanged(QString)), this, SLOT(blockwordEndTextChanged(QString)));
    connect(ui->blockKeywordCaseSensitive, SIGNAL(toggled(bool)), this, SLOT(toggleBlockwordCaseSensitive(bool)));
    connect(ui->blockKeywordWholeWords, SIGNAL(toggled(bool)), this, SLOT(toggleBlockwordWholeWords(bool)));
    connect(ui->blockKeywordRegularExpression, SIGNAL(toggled(bool)), this, SLOT(toggleBlockwordRegularExpression(bool)));
    connect(ui->blockKeywordHighlightFormats, SIGNAL(currentIndexChanged(int)), this, SLOT(blockwordHighlightFormatsChanged(int)));

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        int index = findEditorType(textEdit->configs().type.key);
        currentFileNameChanged(textEdit);
        editorTypeChanged(index);
        currentEditorTypeChanged(textEdit);
    } else {
        editorTypeChanged(0);
    }
}

ConfigEditorPage::~ConfigEditorPage()
{
    delete ui;
}

void ConfigEditorPage::tabChanged(int index)
{
    QWidget *widget = ui->tabWidget->widget(index);
    if (widget == ui->tabLayout) {
        updateLayout();
    } else if (widget == ui->tabLayout) {
        updateBehavior();
    } else if (widget == ui->tabTextFormats) {
        updatedTextFormats();
    } else if (widget == ui->tabKeywords) {
        updatedKeywords();
        updatedBlockKeywords();
    }
}

void ConfigEditorPage::currentFileNameChanged(TextEditor *textEdit)
{
    if (textEdit) {
        int index = ui->fileName->findText(textEdit->currentFile());
        ui->fileName->setCurrentIndex(index);
    }
}

int ConfigEditorPage::findEditorType(const QString &key)
{
    for (int i = 1; i < ui->editorType->count(); ++i) {
        const TextEditor::ConfigType &config_type = ui->editorType->itemData(i).value<TextEditor::ConfigType>();
        if (config_type.key == key) {
            return i;
        }
    }

    return 0;
}

void ConfigEditorPage::currentEditorTypeChanged(QString text)
{
    QMdiSubWindow *window = mainWindow->findMdiChild(text);
    mainWindow->setActiveSubWindow(window);
    TextEditor *textEdit = qobject_cast<TextEditor *>(window->widget());
    currentEditorTypeChanged(textEdit);
}

void ConfigEditorPage::currentEditorTypeChanged(TextEditor *textEdit)
{
    if (textEdit) {
        int index = findEditorType(textEdit->configs().type.key);
        ui->editorType->setCurrentIndex(index);
    }
}

void ConfigEditorPage::editorTypeChanged(int index)
{
    bool enabled = (bool)index;
    ui->editorType->lineEdit()->setEnabled(enabled);
    ui->suffixes->setEnabled(enabled);
    ui->removeEditorType->setEnabled(enabled);

    updateTextEditorConfig();
    tabChanged(ui->tabWidget->currentIndex());
    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->updateConfig(index);
    }
}

void ConfigEditorPage::editorTypeNameChanged(QString name)
{
    int index = ui->editorType->currentIndex();
    if (index <= 0)
        return;

    ui->editorType->setItemText(index, name);
    TextEditor::ConfigType config_type = ui->editorType->itemData(index).value<TextEditor::ConfigType>();
    config_type.name = name;
    ui->editorType->setItemData(index, QVariant::fromValue(config_type));

    updateEditorType();
}

void ConfigEditorPage::suffixesChanged(QString suffixes)
{
    int index = ui->editorType->currentIndex();
    if (index <= 0)
        return;

    TextEditor::ConfigType config_type = ui->editorType->itemData(index).value<TextEditor::ConfigType>();
    config_type.suffixes = suffixes;
    ui->editorType->setItemData(index, QVariant::fromValue(config_type));

    updateEditorType();
}

void ConfigEditorPage::addEditorType()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("ファイル種別追加"),
                                         tr("ファイル種別名:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        // key取得処理
        QString key;
        do {
            QString data = QDateTime::currentDateTime().toString("yyMdhmsz");
            key = QCryptographicHash::hash(data.toLatin1(), QCryptographicHash::Md5).toHex();
        } while (key.isNull());

        // MAPデータ追加
        TextEditor::ConfigType config_type;
        config_type.key = key;
        config_type.name = text;
        config_type.suffixes = "";
        qDebug() << config_type.key;

        ui->editorType->addItem(text, QVariant::fromValue(config_type));
        ui->editorType->setCurrentIndex(ui->editorType->count() - 1);

        updateEditorType();
    }
}

void ConfigEditorPage::removeEditorType()
{
    int index = ui->editorType->currentIndex();
    if (index <= 0)
        return;

    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, "", tr("'%1' を削除します。\n"
                                            "削除したデータは復元するこはできませんが、本当に削除しますか？").arg(ui->editorType->itemText(index)),
                               QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        ui->editorType->removeItem(index);

        updateEditorType();
    }
}

void ConfigEditorPage::fontFamilyChanged(QString family)
{
    configs.fontFamily = family;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("fontFamily", family);
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setFontFamily(family);
    }
}

void ConfigEditorPage::fontPointSizeFChanged(double sizeF)
{
    configs.fontPointSizeF = sizeF;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("fontPointSizeF", sizeF);
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setFontPointSizeF(sizeF);
    }
}

void ConfigEditorPage::zoomChanged(int zoom)
{
    configs.zoom = zoom / 100.0;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("zoom", configs.zoom);
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setZoom(configs.zoom);
    }
}

void ConfigEditorPage::lineNumberVisibleChanged(bool visible)
{
    configs.lineNumberFormat.enabled = visible;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("lineNumberFormat", QVariant::fromValue(configs.lineNumberFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setLineNumberVisible(visible);
    }
}

void ConfigEditorPage::columnNumberVisibleChanged(bool visible)
{
    configs.columnNumberFormat.enabled = visible;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("columnNumberFormat", QVariant::fromValue(configs.columnNumberFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setColumnNumberVisible(visible);
    }
}

void ConfigEditorPage::tabVisibleChanged(bool visible)
{
    configs.tabVisibleFormat.enabled = visible;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("tabVisibleFormat", QVariant::fromValue(configs.tabVisibleFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setTabVisible(visible);
    }
}

void ConfigEditorPage::tabCharChanged(const QString &text)
{
    configs.tabChar = text;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("tabChar", text);
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setTabChar(text);
    }
}

void ConfigEditorPage::tabStopDigitsChanged(int digits)
{
    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("tabStopDigits", digits);
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setTabStopDigits(digits);
    }
}

void ConfigEditorPage::halfSpaceVisibleChanged(bool visible)
{
    configs.halfSpaceVisibleFormat.enabled = visible;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("halfSpaceVisibleFormat", QVariant::fromValue(configs.halfSpaceVisibleFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setHalfSpaceVisible(visible);
    }
}

void ConfigEditorPage::halfSpaceCharChanged(const QString &text)
{
    configs.halfSpaceChar = text;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("halfSpaceChar", text);
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setHalfSpaceChar(text);
    }
}

void ConfigEditorPage::fullSpaceVisibleChanged(bool visible)
{
    configs.fullSpaceVisibleFormat.enabled = visible;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("fullSpaceVisibleFormat", QVariant::fromValue(configs.fullSpaceVisibleFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setFullSpaceVisible(visible);
    }
}

void ConfigEditorPage::fullSpaceCharChanged(const QString &text)
{
    configs.fullSpaceChar = text;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("fullSpaceChar", text);
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setFullSpaceChar(text);
    }
}

void ConfigEditorPage::endOfLineVisibleChanged(bool visible)
{
    configs.endOfLineFormat.enabled = visible;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("endOfLineFormat", QVariant::fromValue(configs.endOfLineFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setEndOfLineVisible(visible);
    }
}

void ConfigEditorPage::endOfLineCharChanged(const QString &text)
{
    configs.endOfLineChar = text;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("endOfLineChar", text);
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setEndOfLineChar(text);
    }
}

void ConfigEditorPage::endOfFileVisibleChanged(bool visible)
{
    configs.endOfFileFormat.enabled = visible;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("endOfFileFormat", QVariant::fromValue(configs.endOfFileFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setEndOfFileVisible(visible);
    }
}

void ConfigEditorPage::endOfFileCharChanged(const QString &text)
{
    configs.endOfFileChar = text;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("endOfFileChar", text);
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setEndOfFileChar(text);
    }
}

void ConfigEditorPage::defTextCodecNameChanged(QString textCodecName)
{
    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("defTextCodecName", textCodecName);
    settings.endGroup();
}

void ConfigEditorPage::textFormatItemChanged(QListWidgetItem *current_item)
{
    EditorFormatItem *item = dynamic_cast<EditorFormatItem *>(current_item);
    ui->textFormatItems->setEnabled(item != 0);

    if (item) {
        TextEditor::TextFormat textFormat = item->textFormat();
        ui->textFormats->setCurrentItem(item, QItemSelectionModel::SelectCurrent);
        ui->textFormatEnabled->setChecked(textFormat.enabled);
        ui->textFormatName->setText(textFormat.name);
        ui->textFormatForeground->setText(textFormat.foreground.name());
        ui->textFormatForeground->setPalette(textFormat.foreground);
        ui->textFormatForeground->setAutoFillBackground(true);
        ui->textFormatBackground->setText(textFormat.background.name());
        ui->textFormatBackground->setPalette(textFormat.background);
        ui->textFormatBackground->setAutoFillBackground(true);
        ui->textFormatBold->setChecked(textFormat.option.bold);
        ui->textFormatItalic->setChecked(textFormat.option.italic);
        ui->textFormatUnderline->setChecked(textFormat.option.underline);

        ui->textFormatEnabled->setVisible(item->visableFlags() & EditorFormatItem::ItemEnableVisable);
        ui->textFormatName->setVisible(item->visableFlags() & EditorFormatItem::ItemNameVisable);
        ui->textFormatForegroundLabel->setVisible(item->visableFlags() & EditorFormatItem::ItemForegroundVisable);
        ui->textFormatForeground->setVisible(item->visableFlags() & EditorFormatItem::ItemForegroundVisable);
        ui->textFormatBackgroundLabel->setVisible(item->visableFlags() & EditorFormatItem::ItemBackgroundVisable);
        ui->textFormatBackground->setVisible(item->visableFlags() & EditorFormatItem::ItemBackgroundVisable);
        ui->textFormatBold->setVisible(item->visableFlags() & EditorFormatItem::ItemBoldVisable);
        ui->textFormatItalic->setVisible(item->visableFlags() & EditorFormatItem::ItemItalicVisable);
        ui->textFormatUnderline->setVisible(item->visableFlags() & EditorFormatItem::ItemUnderlineVisable);

        ui->textFormatEnabled->setEnabled(item->enableItemFlags() & EditorFormatItem::ItemEnableEnable);
        ui->textFormatName->setEnabled(item->enableItemFlags() & EditorFormatItem::ItemNameEnable);
        ui->textFormatForegroundLabel->setEnabled(item->enableItemFlags() & EditorFormatItem::ItemForegroundEnable);
        ui->textFormatForeground->setEnabled(item->enableItemFlags() & EditorFormatItem::ItemForegroundEnable);
        ui->textFormatBackgroundLabel->setEnabled(item->enableItemFlags() & EditorFormatItem::ItemBackgroundEnable);
        ui->textFormatBackground->setEnabled(item->enableItemFlags() & EditorFormatItem::ItemBackgroundEnable);
        ui->textFormatBold->setEnabled(item->enableItemFlags() & EditorFormatItem::ItemBoldEnable);
        ui->textFormatItalic->setEnabled(item->enableItemFlags() & EditorFormatItem::ItemItalicEnable);
        ui->textFormatUnderline->setEnabled(item->enableItemFlags() & EditorFormatItem::ItemUnderlineEnable);
    }
}

void ConfigEditorPage::textFormatEnabledChanged()
{
    EditorFormatItem *item = dynamic_cast<EditorFormatItem *>(ui->textFormats->currentItem());
    if (!item) return;

    bool checked = ui->textFormatEnabled->isChecked();

    item->setChecked(checked);
    item->emitUpdateFormat();
}

void ConfigEditorPage::textFormatNameChanged()
{
    EditorFormatItem *item = dynamic_cast<EditorFormatItem *>(ui->textFormats->currentItem());
    if (!item) return;

    const QString &text = ui->textFormatName->text();

    item->setText(text);
    item->emitUpdateFormat();
}

void ConfigEditorPage::textFormatForegroundChanged()
{
    EditorFormatItem *item = dynamic_cast<EditorFormatItem *>(ui->textFormats->currentItem());
    if (!item) return;

    // カラーパレット表示
    QPalette p(ui->textFormatForeground->palette());
    QColor color = QColorDialog::getColor(p.color(QPalette::Background), this, tr("色選択"), QColorDialog::ShowAlphaChannel);
    if (!color.isValid()) return;

    ui->textFormatForeground->setText(color.name());
    ui->textFormatForeground->setPalette(QPalette(color));
    ui->textFormatForeground->setAutoFillBackground(true);

    item->setForeground(color);
    item->emitUpdateFormat();
}

void ConfigEditorPage::textFormatBackgroundChanged()
{
    EditorFormatItem *item = dynamic_cast<EditorFormatItem *>(ui->textFormats->currentItem());
    if (!item) return;

    // カラーパレット表示
    QPalette p(ui->textFormatBackground->palette());
    QColor color = QColorDialog::getColor(p.color(QPalette::Background), this, tr("色選択"), QColorDialog::ShowAlphaChannel);
    if (!color.isValid()) return;

    ui->textFormatBackground->setText(color.name());
    ui->textFormatBackground->setPalette(QPalette(color));
    ui->textFormatBackground->setAutoFillBackground(true);

    item->setBackground(color);
    item->emitUpdateFormat();
}

void ConfigEditorPage::toggleTextFormatBold(bool enabled)
{
    EditorFormatItem *item = dynamic_cast<EditorFormatItem *>(ui->textFormats->currentItem());
    if (!item) return;

    item->setTextFormatBold(enabled);
    item->emitUpdateFormat();
}

void ConfigEditorPage::toggleTextFormatItalic(bool enabled)
{
    EditorFormatItem *item = dynamic_cast<EditorFormatItem *>(ui->textFormats->currentItem());
    if (!item) return;

    item->setTextFormatItalic(enabled);
    item->emitUpdateFormat();
}

void ConfigEditorPage::toggleTextFormatUnderline(bool enabled)
{
    EditorFormatItem *item = dynamic_cast<EditorFormatItem *>(ui->textFormats->currentItem());
    if (!item) return;

    item->setTextFormatUnderline(enabled);
    item->emitUpdateFormat();
}

void ConfigEditorPage::currentKeywordItemChanged(QTreeWidgetItem *current, QTreeWidgetItem */*previous*/)
{
    ui->removeKeyword->setEnabled(current != 0);
    ui->keywordPage->setEnabled(current != 0);

    if (!current) return;

    ui->keywordText->setText(current->text(KeywordItemText));

    TextEditor::KeywordOption option = current->data(KeywordItemOption, Qt::UserRole).value<TextEditor::KeywordOption>();
    ui->keywordCaseSensitive->setChecked(option.caseSensitive);
    ui->keywordWholeWords->setChecked(option.wholeWords);
    ui->keywordRegularExpression->setChecked(option.regularExpression);

    int index = current->data(KeywordItemColor, Qt::UserRole).toInt();
    ui->keywordHighlightFormats->setCurrentIndex(index);
}

void ConfigEditorPage::keywordChanged(QTreeWidgetItem *item, int column)
{
    if (item) {
        QString key = item->data(KeywordItemText, Qt::UserRole).toString();
        TextEditor::KeywordData data = keywordMap[key].value<TextEditor::KeywordData>();
        switch (column) {
        case KeywordItemText:
            data.text = item->text(KeywordItemText);
            break;
        case KeywordItemOption:
            data.option = item->data(KeywordItemOption, Qt::UserRole).value<TextEditor::KeywordOption>();
            break;
        case KeywordItemColor:
            data.highlightIndex = item->data(KeywordItemColor, Qt::UserRole).toInt();
            break;
        }
        keywordMap[key] = QVariant::fromValue(data);
    }
    updateConfigKeyword();
}

void ConfigEditorPage::addKeyword()
{
    TextEditor::KeywordData data;

    data.text = tr("新しいアイテム");
    data.option.caseSensitive = ui->keywordCaseSensitive->isChecked();
    data.option.wholeWords = ui->keywordWholeWords->isChecked();
    data.option.regularExpression = ui->keywordRegularExpression->isChecked();
    data.highlightIndex = ui->keywordHighlightFormats->currentIndex();

    QTreeWidgetItem *item = createKeyword(data);
    keywordChanged(0, 0);

    ui->keywords->addTopLevelItem(item);
    ui->keywords->setCurrentItem(item, QItemSelectionModel::SelectCurrent);
    ui->keywordText->setFocus();
    ui->keywordText->selectAll();
}

void ConfigEditorPage::removeKeyword()
{
    QTreeWidgetItem *item = ui->keywords->currentItem();

    if (item) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, "", tr("'%1' を削除します。\n"
                                                "本当に削除しますか？").arg(item->text(KeywordItemText)),
                                   QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            QString key = item->data(KeywordItemText, Qt::UserRole).toString();
            keywordMap.remove(key);
            keywordChanged(0, 0);
            delete item;
        }
    }
}

void ConfigEditorPage::keywordTextChanged(QString text)
{
    QTreeWidgetItem *item = ui->keywords->currentItem();
    if (!item) return;
    item->setText(KeywordItemText, text);
}

void ConfigEditorPage::toggleKeywordCaseSensitive(bool enabled)
{
    QTreeWidgetItem *item = ui->keywords->currentItem();
    if (!item) return;

    TextEditor::KeywordOption option = item->data(KeywordItemOption, Qt::UserRole).value<TextEditor::KeywordOption>();
    option.caseSensitive = enabled;
    setKeywordOption(item, option);
}

void ConfigEditorPage::toggleKeywordWholeWords(bool enabled)
{
    QTreeWidgetItem *item = ui->keywords->currentItem();
    if (!item) return;

    TextEditor::KeywordOption option = item->data(KeywordItemOption, Qt::UserRole).value<TextEditor::KeywordOption>();
    option.wholeWords = enabled;
    setKeywordOption(item, option);
}

void ConfigEditorPage::toggleKeywordRegularExpression(bool enabled)
{
    QTreeWidgetItem *item = ui->keywords->currentItem();
    if (!item) return;

    TextEditor::KeywordOption option = item->data(KeywordItemOption, Qt::UserRole).value<TextEditor::KeywordOption>();
    option.regularExpression = enabled;
    setKeywordOption(item, option);
}

void ConfigEditorPage::changedKeywordHighlightFormats(int index)
{
    if ((unsigned int)configs.highlightFormats.size() < (unsigned int)index) return;
    const TextEditor::TextFormat &format = configs.highlightFormats[index].value<TextEditor::TextFormat>();

    QTreeWidgetItem *item = ui->keywords->currentItem();
    if (!item) return;
    item->setText(KeywordItemColor, format.name);
    item->setData(KeywordItemColor, Qt::ForegroundRole, format.foreground);
    item->setData(KeywordItemColor, Qt::BackgroundRole, format.background);
    item->setData(KeywordItemColor, Qt::UserRole, index);
}

void ConfigEditorPage::currentBlockwordItemChanged(QTreeWidgetItem *current, QTreeWidgetItem */*previous*/)
{
    ui->removeBolockword->setEnabled(current != 0);
    ui->blockwordPage->setEnabled(current != 0);

    if (!current) return;

    ui->blockKeywordBeginText->setText(current->text(BlockwordItemBeginText));
    ui->blockKeywordEndText->setText(current->text(BlockwordItemEndText));

    TextEditor::KeywordOption option = current->data(BlockwordItemOption, Qt::UserRole).value<TextEditor::KeywordOption>();
    ui->blockKeywordCaseSensitive->setChecked(option.caseSensitive);
    ui->blockKeywordWholeWords->setChecked(option.wholeWords);
    ui->blockKeywordRegularExpression->setChecked(option.regularExpression);

    int index = current->data(BlockwordItemColor, Qt::UserRole).toInt();
    ui->blockKeywordHighlightFormats->setCurrentIndex(index);
}

void ConfigEditorPage::blockwordChanged(QTreeWidgetItem *item, int column)
{
    if (item) {
        QString key = item->data(BlockwordItemBeginText, Qt::UserRole).toString();
        TextEditor::BlockwordData data = blokwordMap[key].value<TextEditor::BlockwordData>();
        switch (column) {
        case BlockwordItemBeginText:
            data.beginText = item->text(BlockwordItemBeginText);
            break;
        case BlockwordItemEndText:
            data.endText = item->text(BlockwordItemEndText);
            break;
        case BlockwordItemOption:
            data.option = item->data(BlockwordItemOption, Qt::UserRole).value<TextEditor::KeywordOption>();
            break;
        case BlockwordItemColor:
            data.highlightIndex = item->data(BlockwordItemColor, Qt::UserRole).toInt();
            break;
        }
        blokwordMap[key] = QVariant::fromValue(data);
    }
    updateConfigBlockkeyword();
}

void ConfigEditorPage::addBlockword()
{
    TextEditor::BlockwordData data;

    data.beginText = tr("新しいアイテム");
    data.endText = "";
    data.option.caseSensitive = ui->keywordCaseSensitive->isChecked();
    data.option.wholeWords = ui->keywordWholeWords->isChecked();
    data.option.regularExpression = ui->keywordRegularExpression->isChecked();
    data.highlightIndex = ui->keywordHighlightFormats->currentIndex();

    QTreeWidgetItem *item = createBlockword(data);
    blockwordChanged(0, 0);

    ui->blockwords->addTopLevelItem(item);
    ui->blockwords->setCurrentItem(item, QItemSelectionModel::SelectCurrent);
    ui->blockKeywordBeginText->setFocus();
    ui->blockKeywordBeginText->selectAll();
}

void ConfigEditorPage::removeBlockword()
{
    QTreeWidgetItem *item = ui->blockwords->currentItem();
    if (item) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, "", tr("'%1'-'%2' を削除します。\n"
                                                "本当に削除しますか？").arg(item->text(BlockwordItemBeginText)).arg(item->text(BlockwordItemEndText)),
                                   QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            QString key = item->data(BlockwordItemBeginText, Qt::UserRole).toString();
            blokwordMap.remove(key);
            blockwordChanged(0, 0);
            delete item;
        }
    }
}

void ConfigEditorPage::blockwordBeginTextChanged(QString text)
{
    QTreeWidgetItem *item = ui->blockwords->currentItem();
    if (!item) return;
    item->setText(BlockwordItemBeginText, text);
}

void ConfigEditorPage::blockwordEndTextChanged(QString text)
{
    QTreeWidgetItem *item = ui->blockwords->currentItem();
    if (!item) return;
    item->setText(BlockwordItemEndText, text);
}

void ConfigEditorPage::toggleBlockwordCaseSensitive(bool enabled)
{
    QTreeWidgetItem *item = ui->blockwords->currentItem();
    if (!item) return;

    TextEditor::KeywordOption option = item->data(BlockwordItemOption, Qt::UserRole).value<TextEditor::KeywordOption>();
    option.caseSensitive = enabled;
    setBlockwordOption(item, option);
}

void ConfigEditorPage::toggleBlockwordWholeWords(bool enabled)
{
    QTreeWidgetItem *item = ui->blockwords->currentItem();
    if (!item) return;

    TextEditor::KeywordOption option = item->data(BlockwordItemOption, Qt::UserRole).value<TextEditor::KeywordOption>();
    option.wholeWords = enabled;
    setBlockwordOption(item, option);
}

void ConfigEditorPage::toggleBlockwordRegularExpression(bool enabled)
{
    QTreeWidgetItem *item = ui->blockwords->currentItem();
    if (!item) return;

    TextEditor::KeywordOption option = item->data(BlockwordItemOption, Qt::UserRole).value<TextEditor::KeywordOption>();
    option.regularExpression = enabled;
    setBlockwordOption(item, option);
}

void ConfigEditorPage::blockwordHighlightFormatsChanged(int index)
{
    if ((unsigned int)configs.highlightFormats.size() < (unsigned int)index) return;
    const TextEditor::TextFormat &format = configs.highlightFormats[index].value<TextEditor::TextFormat>();

    QTreeWidgetItem *item = ui->blockwords->currentItem();
    if (!item) return;
    item->setText(BlockwordItemColor, format.name);
    item->setData(BlockwordItemColor, Qt::ForegroundRole, format.foreground);
    item->setData(BlockwordItemColor, Qt::BackgroundRole, format.background);
    item->setData(BlockwordItemColor, Qt::UserRole, index);
}

void ConfigEditorPage::updateBasicFormat()
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.basicFormat = textFormat;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("basicFormat", QVariant::fromValue(textFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setBasicFormat(textFormat);
    }
}

void ConfigEditorPage::updateStripeFormat()
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.stripeFormat = textFormat;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("stripeFormat", QVariant::fromValue(textFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setStripeFormat(textFormat);
    }
}

void ConfigEditorPage::updateLineNumberFormat()
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.lineNumberFormat = textFormat;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("lineNumberFormat", QVariant::fromValue(textFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setLineNumberFormat(textFormat);
    }
}

void ConfigEditorPage::updateLineNumberCurrentFormat()
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.lineNumberCurrentFormat = textFormat;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("lineNumberCurrentFormat", QVariant::fromValue(textFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setLineNumberCurrentFormat(textFormat);
    }
}

void ConfigEditorPage::updateColumnNumberFormat()
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.columnNumberFormat = textFormat;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("columnNumberFormat", QVariant::fromValue(textFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setColumnNumberFormat(textFormat);
    }
}

void ConfigEditorPage::updateColumnNumberCurrentFormat()
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.columnNumberCurrentFormat = textFormat;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("columnNumberCurrentFormat", QVariant::fromValue(textFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setColumnNumberCurrentFormat(textFormat);
    }
}

void ConfigEditorPage::updateHighlightFormat(const int index)
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.highlightFormats[index] = QVariant::fromValue(textFormat);

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("highlightFormats", configs.highlightFormats);
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setHighlightFormat(index, textFormat);
    }
}

void ConfigEditorPage::updateFindFormat(const int index)
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.findFormats[index] = QVariant::fromValue(textFormat);

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("findFormats", configs.findFormats);
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setFindFormat(index, textFormat);
    }
}

void ConfigEditorPage::updateHalfSpaceVisibleFormat()
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.halfSpaceVisibleFormat = textFormat;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("halfSpaceVisibleFormat", QVariant::fromValue(textFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setHalfSpaceVisibleFormat(textFormat);
    }
}

void ConfigEditorPage::updateFullSpaceVisibleFormat()
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.fullSpaceVisibleFormat = textFormat;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("fullSpaceVisibleFormat", QVariant::fromValue(textFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setFullSpaceVisibleFormat(textFormat);
    }
}

void ConfigEditorPage::updateTabVisibleFormat()
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.tabVisibleFormat = textFormat;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("tabVisibleFormat", QVariant::fromValue(textFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setTabVisibleFormat(textFormat);
    }
}

void ConfigEditorPage::updateEndOfLineFormat()
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.endOfLineFormat = textFormat;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("endOfLineFormat", QVariant::fromValue(textFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setEndOfLineFormat(textFormat);
    }
}

void ConfigEditorPage::updateEndOfFileFormat()
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.endOfFileFormat = textFormat;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("endOfFileFormat", QVariant::fromValue(textFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setEndOfFileFormat(textFormat);
    }
}

void ConfigEditorPage::updateCurrentLineFormat()
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.currentLineFormat = textFormat;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("currentLineFormat", QVariant::fromValue(textFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setCurrentLineFormat(textFormat);
    }
}

void ConfigEditorPage::updateCurrentColumnFormat()
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.currentColumnFormat = textFormat;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("currentColumnFormat", QVariant::fromValue(textFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setCurrentColumnFormat(textFormat);
    }
}

void ConfigEditorPage::updateSelectionFormat()
{
    const TextEditor::TextFormat &textFormat = currentFormat();
    configs.selectionFormat = textFormat;

    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("selectionFormat", QVariant::fromValue(textFormat));
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setSelectionFormat(textFormat);
    }
}

void ConfigEditorPage::keywordFilterChanged()
{
    const QString &text = ui->keywordFilter->text();
    int index = ui->keywordFilterIndex->currentIndex();
    QTreeWidgetItemIterator it(ui->keywords);
    while (*it)
    {
        QTreeWidgetItem *item = *it;
        if (item->text(index).indexOf(text) == -1)
            item->setHidden(true);
        else
            item->setHidden(false);
        ++it;
    }
}

void ConfigEditorPage::blockwordFilterChanged()
{
    const QString &text = ui->blockwordFilter->text();
    int index = ui->blockwordFilterIndex->currentIndex();
    QTreeWidgetItemIterator it(ui->blockwords);
    while (*it)
    {
        QTreeWidgetItem *item = *it;
        if (item->text(index).indexOf(text) == -1)
            item->setHidden(true);
        else
            item->setHidden(false);
        ++it;
    }
}

void ConfigEditorPage::updateEditorType()
{
    QList<QVariant> editorTypes;
    for (int i = 1; i < ui->editorType->count(); ++i) {
        editorTypes << ui->editorType->itemData(i);
    }

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.setValue("editorTypes", editorTypes);
}

void ConfigEditorPage::updateConfigKeyword()
{
    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("keywords", keywordMap.values());
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setKeywords(keywordMap.values());
    }
}

void ConfigEditorPage::updateConfigBlockkeyword()
{
    const TextEditor::ConfigType &config_type = currentConfigType();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MyEditor", "Editor");
    settings.beginGroup(config_type.key);
    settings.setValue("blockwords", blokwordMap.values());
    settings.endGroup();

    TextEditor *textEdit = currentTextEditor();
    if (textEdit) {
        textEdit->setBlockwords(blokwordMap.values());
    }
}

void ConfigEditorPage::setKeywordOption(QTreeWidgetItem *item, const TextEditor::KeywordOption &option)
{
    QStringList options;
    if (option.caseSensitive)     options << tr("区");
    if (option.wholeWords)        options << tr("単");
    if (option.regularExpression) options << tr("正");
    options.sort();
    item->setText(KeywordItemOption, options.join("/"));
    item->setData(KeywordItemOption, Qt::UserRole, QVariant::fromValue(option));
}

void ConfigEditorPage::setBlockwordOption(QTreeWidgetItem *item, const TextEditor::KeywordOption &option)
{
    QStringList options;
    if (option.caseSensitive)     options << tr("区");
    if (option.wholeWords)        options << tr("単");
    if (option.regularExpression) options << tr("正");
    options.sort();
    item->setText(BlockwordItemOption, options.join("/"));
    item->setData(BlockwordItemOption, Qt::UserRole, QVariant::fromValue(option));
}

QTreeWidgetItem *ConfigEditorPage::createKeyword(const TextEditor::KeywordData &data)
{
    QString tmp = QDateTime::currentDateTime().toString("yyMdhmsz") + data.text;
    QString key = QCryptographicHash::hash(tmp.toLatin1(), QCryptographicHash::Md5).toHex();
    keywordMap.insert(key, QVariant::fromValue(data));

    QTreeWidgetItem *item = new QTreeWidgetItem();

    // キーワード
    item->setText(KeywordItemText, data.text);
    item->setData(KeywordItemText, Qt::UserRole, key);

    // オプション
    setKeywordOption(item, data.option);

    const TextEditor::TextFormat &format = configs.highlightFormats[data.highlightIndex].value<TextEditor::TextFormat>();
    item->setText(KeywordItemColor, format.name);
    item->setData(KeywordItemColor, Qt::ForegroundRole, format.foreground);
    item->setData(KeywordItemColor, Qt::BackgroundRole, format.background);
    item->setData(KeywordItemColor, Qt::UserRole, data.highlightIndex);

    return item;
}

QTreeWidgetItem *ConfigEditorPage::createBlockword(const TextEditor::BlockwordData &data)
{
    QString tmp = QDateTime::currentDateTime().toString("yyMdhmsz") + data.beginText + data.endText;
    QString key = QCryptographicHash::hash(tmp.toLatin1(), QCryptographicHash::Md5).toHex();
    blokwordMap[key] = QVariant::fromValue(data);

    QTreeWidgetItem *item = new QTreeWidgetItem();

    // キーワード
    item->setText(BlockwordItemBeginText, data.beginText);
    item->setData(BlockwordItemBeginText, Qt::UserRole, key);
    item->setText(BlockwordItemEndText, data.endText);

    // オプション
    setBlockwordOption(item, data.option);

    const TextEditor::TextFormat &format = configs.highlightFormats[data.highlightIndex].value<TextEditor::TextFormat>();
    item->setText(BlockwordItemColor, format.name);
    item->setData(BlockwordItemColor, Qt::ForegroundRole, format.foreground);
    item->setData(BlockwordItemColor, Qt::BackgroundRole, format.background);
    item->setData(BlockwordItemColor, Qt::UserRole, data.highlightIndex);

    return item;
}

void ConfigEditorPage::updateTextEditorConfig()
{
    const TextEditor::ConfigType &config_type = currentConfigType();

    qDebug() << config_type.key;
    configs = TextEditor::configs(config_type.key);

    ui->suffixes->setText(config_type.suffixes);
}

void ConfigEditorPage::updateLayout()
{
    ui->fontFamily->setCurrentFont(configs.fontFamily);
    ui->fontPointSizeF->setValue(configs.fontPointSizeF);
    ui->zoom->setValue(configs.zoom * 100.0);
    ui->lineNumberVisible->setChecked(configs.lineNumberFormat.enabled);
    ui->columnNumberVisible->setChecked(configs.columnNumberFormat.enabled);
    ui->tabVisible->setChecked(configs.tabVisibleFormat.enabled);
    ui->tabChar->setText(configs.tabChar);
    ui->tabStopDigits->setValue(configs.tabStopDigits);
    ui->halfSpaceVisible->setChecked(configs.halfSpaceVisibleFormat.enabled);
    ui->halfSpaceChar->setText(configs.halfSpaceChar);
    ui->fullSpaceVisible->setChecked(configs.fullSpaceVisibleFormat.enabled);
    ui->fullSpaceChar->setText(configs.fullSpaceChar);
    ui->endOfLineVisible->setChecked(configs.endOfLineFormat.enabled);
    ui->endOfLineChar->setText(configs.endOfLineChar);
    ui->endOfFileVisible->setChecked(configs.endOfFileFormat.enabled);
    ui->endOfFileChar->setText(configs.endOfFileChar);
}

void ConfigEditorPage::updateBehavior()
{
    ui->defTextCodecName->setCurrentIndex(ui->defTextCodecName->findText(configs.defTextCodecName));
}

void ConfigEditorPage::updatedTextFormats()
{
    // スキームアイテムｸﾘｱ
    ui->textFormats->clear();
    QFont f = ui->textFormats->font();
    f.setFamily(configs.fontFamily);
    //f.setPointSizeF(configs.fontPointSizeF);
    ui->textFormats->setFont(f);

    // 基本
    configs.basicFormat.name = tr("テキスト");
    EditorFormatItem *basicFormat = new EditorFormatItem();
    basicFormat->setTextFormat(configs.basicFormat);
    basicFormat->setFlags(basicFormat->flags() ^ Qt::ItemIsUserCheckable);
    basicFormat->setVisableFlags(basicFormat->visableFlags() ^ EditorFormatItem::ItemFontStyleVisable);
    basicFormat->setEnableItemFlags(basicFormat->enableItemFlags() ^ EditorFormatItem::ItemNameEnable ^ EditorFormatItem::ItemEnableEnable);
    connect(basicFormat, SIGNAL(updatedFormat()), this, SLOT(updateBasicFormat()));
    ui->textFormats->addItem(basicFormat);

    // ストライプ
    configs.stripeFormat.name = tr("ストライプ");
    EditorFormatItem *stripeFormat = new EditorFormatItem();
    stripeFormat->setTextFormat(configs.stripeFormat);
    stripeFormat->setVisableFlags(stripeFormat->visableFlags() ^ EditorFormatItem::ItemForegroundVisable ^ EditorFormatItem::ItemFontStyleVisable);
    stripeFormat->setEnableItemFlags(stripeFormat->enableItemFlags() ^ EditorFormatItem::ItemNameEnable);
    connect(stripeFormat, SIGNAL(updatedFormat()), this, SLOT(updateStripeFormat()));
    ui->textFormats->addItem(stripeFormat);

    // 行番号
    configs.lineNumberFormat.name = tr("行番号");
    EditorFormatItem *lineNumberFormat = new EditorFormatItem();
    lineNumberFormat->setTextFormat(configs.lineNumberFormat);
    lineNumberFormat->setEnableItemFlags(lineNumberFormat->enableItemFlags() ^ EditorFormatItem::ItemNameEnable);
    connect(lineNumberFormat, SIGNAL(updatedFormat()), this, SLOT(updateLineNumberFormat()));
    ui->textFormats->addItem(lineNumberFormat);

    // 現在の行
    configs.lineNumberCurrentFormat.name = tr("現在の行");
    EditorFormatItem *lineNumberCurrentFormat = new EditorFormatItem();
    lineNumberCurrentFormat->setTextFormat(configs.lineNumberCurrentFormat);
    lineNumberCurrentFormat->setEnableItemFlags(lineNumberCurrentFormat->enableItemFlags() ^ EditorFormatItem::ItemNameEnable);
    connect(lineNumberCurrentFormat, SIGNAL(updatedFormat()), this, SLOT(updateLineNumberCurrentFormat()));
    ui->textFormats->addItem(lineNumberCurrentFormat);

    // 列番号
    configs.columnNumberFormat.name = tr("列番号");
    EditorFormatItem *columnNumberFormat = new EditorFormatItem();
    columnNumberFormat->setTextFormat(configs.columnNumberFormat);
    columnNumberFormat->setEnableItemFlags(columnNumberFormat->enableItemFlags() ^ EditorFormatItem::ItemNameEnable);
    connect(columnNumberFormat, SIGNAL(updatedFormat()), this, SLOT(updateColumnNumberFormat()));
    ui->textFormats->addItem(columnNumberFormat);

    // 現在の列
    configs.columnNumberCurrentFormat.name = tr("現在の列");
    EditorFormatItem *columnNumberCurrentFormat = new EditorFormatItem();
    columnNumberCurrentFormat->setTextFormat(configs.columnNumberCurrentFormat);
    columnNumberCurrentFormat->setVisableFlags(columnNumberCurrentFormat->visableFlags() ^ EditorFormatItem::ItemForegroundVisable ^ EditorFormatItem::ItemFontStyleVisable);
    columnNumberCurrentFormat->setEnableItemFlags(columnNumberCurrentFormat->enableItemFlags() ^ EditorFormatItem::ItemNameEnable);
    connect(columnNumberCurrentFormat, SIGNAL(updatedFormat()), this, SLOT(updateColumnNumberCurrentFormat()));
    ui->textFormats->addItem(columnNumberCurrentFormat);

    // 強調文字列
    for (int i = 0; i < configs.highlightFormats.size(); ++i) {
        EditorFormatItem *highlightFormat = new EditorFormatItem();
        highlightFormat->setTextFormat(configs.highlightFormats[i].value<TextEditor::TextFormat>());
        connect(highlightFormat, SIGNAL(updatedFormat()), highlightFormatMapper, SLOT(map()));
        highlightFormatMapper->setMapping(highlightFormat, i);
        ui->textFormats->addItem(highlightFormat);
    }

    // 検索文字列
    for (int i = 0; i < configs.findFormats.size(); ++i) {
        EditorFormatItem *findFormat = new EditorFormatItem();
        findFormat->setTextFormat(configs.findFormats[i].value<TextEditor::TextFormat>());
        connect(findFormat, SIGNAL(updatedFormat()), findFormatMapper, SLOT(map()));
        findFormatMapper->setMapping(findFormat, i);
        ui->textFormats->addItem(findFormat);
    }

    // 半角空白
    configs.halfSpaceVisibleFormat.name = tr("半角空白");
    EditorFormatItem *halfSpaceVisibleFormat = new EditorFormatItem();
    halfSpaceVisibleFormat->setTextFormat(configs.halfSpaceVisibleFormat);
    halfSpaceVisibleFormat->setVisableFlags(halfSpaceVisibleFormat->visableFlags() ^ EditorFormatItem::ItemFontStyleVisable);
    halfSpaceVisibleFormat->setEnableItemFlags(halfSpaceVisibleFormat->enableItemFlags() ^ EditorFormatItem::ItemNameEnable);
    connect(halfSpaceVisibleFormat, SIGNAL(updatedFormat()), this, SLOT(updateHalfSpaceVisibleFormat()));
    ui->textFormats->addItem(halfSpaceVisibleFormat);

    // 全角空白
    configs.fullSpaceVisibleFormat.name = tr("全角空白");
    EditorFormatItem *fullSpaceVisibleFormat = new EditorFormatItem();
    fullSpaceVisibleFormat->setTextFormat(configs.fullSpaceVisibleFormat);
    fullSpaceVisibleFormat->setVisableFlags(fullSpaceVisibleFormat->visableFlags() ^ EditorFormatItem::ItemFontStyleVisable);
    fullSpaceVisibleFormat->setEnableItemFlags(fullSpaceVisibleFormat->enableItemFlags() ^ EditorFormatItem::ItemNameEnable);
    connect(fullSpaceVisibleFormat, SIGNAL(updatedFormat()), this, SLOT(updateFullSpaceVisibleFormat()));
    ui->textFormats->addItem(fullSpaceVisibleFormat);

    // タブ
    configs.tabVisibleFormat.name = tr("タブ");
    EditorFormatItem *tabVisibleFormat = new EditorFormatItem();
    tabVisibleFormat->setTextFormat(configs.tabVisibleFormat);
    tabVisibleFormat->setVisableFlags(tabVisibleFormat->visableFlags() ^ EditorFormatItem::ItemFontStyleVisable);
    tabVisibleFormat->setEnableItemFlags(tabVisibleFormat->enableItemFlags() ^ EditorFormatItem::ItemNameEnable);
    connect(tabVisibleFormat, SIGNAL(updatedFormat()), this, SLOT(updateTabVisibleFormat()));
    ui->textFormats->addItem(tabVisibleFormat);

    // 改行コード
    configs.endOfLineFormat.name = tr("改行コード");
    EditorFormatItem *endOfLineFormat = new EditorFormatItem();
    endOfLineFormat->setTextFormat(configs.endOfLineFormat);
    endOfLineFormat->setVisableFlags(endOfLineFormat->visableFlags() ^ EditorFormatItem::ItemFontStyleVisable);
    endOfLineFormat->setEnableItemFlags(endOfLineFormat->enableItemFlags() ^ EditorFormatItem::ItemNameEnable);
    connect(endOfLineFormat, SIGNAL(updatedFormat()), this, SLOT(updateEndOfLineFormat()));
    ui->textFormats->addItem(endOfLineFormat);

    // EOF
    configs.endOfFileFormat.name = tr("EOF");
    EditorFormatItem *endOfFileFormat = new EditorFormatItem();
    endOfFileFormat->setTextFormat(configs.endOfFileFormat);
    endOfFileFormat->setVisableFlags(endOfFileFormat->visableFlags() ^ EditorFormatItem::ItemFontStyleVisable);
    endOfFileFormat->setEnableItemFlags(endOfFileFormat->enableItemFlags() ^ EditorFormatItem::ItemNameEnable);
    connect(endOfFileFormat, SIGNAL(updatedFormat()), this, SLOT(updateEndOfFileFormat()));
    ui->textFormats->addItem(endOfFileFormat);

    // 選択範囲
    configs.selectionFormat.name = tr("選択範囲");
    EditorFormatItem *selectionFormat = new EditorFormatItem();
    selectionFormat->setTextFormat(configs.selectionFormat);
    selectionFormat->setFlags(selectionFormat->flags() ^ Qt::ItemIsUserCheckable);
    selectionFormat->setVisableFlags(selectionFormat->visableFlags() ^ EditorFormatItem::ItemFontStyleVisable);
    selectionFormat->setEnableItemFlags(selectionFormat->enableItemFlags() ^ EditorFormatItem::ItemNameEnable ^ EditorFormatItem::ItemEnableEnable);
    connect(selectionFormat, SIGNAL(updatedFormat()), this, SLOT(updateSelectionFormat()));
    ui->textFormats->addItem(selectionFormat);

    // 現在の行
    configs.currentLineFormat.name = tr("現在の行");
    EditorFormatItem *currentLineFormat = new EditorFormatItem();
    currentLineFormat->setTextFormat(configs.currentLineFormat);
    currentLineFormat->setVisableFlags(currentLineFormat->visableFlags() ^ EditorFormatItem::ItemFontStyleVisable);
    currentLineFormat->setEnableItemFlags(currentLineFormat->enableItemFlags() ^ EditorFormatItem::ItemNameEnable);
    connect(currentLineFormat, SIGNAL(updatedFormat()), this, SLOT(updateCurrentLineFormat()));
    ui->textFormats->addItem(currentLineFormat);

    // 現在の列
    configs.currentColumnFormat.name = tr("現在の列");
    EditorFormatItem *currentColumnFormat = new EditorFormatItem();
    currentColumnFormat->setTextFormat(configs.currentColumnFormat);
    currentColumnFormat->setVisableFlags(currentColumnFormat->visableFlags() ^ EditorFormatItem::ItemForegroundVisable ^ EditorFormatItem::ItemFontStyleVisable);
    currentColumnFormat->setEnableItemFlags(currentColumnFormat->enableItemFlags() ^ EditorFormatItem::ItemNameEnable);
    connect(currentColumnFormat, SIGNAL(updatedFormat()), this, SLOT(updateCurrentColumnFormat()));
    ui->textFormats->addItem(currentColumnFormat);

    textFormatItemChanged(0);
}

void ConfigEditorPage::updatedKeywords()
{
    keywordMap.clear();
    ui->keywords->clear();
    for (int i = 0; i < configs.keywords.size(); ++i) {
        const TextEditor::KeywordData &data = configs.keywords[i].value<TextEditor::KeywordData>();
        QTreeWidgetItem *item = createKeyword(data);
        ui->keywords->addTopLevelItem(item);
    }

    ui->keywordHighlightFormats->clear();
    for (int i = 0; i < configs.highlightFormats.size(); ++i) {
        const TextEditor::TextFormat &format = configs.highlightFormats[i].value<TextEditor::TextFormat>();
        ui->keywordHighlightFormats->insertItem(i, format.name);
        ui->keywordHighlightFormats->setItemData(i, QBrush(format.foreground), Qt::ForegroundRole);
        ui->keywordHighlightFormats->setItemData(i, QBrush(format.background), Qt::BackgroundRole);
    }

    currentKeywordItemChanged(0, 0);
}

void ConfigEditorPage::updatedBlockKeywords()
{
    blokwordMap.clear();
    ui->blockwords->clear();
    for (int i = 0; i < configs.blockwords.size(); ++i) {
        const TextEditor::BlockwordData &data = configs.blockwords[i].value<TextEditor::BlockwordData>();
        QTreeWidgetItem *item = createBlockword(data);
        ui->blockwords->addTopLevelItem(item);
    }

    ui->blockKeywordHighlightFormats->clear();
    for (int i = 0; i < configs.highlightFormats.size(); ++i) {
        const TextEditor::TextFormat &format = configs.highlightFormats[i].value<TextEditor::TextFormat>();
        ui->blockKeywordHighlightFormats->insertItem(i, format.name);
        ui->blockKeywordHighlightFormats->setItemData(i, QBrush(format.foreground), Qt::ForegroundRole);
        ui->blockKeywordHighlightFormats->setItemData(i, QBrush(format.background), Qt::BackgroundRole);
    }

    currentBlockwordItemChanged(0, 0);
}

TextEditor::ConfigType ConfigEditorPage::currentConfigType() const
{
    const int &index = ui->editorType->currentIndex();
    if (index <= 0) {
        TextEditor::ConfigType config_type;
        config_type.key = "default";
        return config_type;
    }

    const TextEditor::ConfigType &config_type = ui->editorType->itemData(index).value<TextEditor::ConfigType>();
    return config_type;
}

TextEditor::TextFormat ConfigEditorPage::currentFormat() const
{
    TextEditor::TextFormat format;
    EditorFormatItem *current = dynamic_cast<EditorFormatItem *>(ui->textFormats->currentItem());
    if (current)
        format = current->textFormat();

    return format;
}

TextEditor *ConfigEditorPage::currentTextEditor() const
{
    return mainWindow->activeMdiChild();
}
