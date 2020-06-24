#pragma once

#include <QObject>

class CmdHandler : public QObject
{
    Q_OBJECT
public:
    CmdHandler(QObject *parent = nullptr);
    virtual ~CmdHandler();
    void handleCmd(const QStringList &args);
};

