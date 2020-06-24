#pragma once

#include <QObject>
#include <memory>

class QTcpServer;
class QTcpSocket;
class CmdServer : public QObject
{
    Q_OBJECT
public:
    CmdServer(QObject *parent = nullptr);
    virtual ~CmdServer();
signals:
    void terminateApp();
    void handleCmd(const QStringList &args);
protected slots:
    void acceptConnection();
    void readClient();
    void onClientDisconnected();
protected:
    bool listen();

    std::shared_ptr<QTcpServer> m_server;
    std::shared_ptr<QTcpSocket> m_client;
    QByteArray m_readBuffer;
};

