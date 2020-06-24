#include "CmdServer.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>

CmdServer::CmdServer(QObject* parent)
    : QObject(parent)
{
    qDebug() << "CmdServer ctor";
    listen();
}

CmdServer::~CmdServer()
{
    qDebug() << "CmdServer dtor";
    if(m_server)
    {
        m_server->close();
    }
}
bool CmdServer::listen()
{
    qDebug() << "CmdServer listen";

    m_server = std::make_shared<QTcpServer>();
    if (!m_server->listen(QHostAddress::Any, 55555))
    {
        qDebug() << QString("Unable to start the server: %1.")
                              .arg(m_server->errorString());
        m_server->close();
        return false;
    }
    qDebug() << "CmdServer listening";

    connect(m_server.get(), &QTcpServer::newConnection, this, &CmdServer::acceptConnection);
    return true;
}

void CmdServer::acceptConnection()
{
    qDebug() << "CmdServer accepted";

    m_client.reset(m_server->nextPendingConnection());
    connect(m_client.get(), &QTcpSocket::readyRead, this, &CmdServer::readClient);
    connect(m_client.get(), &QAbstractSocket::disconnected, this, &CmdServer::onClientDisconnected);

    m_client->write("Hello!\n");
    qDebug() << "Sending Hello! to newly connected client";
}

void CmdServer::onClientDisconnected()
{
    qDebug() << "CmdServer disconnected";

    if(m_client)
    {
        m_client->deleteLater();
        m_client = nullptr;
    }
}


void CmdServer::readClient()
{
    qDebug() << "CmdServer readClient";

    m_readBuffer.append(m_client->read(m_client->bytesAvailable()));

    QString msg(m_readBuffer);
    m_readBuffer.clear();

    auto lines = msg.split("\n");
    qDebug() << "readClient  " << lines;
    while(lines.size() > 0)
    {
        auto line = lines[0];
        if(line.compare("quit", Qt::CaseInsensitive) == 0 || line.compare("exit", Qt::CaseInsensitive) == 0)
        {
            m_client->disconnectFromHost();
            m_client->close();
            m_client = nullptr;
            emit terminateApp();
        }
        else if(line.startsWith("debmaker", Qt::CaseInsensitive))
        {
            auto args = line.split(" ");
            args.removeFirst();
            emit handleCmd(args);
        }
        else
        {
            m_readBuffer.append(line.toUtf8());
        }
        lines.removeFirst();
    }
}
