#ifndef FINDREPLACEWIDGET_H
#define FINDREPLACEWIDGET_H

#include <QDialog>

namespace Ui {
class FindReplaceWidget;
}

class FindReplaceWidget : public QDialog
{
    Q_OBJECT

public:
    explicit FindReplaceWidget(QWidget *parent = 0);
    ~FindReplaceWidget();

signals:
    void findAndReplace(const QString &, const QString &);
private slots:
    void on_buttonBox_accepted();

private:
    Ui::FindReplaceWidget *ui;
};

#endif // FINDREPLACEWIDGET_H
