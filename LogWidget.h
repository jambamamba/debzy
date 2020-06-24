#pragma once

#include <QWidget>

namespace Ui {
class LogWidget;
}

class QPlainTextEdit;
class LogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LogWidget(QWidget *parent = 0);
    ~LogWidget();

    QPlainTextEdit *plainTextEdit();

private slots:
    void on_pushButtonClear_clicked();

private:
    Ui::LogWidget *ui;
};

