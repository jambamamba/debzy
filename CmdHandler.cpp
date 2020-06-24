#include "CmdHandler.h"
#include <QDebug>
#include <stdio.h>

CmdHandler::CmdHandler(QObject *parent)
    : QObject(parent)
{
    qDebug() << "CmdHandler ctor";

}

CmdHandler::~CmdHandler()
{
    qDebug() << "CmdHandler dtor";
}

void CmdHandler::handleCmd(const QStringList &args)
{
    qDebug() << "CmdHandler handleCmd" << args.join(" ");
}
