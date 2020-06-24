#include "FindReplaceWidget.h"
#include "ui_FindReplaceWidget.h"

FindReplaceWidget::FindReplaceWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindReplaceWidget)
{
    ui->setupUi(this);
}

FindReplaceWidget::~FindReplaceWidget()
{
    delete ui;
}

void FindReplaceWidget::on_buttonBox_accepted()
{
    emit findAndReplace(ui->lineEditFind->text(), ui->lineEditReplace->text());
}
