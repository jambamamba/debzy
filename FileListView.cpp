#include "FileListView.h"
#include "ui_FileListView.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QStringListModel>

FileListView::FileListView(QWidget *parent)
    : QListView(parent)
    , ui(new Ui::FileListView)
    , m_model(new QStringListModel(this))
{
    ui->setupUi(this);
    setModel(m_model);
    setAcceptDrops(true);
}

FileListView::~FileListView()
{
    delete ui;
}

const QStringList FileListView::getFileList() const
{
    return m_files;
}

void FileListView::clear()
{
    m_files.clear();
    m_model->setStringList(m_files);
}

void FileListView::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void FileListView::dragMoveEvent(QDragMoveEvent *event)
{

}

void FileListView::dropEvent(QDropEvent *e)
{
    //Todo: instead of m_filename, use

//    if info.isDir() is false, then target path is
//            ui->lineEditInstallDirectory->text() "/" info.fileName()
//          else if its a dir, then the target path is
//            ui->lineEditInstallDirectory->text() "/" info.fileP

    QString target_path("/usr/local/bin/myprogram");

    qDebug() << "dropped";
    m_files.clear();
    foreach (const QUrl &url, e->mimeData()->urls())
    {
        QString file_path = url.toLocalFile();
        QFileInfo info(file_path);
        QString file_name = info.fileName();
        qDebug() << "dropped" << file_path;
        if(info.isDir())
        {
            qDebug() << "\tcontents of dir:";
            QDirIterator it(file_path, QStringList(), QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext())
            {
                QString inode = it.next();
                qDebug() << "\t\t" << inode;
                QFileInfo info(inode);
                m_files << (file_path + ":" + target_path + "/" + file_name + "/" + info.fileName());
            }
        }
        else
        {
            m_files << (file_path + ":" + target_path + "/" + file_name);
        }
    }
    m_model->setStringList(m_files);
}
