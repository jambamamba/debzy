#pragma once

#include <QLabel>

namespace Ui {
class IconView;
}

class IconView : public QLabel
{
    Q_OBJECT

signals:

public:
    explicit IconView(QWidget *parent = 0);
    ~IconView();

    QString iconFile() const;

protected:
    virtual void dragEnterEvent(QDragEnterEvent *e) override;
    virtual void dropEvent(QDropEvent *e) override;
    virtual void dragMoveEvent(QDragMoveEvent *event);

private:
    Ui::IconView *ui;
    QString m_fileName;
};

