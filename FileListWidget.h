#pragma once

#include <QWidget>

namespace Ui {
class FileListWidget;
}

class FileListModel;
class FindReplaceWidget;
class LibraryGrabberWizard;
class QStringListModel;
class FileListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileListWidget(QWidget *parent = 0);
    ~FileListWidget();

    const QStringList fileList() const;
    QString installDirectory() const;
    QString dataAsJsonString() const;
    void setData(const QStringList &files);
    void clear();

signals:
    void progress(int) const;

protected:
    QStringList iterateFiles(const QString &prefix_path, const QString &source_path, const QString &target_path) const;
    virtual void dragEnterEvent(QDragEnterEvent *e) override;
    virtual void dragMoveEvent(QDragMoveEvent *);
    virtual void dropEvent(QDropEvent *e) override;

protected slots:
    void on_pushButtonClearFiles_clicked();
    void addFiles(const QStringList &files);
    void onListDataChanged(int row, const QString &value);
    void onFindAndReplace(const QString &find, const QString &replace);

protected:
    Ui::FileListWidget *ui;
    QStringList m_files;
    FileListModel *m_model;
    mutable int m_progress;
    mutable int m_progress_max;
    LibraryGrabberWizard *m_wizard;
    FindReplaceWidget *m_find_replace;
private slots:
    void on_pushButtonSelectFiles_clicked();
    void on_pushButtonLibraryGrabber_clicked();
    void on_lineEditFileFilter_textChanged(const QString &arg1);
    void on_pushButtonFindReplace_clicked();
};
