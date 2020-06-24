#include "LibraryGrabberWizardPage3.h"
#include "ui_LibraryGrabberWizardPage3.h"

#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QStandardItem>

namespace {

QStringList loadedLibraries(int pid)
{
    QStringList libs;
    QFile maps(QString("/proc/%1/maps").arg(pid));
    if(!maps.open(QIODevice::ReadOnly))
    {
        return libs;
    }
    for(QString line : maps.readAll().split('\n'))
    {
        QStringList tokens = line.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
        if(tokens.count() == 6)
        {
            QString file = tokens.at(5);
            if(file.contains(".so") && !libs.contains(file))
            {
                libs.append(file);
            }
        }
    }

    return libs;
}
}//namespace

LibraryGrabberWizardPage3::LibraryGrabberWizardPage3(QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::LibraryGrabberWizardPage3)
    , m_model(new QStandardItemModel(1, 1, parent))
{
    ui->setupUi(this);
    m_model->setHorizontalHeaderItem(0, new QStandardItem("Library"));
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setModel(m_model);
}

LibraryGrabberWizardPage3::~LibraryGrabberWizardPage3()
{
    delete ui;
}

void LibraryGrabberWizardPage3::setPid(int pid)
{
    //cat /proc/15407/maps | awk '{print $6}' | grep '\.so' | sort | uniq

    auto list = loadedLibraries(pid);
    int i = 0;
    for(auto file : list)
    {
        QStandardItem *item = new QStandardItem(file);
//        item->setCheckable(true);
//        item->setCheckState(Qt::Unchecked);
        m_model->setItem(i, 0, item);
        ++i;
    }
}

QStringList LibraryGrabberWizardPage3::selectedLibraries() const
{
    QStringList files;
    QItemSelectionModel *select = ui->tableView->selectionModel();
    if(!select->hasSelection())
    {
        return files;
    }
    for(auto idx: select->selectedRows())
    {
        files.append(m_model->data(m_model->index(idx.row(), 0), Qt::DisplayRole).toString());
    }
    return files;
}
