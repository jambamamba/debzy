#pragma once

#include <QWizardPage>

namespace Ui {
class LibraryGrabberWizardPage3;
}

class QStandardItemModel;
class LibraryGrabberWizardPage3 : public QWizardPage
{
    Q_OBJECT

public:
    explicit LibraryGrabberWizardPage3(QWidget *parent = 0);
    ~LibraryGrabberWizardPage3();
    void setPid(int pid);
    QStringList selectedLibraries() const;

protected:
    Ui::LibraryGrabberWizardPage3 *ui;
    QStandardItemModel *m_model;
};
