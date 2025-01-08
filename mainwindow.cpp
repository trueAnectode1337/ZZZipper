#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <archive.h>
#include <archive_entry.h>
#include <QPushButton>
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <QFileIconProvider>
#include <QMenu>
#include <QAction>
#include <QStandardItem>
#include <QFileInfo>
#include <QHeaderView>
#include <QMap>
#include <QFileInfo>
#include <QDir>
#include <QHeaderView>
#include <QMap>
#include <QStorageInfo>
#include <QIcon>
#include <QMimeData>
#include <QDrag>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QVBoxLayout>
#include <QDebug>
#include <QFile>
#include <QUrl>
#include <QDesktopServices>
#include <QStack>
#include <QDirIterator>
#include "archivedialog.h"
#include <QGuiApplication>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDockWidget>
#include <QListWidget>
#include <QFileIconProvider>
#include <QStandardPaths>
#include <QSplitter>
#include <QLabel>
CustomTreeView* treeView;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("ZZZipper");
    // Установка центрального виджета
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Создаем горизонтальный лэйаут для кнопок и QLineEdit
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(10, 10, 10, 10);
    topLayout->setSpacing(15);



    // Настройка кнопок
    ui->plusbut->setIcon(QIcon(":/ico/ico/p1.png"));
    ui->minusbut->setIcon(QIcon(":/ico/ico/m1.png"));
    ui->serbut->setIcon(QIcon(":/ico/ico/s1.png"));
    ui->infobut->setIcon(QIcon(":/ico/ico/i1.png"));
    ui->openbut->setIcon(QIcon(":/ico/ico/f1.png"));
    // Применяем стиль кнопкам
    QString buttonStyle = R"(
        QPushButton {
            background-color: #2e3b4e;
            color: #ffffff;
            border: 1px solid #3c4a5d;
            border-radius: 6px;
            padding: 8px;
        }
        QPushButton:hover {
            background-color: #3c4a5d;
        }
        QPushButton:pressed {
            background-color: #1f2a38;
        }
    )";
    ui->plusbut->setStyleSheet(buttonStyle);
    ui->minusbut->setStyleSheet(buttonStyle);
    ui->serbut->setStyleSheet(buttonStyle);
    ui->infobut->setStyleSheet(buttonStyle);
    ui->openbut->setStyleSheet(buttonStyle);
    // Добавляем кнопки в лэйаут
    topLayout->addWidget(ui->plusbut);
    topLayout->addWidget(ui->minusbut);
    topLayout->addWidget(ui->infobut);
    topLayout->addWidget(ui->serbut);
    topLayout->addWidget(ui->openbut);
    // Настройка QLineEdit
    ui->lineEdit = new QLineEdit(this);
    ui->lineEdit->setPlaceholderText("Search...");
    ui->lineEdit->setEnabled(false);
    ui->lineEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: #2e3b4e;
            color: #ffffff;
            border: 1px solid #3c4a5d;
            border-radius: 6px;
            padding: 8px;
        }
        QLineEdit:focus {
            border: 1px solid #4a90e2;
        }
    )");
    topLayout->addWidget(ui->lineEdit);

    // Создаём QSplitter для Dock и TreeView
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    // Инициализация DockWidget
    setupDockWidget();
    splitter->addWidget(dock);

    // TreeView
    treeView = new CustomTreeView(this);
    treeModel = new QStandardItemModel(this);
    treeModel->setHorizontalHeaderLabels({ "Name", "Type", "Size (bytes)", "Extension" });
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(treeModel);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    treeView->setModel(proxyModel);
    treeView->setSortingEnabled(true);

    // Стиль TreeView
    treeView->setStyleSheet(R"(
        QTreeView {
            background-color: #1f2a38;
            color: #ffffff;
            border: none;
            padding: 8px;
        }
        QTreeView::item:hover {
            background-color: #3c4a5d;
        }
        QTreeView::item:selected {
            background-color: #4a90e2;
        }
        QHeaderView::section {
            background-color: #2e3b4e;
            color: #ffffff;
            padding: 5px;
            border: none;
        }
    )");
    splitter->addWidget(treeView);

    // Настройка пропорций
    splitter->setStretchFactor(0, 1); // Dock занимает 1 часть
    splitter->setStretchFactor(1, 3); // TreeView занимает 3 части

    // Основной лэйаут
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(splitter);
    statusIndicator = new QLabel(this);
    statusIndicator->setText("Idle"); // Текст по умолчанию
    statusIndicator->setAlignment(Qt::AlignCenter);
    statusIndicator->setFixedHeight(30);
    statusIndicator->setStyleSheet(R"(
    QLabel {
        background-color: #2e3b4e;
        color: #ffffff;
        border-radius: 6px;
        font-weight: bold;
    }
)");
    mainLayout->addWidget(statusIndicator);

    // Подключение событий
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_textChanged);
    connect(treeView, &QTreeView::customContextMenuRequested, this, &MainWindow::showContextMenu);
    connect(treeView, &QTreeView::doubleClicked, this, &MainWindow::onTreeItemClicked);
    connect(ui->openbut, &QPushButton::clicked, this, &MainWindow::openFile);


    // Инициализация файловой системы
    initializeFileSystem();

    // Минимальный размер окна
    setMinimumSize(900, 600);

    // Общий стиль окна
    setStyleSheet(R"(
        QMainWindow {
            background-color: #1f2a38;
        }
        QMenuBar {
            background-color: #2e3b4e;
            color: #ffffff;
        }
        QMenuBar::item:selected {
            background-color: #3c4a5d;
        }
        QMenu {
            background-color: #2e3b4e;
            color: #ffffff;
        }
        QMenu::item:selected {
            background-color: #3c4a5d;
        }
    )");

    QPushButton *aboutButton = new QPushButton(this);
    aboutButton->setText("About");
    aboutButton->setStyleSheet(R"(
    QPushButton {
        background-color: #2e3b4e;
        color: #ffffff;
        border: 1px solid #3c4a5d;
        border-radius: 6px;
        padding: 8px;
    }
    QPushButton:hover {
        background-color: #3c4a5d;
    }
    QPushButton:pressed {
        background-color: #1f2a38;
    }
)");
    topLayout->addWidget(aboutButton); // Добавляем кнопку в верхний лэйаут

    // Подключаем кнопку к обработчику
    connect(aboutButton, &QPushButton::clicked, this, &MainWindow::showAboutDialog);
}
void MainWindow::showAboutDialog() {
    // Создаём окно "About"
    QDialog *aboutDialog = new QDialog(this);
    aboutDialog->setWindowTitle("About ZZZiper");
    aboutDialog->setFixedSize(400, 300);
    aboutDialog->setStyleSheet(R"(
        QDialog {
            background-color: #1f2a38;
            color: #ffffff;
            border-radius: 8px;
        }
    )");

    // Лэйаут для содержимого
    QVBoxLayout *layout = new QVBoxLayout(aboutDialog);

    // Иконка программы
    QLabel *iconLabel = new QLabel(aboutDialog);
    QPixmap iconPixmap(":/ico/ico/app_icon.png"); // Замените на вашу иконку
    iconLabel->setPixmap(iconPixmap.scaled(64, 64, Qt::KeepAspectRatio));
    iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconLabel);

    // Название программы
    QLabel *titleLabel = new QLabel("ZZZiper", aboutDialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    layout->addWidget(titleLabel);

    // Описание программы
    QLabel *descriptionLabel = new QLabel("ZZZiper - ваш помощник для работы с архивами. "
                                          "Быстрый, удобный и современный инструмент.", aboutDialog);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(descriptionLabel);

    // Копирайт
    QLabel *copyrightLabel = new QLabel("© 2025 TrueAnectode", aboutDialog);
    copyrightLabel->setAlignment(Qt::AlignCenter);
    copyrightLabel->setStyleSheet("font-size: 12px; color: #aaaaaa;");
    layout->addWidget(copyrightLabel);

    // Кнопка закрытия
    QPushButton *closeButton = new QPushButton("OK", aboutDialog);
    closeButton->setStyleSheet(R"(
        QPushButton {
            background-color: #2e3b4e;
            color: #ffffff;
            border: 1px solid #3c4a5d;
            border-radius: 6px;
            padding: 8px;
        }
        QPushButton:hover {
            background-color: #3c4a5d;
        }
        QPushButton:pressed {
            background-color: #1f2a38;
        }
    )");
    connect(closeButton, &QPushButton::clicked, aboutDialog, &QDialog::accept);
    layout->addWidget(closeButton);

    aboutDialog->setLayout(layout);
    aboutDialog->exec();
}

void MainWindow::setupDockWidget() {
    // Создаём QDockWidget
    dock = new QDockWidget("Quick Access", this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    dock->setMinimumWidth(200);

    // Создаём список для быстрого доступа
    QListWidget *quickAccessList = new QListWidget(dock);

    // Добавляем основные папки Windows с иконками
    QFileIconProvider iconProvider;
    struct Folder {
        QString name;
        QString path;
    };

    QList<Folder> folders = {
        {"Desktop", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)},
        {"Documents", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)},
        {"Downloads", QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)},
        {"Music", QStandardPaths::writableLocation(QStandardPaths::MusicLocation)},
        {"Pictures", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)},
        {"Videos", QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)}
    };

    for (const auto &folder : folders) {
        QListWidgetItem *item = new QListWidgetItem(iconProvider.icon(QFileInfo(folder.path)), folder.name, quickAccessList);
        item->setData(Qt::UserRole, folder.path);
        quickAccessList->addItem(item);
    }

    // Настраиваем стиль QListWidget
    quickAccessList->setStyleSheet(R"(
        QListWidget {
            background-color: #2e3b4e;
            color: #ffffff;
            border: none;
        }
        QListWidget::item:hover {
            background-color: #3c4a5d;
        }
        QListWidget::item:selected {
            background-color: #4a90e2;
            color: #ffffff;
        }
    )");

    dock->setWidget(quickAccessList);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // Подключаем выбор элемента к изменению пути в TreeView
    connect(quickAccessList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        QString path = item->data(Qt::UserRole).toString();
        if (QDir(path).exists()) {
            changeDirectory(path);
        } else {
            QMessageBox::warning(this, "Error", "The selected path does not exist!");
        }
    });
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    adjustUI();
}


MainWindow::~MainWindow() {
    delete ui;
}
void MainWindow::adjustUI() {
    int screenWidth = this->width();
    int screenHeight = this->height();

    int dockWidth = 220; // Ширина QDockWidget
    int buttonHeight = 40;
    int topMargin = 10;

    // Устанавливаем размер QDockWidget
    dock->setFixedWidth(dockWidth);
    dock->setFixedHeight(screenHeight);

    // Распределение кнопок
    int buttonWidth = 60;
    int buttonSpacing = 15;
    int totalButtonWidth = 4 * buttonWidth + 3 * buttonSpacing;
    int startX = dockWidth + 20;
    int startY = topMargin;

    ui->plusbut->setGeometry(startX, startY, buttonWidth, buttonHeight);
    ui->minusbut->setGeometry(startX + buttonWidth + buttonSpacing, startY, buttonWidth, buttonHeight);
    ui->infobut->setGeometry(startX + 2 * (buttonWidth + buttonSpacing), startY, buttonWidth, buttonHeight);
    ui->serbut->setGeometry(startX + 3 * (buttonWidth + buttonSpacing), startY, buttonWidth, buttonHeight);

    // Расположение поля поиска
    ui->lineEdit->setGeometry(startX + totalButtonWidth + buttonSpacing, startY, screenWidth - (startX + totalButtonWidth + buttonSpacing + 20), buttonHeight);

    // Расположение TreeView
    int treeViewTop = startY + buttonHeight + 20;
    treeView->setGeometry(
        dockWidth + 10,
        treeViewTop,
        screenWidth - dockWidth - 30,
        screenHeight - treeViewTop - 20);
}







void MainWindow::initializeFileSystem() {
    qDebug() << "Initializing file system...";
    treeModel->clear();
    treeModel->setHorizontalHeaderLabels({ "Name", "Type", "Size (bytes)", "Extension" });

    QFileIconProvider iconProvider;
    QStandardItem *rootItem = treeModel->invisibleRootItem();

    QList<QStorageInfo> drives = QStorageInfo::mountedVolumes();
    for (const QStorageInfo &drive : drives) {
        if (!drive.isReady() || drive.rootPath().isEmpty()) continue;

        QList<QStandardItem *> row;
        QStandardItem *item = new QStandardItem(iconProvider.icon(QFileIconProvider::Drive), drive.rootPath());
        item->setData(drive.rootPath(), Qt::UserRole);
        row.append(item);
        row.append(new QStandardItem("Drive"));
        row.append(new QStandardItem(""));
        row.append(new QStandardItem(""));
        rootItem->appendRow(row);

        qDebug() << "Drive added:" << drive.rootPath();
    }

    treeView->setModel(treeModel);
}

void MainWindow::changeDirectory(const QString &path) {
    qDebug() << "Changing directory to:" << path;

    QDir dir(path);
    if (!dir.exists() && !QFileInfo(path).isRoot()) {
        QMessageBox::warning(this, "Ошибка", "Путь не найден: " + path);
        return;
    }

    currentPath = QDir::cleanPath(path); // Обновляем текущий путь
    treeModel->clear();
    treeModel->setHorizontalHeaderLabels({ "Name", "Type", "Size (bytes)", "Extension" });

    QFileIconProvider iconProvider;
    QStandardItem *rootItem = treeModel->invisibleRootItem();

    // Добавляем элемент для возврата на уровень выше
    if (!dir.isRoot()) {
        QList<QStandardItem *> row;
        QStandardItem *item = new QStandardItem(iconProvider.icon(QFileIconProvider::Folder), "...");
        item->setData(dir.absolutePath(), Qt::UserRole + 2); // Привязываем абсолютный путь
        row.append(item);
        row.append(new QStandardItem("Folder"));
        row.append(new QStandardItem(""));
        row.append(new QStandardItem(""));
        rootItem->appendRow(row);
    }

    // Добавляем элементы текущей директории
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::DirsFirst);
    for (const QFileInfo &entry : entries) {
        QList<QStandardItem *> row;
        QStandardItem *item = new QStandardItem(
            iconProvider.icon(entry.isDir() ? QFileIconProvider::Folder : QFileIconProvider::File),
            entry.fileName()
            );
        item->setData(entry.isDir(), Qt::UserRole + 1); // Привязываем флаг папки/файла
        item->setData(entry.absoluteFilePath(), Qt::UserRole + 2); // Привязываем абсолютный путь
        row.append(item);

        row.append(new QStandardItem(entry.isDir() ? "Folder" : "File"));
        row.append(new QStandardItem(entry.isDir() ? "" : QString::number(entry.size())));
        row.append(new QStandardItem(entry.isDir() ? "" : entry.suffix()));

        rootItem->appendRow(row);
    }

    treeView->setModel(treeModel);
    qDebug() << "TreeView updated for path:" << currentPath;
}



void setStatusIndicator(const QString &status, const QString &color) {
    statusIndicator->setText(status);
    statusIndicator->setStyleSheet(QString(R"(
        QLabel {
            background-color: %1;
            color: #ffffff;
            border-radius: 6px;
            font-weight: bold;
        }
    )").arg(color));
}




void MainWindow::extractFile(const QString &filePath, const QString &destinationDir) {
    qDebug() << "Extracting file:" << filePath << "to:" << destinationDir;

    // Установка индикатора работы
    setStatusIndicator("Extracting file...", "#B0A680"); // Тускло-желтый

    if (filePath.isEmpty()) {
        setStatusIndicator("Error: No file selected", "#A08080"); // Тускло-красный
        QMessageBox::warning(this, "Ошибка", "Не выбран файл для извлечения.");
        return;
    }

    if (!insideArchive) {
        // Если не в архиве, копируем файл напрямую
        if (QFile::copy(filePath, destinationDir + "/" + QFileInfo(filePath).fileName())) {
            qDebug() << "File extracted outside archive.";
            setStatusIndicator("Extraction complete", "#80A080"); // Тускло-зеленый
        } else {
            setStatusIndicator("Error during extraction", "#A08080"); // Тускло-красный
        }
        return;
    }

    // Работа внутри архива
    struct archive *a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    if (archive_read_open_filename(a, archivePath.toStdString().c_str(), 10240) != ARCHIVE_OK) {
        setStatusIndicator("Error opening archive", "#A08080"); // Тускло-красный
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть архив для извлечения.");
        archive_read_free(a);
        return;
    }

    struct archive *ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME);
    archive_write_disk_set_standard_lookup(ext);

    struct archive_entry *entry;
    bool fileFound = false;

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        QString entryPath = QString::fromUtf8(archive_entry_pathname(entry)).trimmed();

        // Проверяем, совпадает ли путь с искомым
        if (entryPath == filePath) {
            fileFound = true;

            QString destination = destinationDir + "/" + QFileInfo(entryPath).fileName();
            archive_entry_set_pathname(entry, destination.toStdString().c_str());

            // Извлекаем файл
            if (archive_write_header(ext, entry) != ARCHIVE_OK) {
                setStatusIndicator("Error writing header", "#A08080"); // Тускло-красный
                QMessageBox::warning(this, "Ошибка", "Не удалось записать заголовок для извлечения.");
            } else {
                const void *buff;
                size_t size;
                la_int64_t offset;
                while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
                    if (archive_write_data_block(ext, buff, size, offset) != ARCHIVE_OK) {
                        setStatusIndicator("Error writing data", "#A08080"); // Тускло-красный
                        QMessageBox::warning(this, "Ошибка", "Не удалось записать данные при извлечении.");
                    }
                }
                qDebug() << "File extracted successfully to:" << destination;
            }
            break;
        }
    }

    if (!fileFound) {
        setStatusIndicator("File not found in archive", "#A08080"); // Тускло-красный
        QMessageBox::warning(this, "Ошибка", "Файл не найден в архиве.");
    } else {
        setStatusIndicator("Extraction complete", "#80A080"); // Тускло-зеленый
    }

    archive_read_free(a);
    archive_write_free(ext);
}




void MainWindow::extractFolder(const QString &folderPath, const QString &destinationDir) {
    qDebug() << "Extracting folder:" << folderPath << "to:" << destinationDir;

    // Установка индикатора работы
    setStatusIndicator("Extracting folder...", "#B0A680"); // Тускло-желтый

    if (folderPath.isEmpty()) {
        setStatusIndicator("Error: No folder selected", "#A08080"); // Тускло-красный
        QMessageBox::warning(this, "Error", "No folder selected for extraction.");
        return;
    }

    if (!insideArchive) {
        // Извлечение из файловой системы
        QDir sourceDir(folderPath);
        QDir destination(destinationDir + "/" + QFileInfo(folderPath).fileName());
        if (!destination.mkpath(".")) {
            setStatusIndicator("Error creating destination folder", "#A08080"); // Тускло-красный
            QMessageBox::warning(this, "Error", "Failed to create folder for extraction.");
            return;
        }

        foreach (QString file, sourceDir.entryList(QDir::Files)) {
            QFile::copy(sourceDir.filePath(file), destination.filePath(file));
        }
        foreach (QString subFolder, sourceDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            extractFolder(sourceDir.filePath(subFolder), destination.filePath(subFolder));
        }
        qDebug() << "Folder extracted outside archive.";
        setStatusIndicator("Extraction complete", "#80A080"); // Тускло-зеленый
        return;
    }

    // Извлечение из архива
    struct archive *a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    if (archive_read_open_filename(a, archivePath.toStdString().c_str(), 10240) != ARCHIVE_OK) {
        setStatusIndicator("Error opening archive", "#A08080"); // Тускло-красный
        QMessageBox::warning(this, "Error", "Failed to open archive for extraction.");
        archive_read_free(a);
        return;
    }

    struct archive *ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME);
    archive_write_disk_set_standard_lookup(ext);

    struct archive_entry *entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        QString entryPath = QString::fromUtf8(archive_entry_pathname(entry)).trimmed();

        // Проверяем, является ли элемент внутри папки
        if (entryPath.startsWith(folderPath + "/") || entryPath == folderPath) {
            QString relativePath = entryPath.mid(folderPath.length() + 1);
            QString destination = destinationDir + "/" + QFileInfo(folderPath).fileName() + "/" + relativePath;

            if (archive_entry_filetype(entry) == AE_IFDIR) {
                QDir().mkpath(destination);
            } else {
                archive_entry_set_pathname(entry, destination.toStdString().c_str());

                if (archive_write_header(ext, entry) != ARCHIVE_OK) {
                    setStatusIndicator("Error writing header", "#A08080"); // Тускло-красный
                    qDebug() << "Failed to write header for entry:" << entryPath;
                } else {
                    const void *buff;
                    size_t size;
                    la_int64_t offset;
                    while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
                        if (archive_write_data_block(ext, buff, size, offset) != ARCHIVE_OK) {
                            setStatusIndicator("Error writing data", "#A08080"); // Тускло-красный
                            qDebug() << "Failed to write data for entry:" << entryPath;
                        }
                    }
                }
            }
        }
    }

    archive_read_free(a);
    archive_write_free(ext);

    setStatusIndicator("Extraction complete", "#80A080"); // Тускло-зеленый
}




QString findRootPath(const QStringList &paths) {
    if (paths.isEmpty()) return "";

    // Разделяем пути на компоненты и находим общий префикс
    QStringList rootParts = paths.first().split("/", Qt::SkipEmptyParts);
    for (const QString &path : paths) {
        QStringList parts = path.split("/", Qt::SkipEmptyParts);
        for (int i = 0; i < rootParts.size(); ++i) {
            if (i >= parts.size() || rootParts[i] != parts[i]) {
                rootParts = rootParts.mid(0, i); // Общий префикс
                break;
            }
        }
    }

    return rootParts.join("/");
}



void MainWindow::openFile() {
    QString selectedFile = QFileDialog::getOpenFileName(this, "Open File", currentPath, "Archives (*.zip *.tar *.rar *.7z *.gz *.bz2)");
    if (selectedFile.isEmpty()) {
        return;
    }

    previousDirectory = currentPath; // Сохраняем текущую директорию
    openArchive(selectedFile);      // Открываем архив
}



void debugFileTree() {
    for (auto it = fileTree.begin(); it != fileTree.end(); ++it) {
        qDebug() << "Path:" << it.key();
        for (const auto &entry : it.value()) {
            qDebug() << "  Name:" << entry.name << "IsFolder:" << entry.isFolder;
        }
    }
}


void MainWindow::updateTreeView() {
    treeModel->clear();
    treeModel->setHorizontalHeaderLabels({ "Name", "Type", "Size (bytes)", "Extension" });

    QFileIconProvider iconProvider;
    QStandardItem *rootItem = treeModel->invisibleRootItem();

    if (insideArchive) {
        if (!archiveFileTree.contains(archiveCurrentPath)) {
            QMessageBox::warning(this, "Ошибка", "Путь не найден в архиве.");
            return;
        }

        // Добавляем элемент для возврата на уровень выше
        QList<QStandardItem *> row;
        QStandardItem *item = new QStandardItem(iconProvider.icon(QFileIconProvider::Folder), "...");
        row.append(item);
        row.append(new QStandardItem("Folder"));
        row.append(new QStandardItem(""));
        row.append(new QStandardItem(""));
        rootItem->appendRow(row);

        // Добавляем содержимое архива
        const auto &entries = archiveFileTree[archiveCurrentPath];
        for (const FileEntry &entry : entries) {
            QList<QStandardItem *> row;
            QStandardItem *item = new QStandardItem(
                iconProvider.icon(entry.isFolder ? QFileIconProvider::Folder : QFileIconProvider::File),
                entry.name
                );
            row.append(item);
            row.append(new QStandardItem(entry.isFolder ? "Folder" : "File"));
            row.append(new QStandardItem(""));
            row.append(new QStandardItem(""));
            rootItem->appendRow(row);
        }
    } else {
        // Обычная файловая система
        changeDirectory(currentPath);
    }

    treeView->setModel(treeModel);
}



void MainWindow::onTreeItemClicked(const QModelIndex &index) {
    if (!index.isValid()) return;

    QString selectedName = index.sibling(index.row(), 0).data(Qt::DisplayRole).toString();
    QString selectedType = index.sibling(index.row(), 1).data(Qt::DisplayRole).toString();
    qDebug() << "Selected item:" << selectedName << "Type:" << selectedType;

    // Если выбран элемент "...", возвращаемся на уровень выше
    if (selectedName == "...") {
        if (insideArchive) {
            if (archiveCurrentPath.isEmpty()) {
                // Выходим из архива
                insideArchive = false;
                archivePath.clear();
                archiveFileTree.clear();
                changeDirectory(previousDirectory); // Возвращаемся в предыдущую директорию
            } else {
                // Возвращаемся на уровень выше внутри архива
                QStringList pathParts = archiveCurrentPath.split("/", Qt::SkipEmptyParts);
                pathParts.removeLast();
                archiveCurrentPath = pathParts.join("/");
                updateTreeView();
            }
        } else {
            QDir currentDir(currentPath);
            if (currentDir.cdUp()) {
                currentPath = currentDir.absolutePath();
                changeDirectory(currentPath);
            } else {
                initializeFileSystem();
            }
        }
        return;
    }

    QString newPath;
    if (selectedType == "Drive") {
        newPath = selectedName;
    } else if (selectedType == "Folder") {
        if (insideArchive) {
            archiveCurrentPath = archiveCurrentPath.isEmpty()
            ? selectedName
            : archiveCurrentPath + "/" + selectedName;
            updateTreeView();
            return;
        } else {
            newPath = currentPath.isEmpty() ? selectedName : QDir(currentPath).filePath(selectedName);
        }
    } else if (selectedType == "File") {
        QString extension = QFileInfo(selectedName).suffix().toLower();
        QStringList archiveExtensions = { "zip", "rar", "7z", "tar", "gz", "bz2" };
        if (archiveExtensions.contains(extension)) {
            openArchive(currentPath.isEmpty() ? selectedName : QDir(currentPath).filePath(selectedName));
            return;
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(currentPath + "/" + selectedName));
            return;
        }
    }

    if (!newPath.isEmpty() && QFileInfo(newPath).isDir()) {
        currentPath = QDir::cleanPath(newPath);
        changeDirectory(currentPath);
    }
}



void MainWindow::openArchive(const QString &archivePath) {
    qDebug() << "Opening archive:" << archivePath;

    struct archive *a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    if (archive_read_open_filename(a, archivePath.toStdString().c_str(), 10240) != ARCHIVE_OK) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть архив: " + archivePath);
        archive_read_free(a);
        return;
    }

    insideArchive = true;
    this->archivePath = archivePath;
    previousDirectory = QFileInfo(archivePath).absolutePath(); // Сохраняем родительскую директорию архива
    archiveCurrentPath.clear();
    archiveFileTree.clear();

    struct archive_entry *entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        QString entryPath = QString::fromUtf8(archive_entry_pathname(entry)).trimmed();
        QStringList pathParts = entryPath.split("/", Qt::SkipEmptyParts);
        if (pathParts.isEmpty()) continue;

        QString parentPath = pathParts.mid(0, pathParts.size() - 1).join("/");
        QString fileName = pathParts.last();
        bool isFolder = archive_entry_filetype(entry) == AE_IFDIR;

        archiveFileTree[parentPath].append({ fileName, isFolder });
    }

    archive_read_free(a);
    updateTreeView();
}



void MainWindow::on_pushButton_clicked()
{

}


void MainWindow::showContextMenu(const QPoint &pos) {
    QModelIndexList selectedIndexes = treeView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        return;
    }

    QMenu contextMenu(treeView);
    QAction *extractAction = contextMenu.addAction("Extract Files");
    QAction *infoAction = contextMenu.addAction("Files Info");
    QAction *selectedAction = contextMenu.exec(treeView->viewport()->mapToGlobal(pos));

    if (selectedAction == extractAction) {
        // Диалоговое окно для выбора пути
        QString destinationDir = QFileDialog::getExistingDirectory(
            this,
            "Select Destination",
            QDir::homePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (!destinationDir.isEmpty()) {
            // Перебираем все выбранные файлы и извлекаем их
            for (const QModelIndex &index : selectedIndexes) {
                QString selectedName = index.data(Qt::DisplayRole).toString().trimmed();

                // Игнорируем пустые элементы или "..."
                if (selectedName.isEmpty() || selectedName == "...") {
                    qDebug() << "Invalid item selected. Skipping:" << selectedName;
                    continue;
                }

                QString selectedPath;
                if (insideArchive) {
                    selectedPath = archiveCurrentPath.isEmpty()
                    ? selectedName
                    : archiveCurrentPath + "/" + selectedName;
                } else {
                    selectedPath = currentPath.isEmpty()
                    ? selectedName
                    : currentPath + "/" + selectedName;
                }

                // Проверяем, является ли выбранный элемент папкой или файлом
                bool isFolder = false;

                if (insideArchive) {
                    const auto &entries = archiveFileTree.value(archiveCurrentPath, QList<FileEntry>());
                    for (const FileEntry &entry : entries) {
                        if (entry.name == selectedName) {
                            isFolder = entry.isFolder;
                            break;
                        }
                    }
                } else {
                    const auto &entries = fileTree.value(currentPath, QList<FileEntry>());
                    for (const FileEntry &entry : entries) {
                        if (entry.name == selectedName) {
                            isFolder = entry.isFolder;
                            break;
                        }
                    }
                }

                // Извлечение папки или файла
                if (isFolder) {
                    qDebug() << "Extracting folder:" << selectedName << "to:" << destinationDir;
                    if (insideArchive) {
                        extractFolder(selectedPath, destinationDir);
                    } else {
                        extractFolder(selectedPath, destinationDir);
                    }
                } else {
                    qDebug() << "Extracting file:" << selectedName << "to:" << destinationDir;
                    if (insideArchive) {
                        extractFile(selectedPath, destinationDir);
                    } else {
                        extractFile(selectedPath, destinationDir);
                    }
                }
            }
        }
    } else if (selectedAction == infoAction) {
             on_infobut_clicked();
        }


}


/// 50/50
void MainWindow::on_infobut_clicked() {
    QModelIndexList selectedIndexes = treeView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select files or folders to view their information.");
        return;
    }

    QSet<QString> processedPaths; // Отслеживаем уникальные пути для исключения дублирования
    QStringList infoMessages;

    for (const QModelIndex &index : selectedIndexes) {
        QString selectedName = index.data(Qt::DisplayRole).toString().trimmed();
        QString selectedPath;

        // Убедимся, что имя и путь валидны
        if (selectedName.isEmpty() || selectedName == "..." || selectedName == "File" || selectedName == "Folder") {
            continue;
        }

        if (insideArchive) {
            selectedPath = archiveCurrentPath.isEmpty()
            ? selectedName
            : archiveCurrentPath + "/" + selectedName;
        } else {
            selectedPath = currentPath.isEmpty()
            ? selectedName
            : currentPath + "/" + selectedName;
        }

        // Пропускаем элементы, которые уже обработаны
        if (processedPaths.contains(selectedPath)) {
            continue;
        }
        processedPaths.insert(selectedPath);

        // Определяем тип и размер
        QString type = "File";
        QString size = "N/A";

        if (insideArchive) {
            const auto &entries = archiveFileTree.value(archiveCurrentPath, QList<FileEntry>());
            for (const FileEntry &entry : entries) {
                if (entry.name == selectedName) {
                    type = entry.isFolder ? "Folder" : "File";
                    break;
                }
            }
        } else {
            QFileInfo fileInfo(selectedPath);
            if (fileInfo.exists()) {
                type = fileInfo.isDir() ? "Folder" : "File";
                if (fileInfo.isFile()) {
                    size = QString::number(fileInfo.size()) + " bytes";
                }
            }
        }

        QString infoMessage = QString("Name: %1\nPath: %2\nType: %3\nSize: %4")
                                  .arg(selectedName)
                                  .arg(selectedPath)
                                  .arg(type)
                                  .arg(size);

        infoMessages.append(infoMessage);
    }

    if (!infoMessages.isEmpty()) {
        QMessageBox::information(this, "Files and Folders Info", infoMessages.join("\n\n"));
    } else {
        QMessageBox::warning(this, "Error", "No valid information could be retrieved.");
    }
}



////////////////////// удаление тоже пашит

void MainWindow::on_minusbut_clicked() {
    QModelIndexList selectedIndexes = treeView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select files or folders to delete.");
        return;
    }

    if (!insideArchive) {
        QMessageBox::warning(this, "Error", "Deletion is only supported inside archives.");
        return;
    }

    // Collect selected paths
    QSet<QString> selectedPaths;
    for (const QModelIndex &index : selectedIndexes) {
        QString selectedName = index.data(Qt::DisplayRole).toString().trimmed();
        if (!selectedName.isEmpty() && selectedName != "...") {
            QString fullPath = archiveCurrentPath.isEmpty()
            ? selectedName
            : archiveCurrentPath + "/" + selectedName;
            selectedPaths.insert(fullPath);
        }
    }

    if (selectedPaths.isEmpty()) {
        QMessageBox::warning(this, "Error", "No items selected for deletion.");
        return;
    }

    // Create a temporary archive
    QString tempArchivePath = QDir::tempPath() + "/temp_archive.zip";
    struct archive *inputArchive = archive_read_new();
    struct archive *outputArchive = archive_write_new();

    archive_read_support_format_all(inputArchive);
    archive_read_support_filter_all(inputArchive);

    if (archive_read_open_filename(inputArchive, archivePath.toStdString().c_str(), 10240) != ARCHIVE_OK) {
        QMessageBox::warning(this, "Error", "Failed to open the archive.");
        qDebug() << "Failed to open archive:" << archive_error_string(inputArchive);
        archive_read_free(inputArchive);
        return;
    }

    archive_write_set_format_zip(outputArchive);
    archive_write_add_filter_none(outputArchive);

    if (archive_write_open_filename(outputArchive, tempArchivePath.toStdString().c_str()) != ARCHIVE_OK) {
        QMessageBox::warning(this, "Error", "Failed to create a temporary archive.");
        qDebug() << "Failed to create temporary archive:" << archive_error_string(outputArchive);
        archive_read_free(inputArchive);
        archive_write_free(outputArchive);
        return;
    }

    struct archive_entry *entry;
    while (archive_read_next_header(inputArchive, &entry) == ARCHIVE_OK) {
        QString entryPath = QString::fromUtf8(archive_entry_pathname(entry)).trimmed();

        // Check if the current item should be deleted
        bool shouldDelete = false;
        for (const QString &path : selectedPaths) {
            if (entryPath == path || entryPath.startsWith(path + "/")) {
                shouldDelete = true;
                break;
            }
        }

        if (shouldDelete) {
            qDebug() << "Skipping entry for deletion:" << entryPath;
            continue; // Skip this entry
        }

        // Prepare the entry for writing
        if (archive_write_header(outputArchive, entry) != ARCHIVE_OK) {
            qDebug() << "Failed to write header for entry:" << entryPath
                     << "Error:" << archive_error_string(outputArchive);
            continue;
        }

        // Use archive_write_data instead of archive_write_data_block
        char buffer[8192];
        ssize_t size;
        while ((size = archive_read_data(inputArchive, buffer, sizeof(buffer))) > 0) {
            if (archive_write_data(outputArchive, buffer, size) < 0) {
                qDebug() << "Failed to write data for entry:" << entryPath
                         << "Error:" << archive_error_string(outputArchive);
                break;
            }
        }

        if (size < 0) {
            qDebug() << "Error reading data for entry:" << entryPath
                     << "Error:" << archive_error_string(inputArchive);
        }
    }

    if (archive_read_free(inputArchive) != ARCHIVE_OK) {
        qDebug() << "Failed to close input archive:" << archive_error_string(inputArchive);
    }

    if (archive_write_close(outputArchive) != ARCHIVE_OK) {
        qDebug() << "Failed to close output archive:" << archive_error_string(outputArchive);
    }

    if (archive_write_free(outputArchive) != ARCHIVE_OK) {
        qDebug() << "Failed to free output archive:" << archive_error_string(outputArchive);
    }

    // Replace the old archive with the new one
    QFile::remove(archivePath);
    if (!QFile::rename(tempArchivePath, archivePath)) {
        QMessageBox::warning(this, "Error", "Failed to replace the original archive.");
        return;
    }

    // Update TreeView
    openArchive(archivePath);
    QMessageBox::information(this, "Success", "Selected items were successfully removed from the archive.");
}




///////////////////////////////////////Хуй сосоте
/// Создание архива, работает иделаьно

bool addFileToArchive(struct archive *a, const QString &filePath, const QString &archivePath) {
    struct archive_entry *entry = archive_entry_new();
    if (!entry) {
        qCritical() << "Failed to create archive entry.";
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed to open file for reading:" << filePath;
        archive_entry_free(entry);
        return false;
    }

    QFileInfo fileInfo(filePath);

    archive_entry_set_pathname(entry, archivePath.toStdString().c_str());
    archive_entry_set_size(entry, file.size());
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_perm(entry, fileInfo.permissions());  // Устанавливаем права доступа

    // Устанавливаем временные метки
    archive_entry_set_mtime(entry, fileInfo.lastModified().toSecsSinceEpoch(), 0);
    archive_entry_set_atime(entry, fileInfo.lastRead().toSecsSinceEpoch(), 0);
    archive_entry_set_ctime(entry, fileInfo.lastRead().toSecsSinceEpoch(), 0);

    if (archive_write_header(a, entry) != ARCHIVE_OK) {
        qCritical() << "Failed to write header for file:" << filePath;
        archive_entry_free(entry);
        return false;
    }

    QByteArray buffer = file.readAll();
    archive_write_data(a, buffer.data(), buffer.size());

    archive_entry_free(entry);
    return true;
}


bool addFolderToArchive(struct archive *a, const QString &folderPath, const QString &archivePath) {
    // Создаем запись для папки
    struct archive_entry *entry = archive_entry_new();
    if (!entry) {
        qCritical() << "Failed to create archive entry for folder.";
        return false;
    }

    archive_entry_set_pathname(entry, archivePath.toStdString().c_str());
    archive_entry_set_filetype(entry, AE_IFDIR);
    archive_entry_set_perm(entry, 0755);  // Стандартные права доступа для папок
    archive_entry_set_size(entry, 0);    // Папки не имеют размера

    // Устанавливаем текущую дату или любую другую (например, текущий год)
    archive_entry_set_mtime(entry, time(nullptr), 0);

    if (archive_write_header(a, entry) != ARCHIVE_OK) {
        qCritical() << "Failed to write header for folder:" << folderPath;
        archive_entry_free(entry);
        return false;
    }
    archive_entry_free(entry);

    // Рекурсивно добавляем содержимое папки
    QDir dir(folderPath);
    for (const QFileInfo &entryInfo : dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (entryInfo.isFile()) {
            addFileToArchive(a, entryInfo.absoluteFilePath(), archivePath + "/" + entryInfo.fileName());
        } else if (entryInfo.isDir()) {
            addFolderToArchive(a, entryInfo.absoluteFilePath(), archivePath + "/" + entryInfo.fileName());
        }
    }

    return true;
}


bool createArchive(QString outputPath, const QString &format, const QStringList &files, const QString &compressionLevel) {
    // Добавляем расширение к имени файла, если его нет
    setStatusIndicator("Working...", "#4a90e2"); // Цвет работы (синий)
    QString extension;
    if (format == "zip") {
        extension = ".zip";
    } else if (format == "tar") {
        extension = ".tar";
    } else if (format == "7z") {
        extension = ".7z";
    } else if (format == "gz") {
        extension = ".tar.gz";
    }

    if (!outputPath.endsWith(extension, Qt::CaseInsensitive)) {
        outputPath += extension;
    }

    struct archive *a = archive_write_new();
    if (!a) {
        qCritical() << "Failed to initialize archive.";
        return false;
    }

    // Настраиваем формат
    if (format == "zip") {
        archive_write_set_format_zip(a);
    } else if (format == "tar") {
        archive_write_set_format_pax_restricted(a);
    } else if (format == "7z") {
        archive_write_set_format_7zip(a);
    } else if (format == "gz") {
        archive_write_set_format_gnutar(a);
    }

    // Настраиваем уровень сжатия
    if (compressionLevel == "Fast") {
        archive_write_set_bytes_per_block(a, 512);
    } else if (compressionLevel == "Normal") {
        archive_write_set_bytes_per_block(a, 1024);
    } else if (compressionLevel == "Maximum") {
        archive_write_set_bytes_per_block(a, 2048);
    }

    // Открываем выходной файл
    if (archive_write_open_filename(a, outputPath.toStdString().c_str()) != ARCHIVE_OK) {
        qCritical() << "Failed to open archive for writing.";
        archive_write_free(a);
        return false;
    }

    for (const QString &path : files) {
        QFileInfo fileInfo(path);
        if (fileInfo.isFile()) {
            addFileToArchive(a, path, fileInfo.fileName());
        } else if (fileInfo.isDir()) {
            addFolderToArchive(a, path, fileInfo.fileName());
        }
    }

    archive_write_close(a);
    archive_write_free(a);

    return true;
}


void MainWindow::on_plusbut_clicked() {
    ArchiveDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {

        QString outputPath = dialog.getOutputPath();
        QString format = dialog.getFormat();
        QString compressionLevel = dialog.getCompressionLevel();
        QStringList selectedFiles = dialog.getSelectedFiles();



        if (createArchive(outputPath, format, selectedFiles, compressionLevel)) {
            setStatusIndicator("Idle", "#2e3b4e"); // Цвет простоя (темный)
            QMessageBox::information(this, "Success", "Archive created successfully!");
        } else {
            setStatusIndicator("Idle", "#2e3b4e");
            QMessageBox::critical(this, "Error", "Failed to create archive.");
        }

    }
}


void MainWindow::on_serbut_clicked()
{
    // Включить/выключить поле для ввода
    bool isEnabled = ui->lineEdit->isEnabled();
    ui->lineEdit->setEnabled(!isEnabled);
    if (!isEnabled) {
        ui->lineEdit->clear();
    } else {
        updateTreeView(); // Возвращаем стандартное отображение
    }
}

void MainWindow::on_lineEdit_textChanged(const QString &text)
{
    if (text.isEmpty()) {
        updateTreeView(); // Если поле пустое, возвращаем стандартное отображение
        return;
    }

    // Создаём фильтрованную модель
    QStandardItemModel *filteredModel = new QStandardItemModel(this);

    // Копируем заголовки из исходной модели
    for (int col = 0; col < treeModel->columnCount(); ++col) {
        filteredModel->setHeaderData(col, Qt::Horizontal, treeModel->headerData(col, Qt::Horizontal));
    }

    // Фильтруем строки
    for (int row = 0; row < treeModel->rowCount(); ++row) {
        QStandardItem *item = treeModel->item(row, 0); // Получаем элемент первого столбца
        if (item->text().contains(text, Qt::CaseInsensitive)) {
            QList<QStandardItem *> newRow;
            for (int col = 0; col < treeModel->columnCount(); ++col) {
                newRow.append(treeModel->item(row, col)->clone());
            }
            filteredModel->appendRow(newRow);
        }
    }

    // Устанавливаем фильтрованную модель в TreeView
    treeView->setModel(filteredModel);
}



