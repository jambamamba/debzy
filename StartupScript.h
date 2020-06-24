#ifndef STARTUPSCRIPT_H
#define STARTUPSCRIPT_H

#include <QWidget>

namespace Ui {
class StartupScript;
}

class StartupScript : public QWidget
{
    Q_OBJECT

public:
    explicit StartupScript(QWidget *parent = nullptr);
    ~StartupScript();

    QString script() const;
    QString dataAsJsonString() const;
    void setData(const QString &script);

private:
    Ui::StartupScript *ui;
};

#endif // STARTUPSCRIPT_H
