#include "FileListWidget.h"
#include "ui_FileListView.h"

#include <QDebug>
#include <QAbstractItemView>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QStringListModel>
#include <QItemSelectionModel>

#include "FileListModel.h"
#include "FindReplaceWidget.h"
#include "LibraryGrabberWizard.h"

namespace {


}//namespace

FileListWidget::FileListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileListWidget)
    , m_model(new FileListModel(this))
    , m_progress(0)
    , m_progress_max(1)
    , m_wizard(new LibraryGrabberWizard)
    , m_find_replace(new FindReplaceWidget(this))
{
    ui->setupUi(this);
    ui->fileListView->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);
    ui->fileListView->setModel(m_model);
    setAcceptDrops(true);

    connect(m_wizard, &LibraryGrabberWizard::selectedLibraries, this, &FileListWidget::addFiles);
    connect(m_model, &FileListModel::listDataChanged, this, &FileListWidget::onListDataChanged);
    connect(m_find_replace, &FindReplaceWidget::findAndReplace, this, &FileListWidget::onFindAndReplace);
}

FileListWidget::~FileListWidget()
{
    delete ui;
    delete m_wizard;
}

const QStringList FileListWidget::fileList() const
{
    return m_files;
}

QString FileListWidget::installDirectory() const
{
    return ui->lineEditInstallDirectory->text();
}


void FileListWidget::clear()
{
    m_files.clear();
    m_model->setStringList(m_files);
}

void FileListWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void FileListWidget::dragMoveEvent(QDragMoveEvent *)
{

}

QStringList FileListWidget::iterateFiles(const QString &prefix_path, const QString &source_path, const QString &target_path) const
{
    QFileInfo info(source_path);

    if(!info.isDir())
    {
        QString map(source_path + "->" + target_path + "/" + info.fileName());
        qDebug() << map;
        return QStringList() << map;
    }
    else
    {
        QStringList list;
        QDirIterator it(source_path, QStringList(), QDir::Files, QDirIterator::Subdirectories);
        while(it.hasNext())
        {
            QString inode = it.next();
            int i = inode.indexOf(prefix_path);
            if(i > -1)
            {
                QString n(inode);
                QString target_path2 = target_path + "/" + QFileInfo(prefix_path).fileName() + n.remove(i, prefix_path.length());
                QString map(inode + "->" + target_path2);
                qDebug() << map;
                list << map;

                if(m_progress > -1)
                {
                    emit progress((++m_progress) * 100/m_progress_max);
                    QApplication::processEvents();
                }
            }
        }
        return list;
    }
    return QStringList();
}

void FileListWidget::dropEvent(QDropEvent *e)
{
    if(ui->lineEditInstallDirectory->text().isEmpty())
    {
        qDebug() << "Need the install directory";
        return;
    }

    QStringList files;
    for(auto &url : e->mimeData()->urls())
    {
        files.append(url.toLocalFile());
    }
    addFiles(files);
}


void FileListWidget::on_pushButtonSelectFiles_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this,
                                                          tr("Add files to package"),
                                                          QDir::currentPath(),
                                                          tr("All files (*.*)") );
    addFiles(files);
}

void FileListWidget::addFiles(const QStringList &files)
{
    //dry run to get a count
    m_progress = -1;
    m_progress_max = 0;
    foreach (const QString &file_path, files)
    {
        m_progress_max += iterateFiles(file_path, file_path, ui->lineEditInstallDirectory->text() + "/<package_name>").count() + 1;
    }

    //run again, then update
    m_progress = 0;
    emit progress(0);
    QApplication::processEvents();

    foreach (const QString &file_path, files)
    {
        m_files << iterateFiles(file_path, file_path, ui->lineEditInstallDirectory->text() + "/<package_name>");
        m_progress += m_files.count() + 1;

        emit progress((++m_progress) * 100/m_progress_max);
        QApplication::processEvents();
    }

    emit progress(100);
    QApplication::processEvents();

    m_model->setStringList(m_files);
}

void FileListWidget::onListDataChanged(int row, const QString &value)
{
    qDebug() << "list data changed2" << row << value;
    m_files[row] = value;
}

void FileListWidget::on_pushButtonClearFiles_clicked()
{
    QStringList list;
    for(auto i : ui->fileListView->selectionModel()->selectedIndexes())
    {
        list.append(ui->fileListView->model()->data(i, Qt::DisplayRole).toString());
    }
    for(auto sel : list)
    {
        m_files.removeOne(sel);
    }
    m_model->setStringList(m_files);

}

QString FileListWidget::dataAsJsonString() const
{
    QString json("[");
    int i = 0;
    for(auto file : m_files)
    {
        if(i > 0) { json.append(","); }
        json.append("\"").append(file).append("\"");
        i++;
    }
    json.append("]");
    return json;
}

void FileListWidget::setData(const QStringList &files)
{
    m_files = files;
    m_model->setStringList(m_files);
}


void FileListWidget::on_pushButtonLibraryGrabber_clicked()
{
    m_wizard->show();
}

void FileListWidget::on_lineEditFileFilter_textChanged(const QString &arg1)
{
    if(arg1.isEmpty())
    {
        m_model->setStringList(m_files);
        return;
    }
    QStringList files;
    for(auto& file : m_files)
    {
        if(file.contains(arg1))
        {
            files.append(file);
        }
        m_model->setStringList(files);
    }
}

void FileListWidget::on_pushButtonFindReplace_clicked()
{
    m_find_replace->show();
}

void FileListWidget::onFindAndReplace(const QString &find, const QString &replace)
{
    for(auto& file : m_files)
    {
        if(file.contains(find))
        {
            file = file.replace(find, replace);
        }
    }
    m_model->setStringList(m_files);
}
