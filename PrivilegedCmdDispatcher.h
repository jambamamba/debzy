#pragma once

#include <QObject>
#include <QAbstractSocket>
#include <QUrl>
#include <memory>

class QTcpSocket;
class PrivilegedCmdDispatcher : public QObject
{
    Q_OBJECT
public:
    PrivilegedCmdDispatcher(QObject *parent);
    virtual ~PrivilegedCmdDispatcher();
    void connectToServer();
    void sendCmd(const QByteArray &cmd);

signals:
    void connected();
    void disconnected();
    void cmdProcessed(bool);

public slots:

protected slots:
    void onReadSocket();
    void onDisconnected();

protected:
    std::shared_ptr<QTcpSocket> m_socket;
    QByteArray m_readBuffer;

    void closeConnection();
};
