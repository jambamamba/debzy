#include "WaitDialog.h"
#include "ui_WaitDialog.h"
#include <QTimer>

WaitDialog::WaitDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WaitDialog)
    , m_timer(new QTimer(this))
    , m_idx(0)
{
    ui->setupUi(this);
    m_timer->setInterval(200);
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, &WaitDialog::animation);
    m_timer->start();
}

WaitDialog::~WaitDialog()
{
    delete ui;
}

void WaitDialog::animation()
{
    ui->breadLable->setPixmap(QPixmap::fromImage(QImage(
                                                     QString(":/resources/bread%1.png").arg(m_idx + 1))));
    m_idx = (m_idx + 1) % 3;
}

void WaitDialog::on_pushButtonCancel_clicked()
{
    emit cancel();
}

void WaitDialog::on_WaitDialog_rejected()
{
    emit cancel();
}
