#include "StartupScript.h"
#include "ui_StartupScript.h"

#include <QString>
#include <QDebug>

StartupScript::StartupScript(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StartupScript)
{
    ui->setupUi(this);
}

StartupScript::~StartupScript()
{
    delete ui;
}

QString StartupScript::script() const
{
    return ui->plainTextEdit->toPlainText();
}

QString StartupScript::dataAsJsonString() const
{
    return QString(ui->plainTextEdit->toPlainText().toUtf8().toBase64());
}

void StartupScript::setData(const QString &script)
{
    QByteArray ba;
    ui->plainTextEdit->setPlainText(QString(ba.fromBase64(script.toUtf8())));
}
