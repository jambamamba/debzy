#ifndef WAITDIALOG_H
#define WAITDIALOG_H

#include <QDialog>

namespace Ui {
class WaitDialog;
}

class WaitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaitDialog(QWidget *parent = 0);
    ~WaitDialog();

signals:
    void cancel();
private slots:
    void animation();
    void on_pushButtonCancel_clicked();
    void on_WaitDialog_rejected();

private:
    Ui::WaitDialog *ui;
    QTimer *m_timer;
    int m_idx;
};

#endif // WAITDIALOG_H
