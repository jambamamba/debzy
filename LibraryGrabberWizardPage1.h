#pragma once

#include <QWizardPage>

namespace Ui {
class LibraryGrabberWizardPage1;
}

class QStandardItemModel;
class LibraryGrabberWizardPage1 : public QWizardPage
{
    Q_OBJECT

public:
    explicit LibraryGrabberWizardPage1(QWidget *parent = 0);
    ~LibraryGrabberWizardPage1();

protected:
    Ui::LibraryGrabberWizardPage1 *ui;
};
