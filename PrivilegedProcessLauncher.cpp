#include "PrivilegedProcessLauncher.h"

#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <memory>

#include "PrivilegedCmdDispatcher.h"

PrivilegedProcessLauncher::PrivilegedProcessLauncher()
    : QObject(nullptr)
    , m_cmd(std::make_unique<PrivilegedCmdDispatcher>(this))
{
    connect(this, &PrivilegedProcessLauncher::startChildProcess,
            this, &PrivilegedProcessLauncher::onStartChildProcess);
    connect(m_cmd.get(), &PrivilegedCmdDispatcher::connected,
            [this](){
        m_cmd->sendCmd(QString("foo").toUtf8());
    });
}
PrivilegedProcessLauncher::~PrivilegedProcessLauncher()
{
}
bool PrivilegedProcessLauncher::IsSetuidBitSet() const
{
    qDebug() << "geteuid()" << geteuid() << "getuid()" << getuid();
    return (geteuid() == 0 && getuid() != 0);
}
void PrivilegedProcessLauncher::onStartChildProcess()
{
    m_process = std::make_shared<QProcess>(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process.get(), &QProcess::readyReadStandardOutput,
            this, &PrivilegedProcessLauncher::processOutput);
    connect(m_process.get(), &QProcess::readyReadStandardError,
            this, &PrivilegedProcessLauncher::processOutput);
    connect(m_process.get(), static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),
            this, &PrivilegedProcessLauncher::processFinished);
    connect(m_process.get(), static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred),
            this, &PrivilegedProcessLauncher::errorOccurred);

    connect(m_process.get(), &QProcess::started,
            this, &PrivilegedProcessLauncher::processStarted);

    QStringList args;

    qDebug() << "starting chroot from parent";
    m_process->start("/home/oosman/work/debmaker/build-chroot-Desktop_Qt_5_10_0_GCC_64bit-Debug/chroot", args);
    dropToNormalUser();
}
void PrivilegedProcessLauncher::dropToNormalUser() const
{
    setuid(getuid());
}

std::shared_ptr<QProcess> PrivilegedProcessLauncher::process() const
{
    return m_process;
}

void PrivilegedProcessLauncher::processStarted()
{
    m_cmd->connectToServer();
}

void PrivilegedProcessLauncher::errorOccurred(QProcess::ProcessError error)
{

}
void PrivilegedProcessLauncher::processOutput()
{
    auto output = m_process->readAllStandardOutput();
    auto error = m_process->readAllStandardError();
    if(!error.isEmpty()) { qDebug() << error; }
    if(!output.isEmpty()) { qDebug() << output; }
}

void PrivilegedProcessLauncher::processFinished()
{
    qDebug() << "Process finished";
    m_process->deleteLater();
    m_process = nullptr;
}
