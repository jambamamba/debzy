#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <QWidget>

namespace Ui {
class ControlWidget;
}

class ControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ControlWidget(QWidget *parent = 0);
    ~ControlWidget();

    QString controlString(const QString &icon_file) const;
    QString menuCategory() const;
    QString projectName() const;
    QString packageName() const;
    QString version() const;
    QString description() const;
    QString dataAsJsonString() const;
    void setData(const QVariantMap &map);
signals:
    void loadProject(const QString &path);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *e) override;
    virtual void dragMoveEvent(QDragMoveEvent *);
    virtual void dropEvent(QDropEvent *e) override;

private:
    Ui::ControlWidget *ui;
};

#endif // CONTROLWIDGET_H
