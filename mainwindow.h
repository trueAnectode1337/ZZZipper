#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QTreeView>
#include <QStandardItemModel>
#include <QMimeData>
#include <QDrag>
#include <QTemporaryDir>
#include <QFileDialog>
#include <QDropEvent>
#include <QCoreApplication>
#include <QSortFilterProxyModel>
#include <QLabel>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
struct FileEntry {
    QString name;
    bool isFolder;
    bool operator==(const FileEntry &other) const {
        return name == other.name && isFolder == other.isFolder;
    }

};
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void updateTreeView();
    void changeDirectory(const QString &path);
    void initializeFileSystem();
    void extractFile(const QString &filePath, const QString &destinationDir);
    void extractFolder(const QString &folderPath, const QString &destinationDir);
    void openArchive(const QString &archivePath);
    void addToArchive(struct archive *a, const QFileInfo &fileInfo, const QString &relativePath);
    QString findCommonRoot(const QStringList &filePaths);
    void adjustUI();
    void setupDockWidget();
    void changeTheme();

private slots:
    void openFile(); // Слот для открытия файла
    void showContextMenu(const QPoint &pos);
    void onTreeItemClicked(const QModelIndex &index);

    void on_pushButton_clicked();

    void on_minusbut_clicked();

    void on_infobut_clicked();
    void on_plusbut_clicked();

    void on_serbut_clicked();
    void on_lineEdit_textChanged(const QString &text);
    void resizeEvent(QResizeEvent *event);
    void showAboutDialog();

private:
    Ui::MainWindow *ui;    // Указатель на Ui

    QStandardItemModel *treeModel;
    QString fileName;  // Путь к открытому архиву
  QString archiveParentPath; // Путь к родительской директории архива

  QString archivePath;                  // Путь к текущему открытому архиву

  QSortFilterProxyModel *proxyModel;

  QString previousDirectory;
   QDockWidget *dock;
};
  void setStatusIndicator(const QString &status, const QString &color);
inline QMap<QString, QList<FileEntry>> archiveFileTree; // Структура файлов архива
inline QString archiveCurrentPath;           // Текущий путь внутри архива
inline bool insideArchive = false;
 inline QMap<QString, QList<FileEntry>> fileTree; // Дерево архива
inline QString currentPath;  // Убедитесь, что currentPath обновляется
 inline QLabel *statusIndicator;
 class CustomTreeView : public QTreeView {
     Q_OBJECT

 public:
     explicit CustomTreeView(QWidget *parent = nullptr) : QTreeView(parent) {
         setDragEnabled(true);
         setDragDropMode(QAbstractItemView::DragDrop);
         setSelectionMode(QAbstractItemView::MultiSelection);
         setAcceptDrops(true);

     }

 protected:
     void startDrag(Qt::DropActions supportedActions) override {
         qDebug() << "startDrag called!";

         QModelIndexList selectedIndexes = this->selectedIndexes();
         if (selectedIndexes.isEmpty()) {
             qDebug() << "No valid index selected.";
             return;
         }

         QSet<QString> selectedPathsSet;

         // Обрабатываем все выбранные индексы
         for (const QModelIndex &index : selectedIndexes) {
             QString selectedName = index.model()->data(index.sibling(index.row(), 0), Qt::DisplayRole).toString();

             if (selectedName.isEmpty()) continue;

             QString fullPath;
             if (insideArchive) {
                 fullPath = archiveCurrentPath.isEmpty()
                 ? selectedName
                 : archiveCurrentPath + "/" + selectedName;
             } else {
                 fullPath = currentPath.isEmpty()
                 ? selectedName
                 : currentPath + "/" + selectedName;
             }

             if (fullPath.startsWith("/")) {
                 fullPath = fullPath.mid(1);
             }

             selectedPathsSet.insert(fullPath);
         }

         if (selectedPathsSet.isEmpty()) {
             qDebug() << "No valid files selected.";
             return;
         }

         QString tempDir = QCoreApplication::applicationDirPath() + "/temp/";
         QDir dir;

         if (!dir.exists(tempDir)) {
             qDebug() << "Temp directory doesn't exist, creating it...";
             dir.mkpath(tempDir);
         }

         QMimeData *mimeData = new QMimeData;
         QList<QUrl> urls;

         MainWindow *mainWindow = qobject_cast<MainWindow *>(this->window());
         if (!mainWindow) {
             qDebug() << "Failed to find MainWindow for extraction.";
             return;
         }

         for (const QString &path : selectedPathsSet) {
             QString extractedFilePath = tempDir + QFileInfo(path).fileName();
             qDebug() << "Extracting file to:" << extractedFilePath;

             bool isFolder = false;

             if (insideArchive) {
                 // Проверяем, является ли элемент папкой в архиве
                 const auto &entries = archiveFileTree.value(archiveCurrentPath, QList<FileEntry>());
                 for (const FileEntry &entry : entries) {
                     if (entry.name == QFileInfo(path).fileName()) {
                         isFolder = entry.isFolder;
                         break;
                     }
                 }
             } else {
                 // Проверяем, является ли элемент папкой в файловой системе
                 const auto &entries = fileTree.value(currentPath, QList<FileEntry>());
                 for (const FileEntry &entry : entries) {
                     if (entry.name == QFileInfo(path).fileName()) {
                         isFolder = entry.isFolder;
                         break;
                     }
                 }
             }

             if (isFolder) {
                 if (insideArchive) {
                     mainWindow->extractFolder(path, tempDir);
                 } else {

                     mainWindow->extractFolder(path, tempDir);
                 }
             } else {
                 if (insideArchive) {
                     mainWindow->extractFile(path, tempDir);
                 } else {
                     mainWindow->extractFile(path, tempDir);
                 }
             }

             urls.append(QUrl::fromLocalFile(extractedFilePath));
         }

         mimeData->setUrls(urls);

         QDrag *drag = new QDrag(this);
         drag->setMimeData(mimeData);

         qDebug() << "Starting drag operation...";

         Qt::DropAction action = drag->exec(supportedActions);

         if (action == Qt::MoveAction || action == Qt::CopyAction) {
             qDebug() << "Drag operation successful!";
         } else {
             qDebug() << "Drag operation failed!";
         }

         dir.setPath(tempDir);
         dir.removeRecursively();
     }



     void mousePressEvent(QMouseEvent *event) override {
         if (!(event->modifiers() & Qt::ControlModifier)) {
             // Убираем выделение, если Ctrl не зажат
             clearSelection();
         }
         QTreeView::mousePressEvent(event);
     }
 };

 class CustomSortFilterProxyModel : public QSortFilterProxyModel {
 protected:
     bool lessThan(const QModelIndex &left, const QModelIndex &right) const override {
         if (left.column() == 2) { // Колонка "Size (bytes)"
             return left.data().toLongLong() < right.data().toLongLong();
         }
         return QSortFilterProxyModel::lessThan(left, right);
     }
 };

#endif // MAINWINDOW_H
