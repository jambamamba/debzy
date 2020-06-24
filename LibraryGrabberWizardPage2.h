#ifndef LIBRARYGRABBERWIZARDPAGE2_H
#define LIBRARYGRABBERWIZARDPAGE2_H

#include <QWizardPage>

namespace Ui {
class LibraryGrabberWizardPage2;
}

class QStandardItemModel;
class LibraryGrabberWizardPage2 : public QWizardPage
{
    Q_OBJECT

public:
    explicit LibraryGrabberWizardPage2(QWidget *parent = 0);
    ~LibraryGrabberWizardPage2();
    int selectedPid() const;
    void loadTableView();
protected:
    Ui::LibraryGrabberWizardPage2 *ui;
    QStandardItemModel *m_model;
};

#endif // LIBRARYGRABBERWIZARDPAGE2_H
