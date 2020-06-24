#pragma once

#include <QMap>
#include <QWizard>

class LibraryGrabberWizardPage1;
class LibraryGrabberWizardPage2;
class LibraryGrabberWizardPage3;
class LibraryGrabberWizard : public QWizard
{
    Q_OBJECT
public:
    LibraryGrabberWizard(QWidget *parent = nullptr);
    virtual ~LibraryGrabberWizard();

signals:
    void selectedLibraries(const QStringList& libraries);

protected:
    virtual void initializePage(int id);
    virtual bool validateCurrentPage();
    virtual void accept();

    LibraryGrabberWizardPage1 *page1;
    LibraryGrabberWizardPage2 *page2;
    LibraryGrabberWizardPage3 *page3;
    QStringList m_libraries;
};
