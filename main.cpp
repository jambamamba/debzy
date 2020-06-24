#include "MainWindow.h"

#include <QApplication>
#include <QDebug>
#include <unistd.h>

#include "CmdHandler.h"
#include "PrivilegedCmdDispatcher.h"
#include "TcpServer.h"

/*
 *
 To Run:

sudo rm -fr $HOME/Documents
sudo rm -fr /opt/debmaker
sudo rm -fr /opt/debzy
sudo cp debmaker debzy && sudo chown root:root debzy && sudo chmod 4755 debzy && QT_QPA_PLATFORM_PLUGIN_PATH=$HOME/Qt/5.11.2/gcc_64/plugins/platforms QT_DEBUG_PLUGINS=1 ./debzy &

...

 */
namespace
{
bool IsSetuidBitSet()
{
    qDebug() << "geteuid()" << geteuid() << "getuid()" << getuid();
    return (geteuid() == 0 && getuid() != 0);
}
void dropToNormalUser()
{
    setuid(getuid());
}

bool runningWithSetuidBitSet()
{
    if(!IsSetuidBitSet())
    {
        qDebug() << "You must cd to buildDir, and run \"sudo chown root:root debmaker\"";
        qDebug() << "followed by \"sudo chmod 4755 debmaker\"";
        qDebug() << "Do not forget to create this symlink";
        qDebug() << "ln -s /home/dev/oosman/Qt5.12.1/Tools/QtCreator/lib/Qt/plugins/platforms platforms";
        qDebug() << "Use this run command";
        qDebug() << "LD_LIBRARY_PATH=/home/dev/oosman/Qt5.12.1/5.12.1/gcc_64/lib:/home/dev/oosman/Qt5.12.1/5.12.1/gcc_64/lib/:/home/dev/oosman/work.git/libs/build/linux/bin/ ./debmaker";
        return false;
    }

    auto pid = fork();
    if(pid < 0)
    {
        qDebug() << "Failed to fork child process";
        return false;
    }
    else if(pid == 0) // child process
    {
        QCoreApplication::setSetuidAllowed(true);
        qDebug() << "child process 1";

        TcpServer server;
        server.run();

        return false;
    }

    dropToNormalUser();
    return true;
}
}//namespace

int main(int argc, char *argv[])
{
    QCoreApplication::setSetuidAllowed(true);

    if(!runningWithSetuidBitSet())
    {
        return 0;
    }

    QApplication a(argc, argv);
    MainWindow w;
    if(argc > 1 && QString(argv[1]).endsWith(".deb.mkr"))
    {
        w.loadProjectFromFile(argv[1]);
    }
    w.show();

    return a.exec();
}
