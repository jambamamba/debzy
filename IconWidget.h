#pragma once

#include <QLabel>

namespace Ui {
class IconWidget;
}

class IconWidget : public QLabel
{
    Q_OBJECT

signals:

public:
    explicit IconWidget(QWidget *parent = 0);
    ~IconWidget();

    QString iconFile() const;
    void setIconFile(const QString &icon_file);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *e) override;
    virtual void dropEvent(QDropEvent *e) override;
    virtual void dragMoveEvent(QDragMoveEvent *event);

private slots:
    void on_pushButtonClearIcon_clicked();

    void on_pushButtonSelectIconFile_clicked();

private:
    Ui::IconWidget *ui;
    QString m_fileName;
};

