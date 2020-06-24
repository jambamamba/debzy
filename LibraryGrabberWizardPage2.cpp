#include "LibraryGrabberWizardPage2.h"
#include "ui_LibraryGrabberWizardPage2.h"

#include <QStandardItem>
#include <QDebug>
#include <QApplication>
#include <QDir>

namespace {

QMap<int, QString> runningProcesses()
{
    QMap <int, QString> running_procs;
    QDir proc_dir("/proc/");
    QStringList files = proc_dir.entryList();
    for(QString &file : files)
    {
        bool ok = false;
        int pid = file.toInt(&ok);
        if(!ok)
        {
            continue;
        }
        QFile exe(QString("/proc/%1/exe").arg(pid));
        if(!exe.open(QIODevice::ReadOnly))
        {
            continue;
        }
        QString path = exe.symLinkTarget();
        if(path.isEmpty())
        {
            continue;
        }
        auto tokens = path.split("/");
        if(tokens.count() == 0)
        {
            continue;
        }
        running_procs[pid] = tokens.last();
    }
    return running_procs;
}
}//namespace
LibraryGrabberWizardPage2::LibraryGrabberWizardPage2(QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::LibraryGrabberWizardPage2)
    , m_model(new QStandardItemModel(1, 2, parent))
{
    ui->setupUi(this);
    m_model->setHorizontalHeaderItem(0, new QStandardItem("PID"));
    m_model->setHorizontalHeaderItem(1, new QStandardItem("Exe"));
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setModel(m_model);
}

LibraryGrabberWizardPage2::~LibraryGrabberWizardPage2()
{
    delete ui;
}

void LibraryGrabberWizardPage2::loadTableView()
{
    auto list = runningProcesses();
    int i = 0;
    for(auto pid : list.keys())
    {
        QStandardItem *item = new QStandardItem(QString::number(pid));
//        item->setCheckable(true);
//        item->setCheckState(Qt::Unchecked);
        m_model->setItem(i, 0, item);
        item = new QStandardItem(list[pid]);
        m_model->setItem(i, 1, item);
        ++i;
    }
}

int LibraryGrabberWizardPage2::selectedPid() const
{
    QItemSelectionModel *select = ui->tableView->selectionModel();
    if(!select->hasSelection())
    {
        return -1;
    }
    auto idx = select->selectedRows().first();
    return m_model->data(idx).toInt();
}
