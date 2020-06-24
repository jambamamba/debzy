#pragma once

#include <QObject>
#include <QProcess>
#include <memory>

class PrivilegedCmdDispatcher;
class PrivilegedProcessLauncher : public QObject
{
    //Make sure geteuid()=0 and getuid()=1000
    //If above is not true, then
    //sudo chown root:root debmaker
    //sudo chmod 4755 debmaker
    //./debmaker
    //Then startChrootProcess()
    //Then drop to normal user by calling setuid(getuid());
    //chroot process will keep running as root user

    Q_OBJECT
signals:
    void startChildProcess();

public:
    PrivilegedProcessLauncher();
    ~PrivilegedProcessLauncher();
    bool IsSetuidBitSet() const;
    std::shared_ptr<QProcess> process() const;
public slots:
    void onStartChildProcess();
protected slots:
    void dropToNormalUser() const;
    void processStarted();
    void processOutput();
    void processFinished();
    void errorOccurred(QProcess::ProcessError error);
protected:
    std::shared_ptr<QProcess> m_process;
    std::unique_ptr<PrivilegedCmdDispatcher> m_cmd;
};
