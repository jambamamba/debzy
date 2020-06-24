#pragma once

#include <QListView>

namespace Ui {
class FileListView;
}

class QStringListModel;
class FileListView : public QListView
{
    Q_OBJECT

public:
    explicit FileListView(QWidget *parent = 0);
    ~FileListView();

    const QStringList getFileList() const;
    void clear();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *e) override;
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *e) override;

private:
    Ui::FileListView *ui;
    QStringList m_files;
    QStringListModel *m_model;
};
