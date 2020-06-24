#include "PrivilegedCmdDispatcher.h"

#include <QDebug>
#include <QTcpSocket>

PrivilegedCmdDispatcher::PrivilegedCmdDispatcher(QObject *parent)
    : QObject(parent)
    , m_socket(std::make_shared<QTcpSocket>(this))
{
    connect(m_socket.get(), &QTcpSocket::connected,
            this, &PrivilegedCmdDispatcher::connected);
    connect(m_socket.get(), &QTcpSocket::disconnected,
            this, &PrivilegedCmdDispatcher::disconnected);
    connect(m_socket.get(), &QIODevice::readyRead,
            this, &PrivilegedCmdDispatcher::onReadSocket);
}

PrivilegedCmdDispatcher::~PrivilegedCmdDispatcher()
{
}

void PrivilegedCmdDispatcher::connectToServer()
{
    const int DEFAULT_PORT = 55555;
    m_socket->connectToHost("127.0.0.1", DEFAULT_PORT);
}

void PrivilegedCmdDispatcher::onReadSocket()
{
    if(!m_socket)
    {
        return;
    }
    m_readBuffer.append(m_socket->read(m_socket->bytesAvailable()));
    QStringList lines = QString(m_readBuffer).split("\n");
    for(auto line : lines)
    {
        qDebug() << line;
        if(line == "done")
        {
            emit cmdProcessed(true);
            break;
        }
    }
    m_readBuffer.clear();
}

void PrivilegedCmdDispatcher::onDisconnected()
{
    qDebug() << "onDisconnected";
    closeConnection();
    emit cmdProcessed(false);
}

void PrivilegedCmdDispatcher::sendCmd(const QByteArray &cmd)
{
    if(!m_socket || !m_socket->isWritable())
    {
        return;
    }
    qDebug() << "sendCmd: " << QString(cmd);
    m_socket->write(cmd);
}

void PrivilegedCmdDispatcher::closeConnection()
{
    if(m_socket)
    {
        m_socket->close();
    }
    m_socket.reset();
}
