#include "archivedialog.h"
#include "ui_archivedialog.h"
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
ArchiveDialog::ArchiveDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ArchiveDialog)
{
    ui->setupUi(this);
    setWindowTitle("Create Archive");

    // Общий стиль окна
    setStyleSheet(R"(
    QDialog {
        background-color: #1f2a38;
    }
    QLabel, QLineEdit, QComboBox {
        background-color: #2e3b4e;
        color: #ffffff;
        border: 1px solid #3c4a5d;
        border-radius: 6px;
        padding: 8px;
    }
    QLabel {
        border: none; /* Убираем рамки для заголовков */
    }
    QLineEdit:focus, QComboBox:focus {
        outline: none; /* Убираем выделение при фокусе */
        border: 1px solid #2e3b4e; /* Рамка совпадает с фоном */
    }
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
    QTreeWidget {
        background-color: #1f2a38;
        color: #ffffff;
        border: none;
        padding: 8px;
    }
    QTreeWidget::item {
        background-color: transparent;
        border: none;
        color: #ffffff;
    }
    QTreeWidget::item:hover {
        background-color: rgba(100, 100, 100, 0.3); /* Мягкий серый цвет */
    }
    QTreeWidget::item:selected {
        background-color: rgba(50, 50, 255, 0.4); /* Мягкий синий цвет */
        border-radius: 4px;
    }
    QHeaderView::section {
        background-color: #2e3b4e;
        color: #ffffff;
        padding: 5px;
        border: none;
    }
)");



    // Поле пути и кнопка "Browse"
    QLabel *outputLabel = new QLabel("Output Path:", this);
    outputPathEdit = new QLineEdit(this);
    QPushButton *browseButton = new QPushButton("Browse", this);
    connect(browseButton, &QPushButton::clicked, this, &ArchiveDialog::selectOutputPath);

    QHBoxLayout *outputLayout = new QHBoxLayout;
    outputLayout->addWidget(outputLabel);
    outputLayout->addWidget(outputPathEdit);
    outputLayout->addWidget(browseButton);

    // Выбор формата
    QLabel *formatLabel = new QLabel("Archive Format:", this);
    formatComboBox = new QComboBox(this);
    formatComboBox->addItems({"zip", "tar", "7z", "gz"});

    // Уровень сжатия
    QLabel *compressionLabel = new QLabel("Compression Level:", this);
    compressionComboBox = new QComboBox(this);
    compressionComboBox->addItems({"Fast", "Normal", "Maximum"});

    // Список файлов/папок
    QLabel *fileListLabel = new QLabel("Files and Folders:", this);
    fileListWidget = new QTreeWidget(this);
    fileListWidget->setHeaderLabels({"Path", "Type"});
    fileListWidget->header()->setSectionResizeMode(QHeaderView::Stretch);

    QPushButton *addFileButton = new QPushButton("Add Files", this);
    QPushButton *addFolderButton = new QPushButton("Add Folders", this);
    QPushButton *removeButton = new QPushButton("Remove Selected", this);

    connect(addFileButton, &QPushButton::clicked, this, &ArchiveDialog::addFiles);
    connect(addFolderButton, &QPushButton::clicked, this, &ArchiveDialog::addFolders);
    connect(removeButton, &QPushButton::clicked, this, &ArchiveDialog::removeSelected);

    QHBoxLayout *fileControlLayout = new QHBoxLayout;
    fileControlLayout->addWidget(addFileButton);
    fileControlLayout->addWidget(addFolderButton);
    fileControlLayout->addWidget(removeButton);

    // Кнопки "OK" и "Cancel"
    QPushButton *okButton = new QPushButton("OK", this);
    QPushButton *cancelButton = new QPushButton("Cancel", this);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    // Основной макет
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(outputLayout);
    mainLayout->addWidget(formatLabel);
    mainLayout->addWidget(formatComboBox);
    mainLayout->addWidget(compressionLabel);
    mainLayout->addWidget(compressionComboBox);
    mainLayout->addWidget(fileListLabel);
    mainLayout->addWidget(fileListWidget);
    mainLayout->addLayout(fileControlLayout);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    // Минимальный размер окна
    setMinimumSize(500, 400);
}


QString ArchiveDialog::getOutputPath() const {
    return outputPathEdit->text();
}

QString ArchiveDialog::getFormat() const {
    return formatComboBox->currentText();
}

QString ArchiveDialog::getCompressionLevel() const {
    return compressionComboBox->currentText();
}

QStringList ArchiveDialog::getSelectedFiles() const {
    QStringList files;
    for (int i = 0; i < fileListWidget->topLevelItemCount(); ++i) {
        files.append(fileListWidget->topLevelItem(i)->text(0));
    }
    return files;
}

void ArchiveDialog::selectOutputPath() {
    QString path = QFileDialog::getSaveFileName(this, "Select Output Path");
    if (!path.isEmpty()) {
        outputPathEdit->setText(path);
    }
}

void ArchiveDialog::addFiles() {
    QStringList files = QFileDialog::getOpenFileNames(this, "Select Files");
    for (const QString &file : files) {
        QTreeWidgetItem *item = new QTreeWidgetItem(fileListWidget);
        item->setText(0, file);
        item->setText(1, "File");
    }
}

void ArchiveDialog::addFolders() {
    QString folder = QFileDialog::getExistingDirectory(this, "Select Folder");
    if (!folder.isEmpty()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(fileListWidget);
        item->setText(0, folder);
        item->setText(1, "Folder");
    }
}

void ArchiveDialog::removeSelected() {
    qDeleteAll(fileListWidget->selectedItems());
}
ArchiveDialog::~ArchiveDialog()
{
    delete ui;
}
