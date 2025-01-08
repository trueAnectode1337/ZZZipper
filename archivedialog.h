#ifndef ARCHIVEDIALOG_H
#define ARCHIVEDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace Ui {
class ArchiveDialog;
}

class ArchiveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ArchiveDialog(QWidget *parent = nullptr);
    QString getOutputPath() const;
    QString getFormat() const;
    QString getCompressionLevel() const;
    QStringList getSelectedFiles() const;
    ~ArchiveDialog();
private slots:
    void selectOutputPath();
    void addFiles();
    void addFolders();
    void removeSelected();
private:
    Ui::ArchiveDialog *ui;
    QLineEdit *outputPathEdit;
    QComboBox *formatComboBox;
    QComboBox *compressionComboBox;
    QTreeWidget *fileListWidget;
};

#endif // ARCHIVEDIALOG_H
