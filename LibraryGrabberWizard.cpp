#include "LibraryGrabberWizard.h"

#include <QDebug>

#include "LibraryGrabberWizardPage1.h"
#include "LibraryGrabberWizardPage2.h"
#include "LibraryGrabberWizardPage3.h"

namespace {


}
LibraryGrabberWizard::LibraryGrabberWizard(QWidget *parent)
    : QWizard(parent)
{
    page1 = new LibraryGrabberWizardPage1;
    addPage(page1);

    page2 = new LibraryGrabberWizardPage2;
    addPage(page2);

    page3 = new LibraryGrabberWizardPage3;
    addPage(page3);

    setWindowTitle("Library Helper");
}

LibraryGrabberWizard::~LibraryGrabberWizard()
{
}

void LibraryGrabberWizard::initializePage(int id)
{
    if(page(id) == page1)
    {
        m_libraries.clear();
    }
    else if(page(id) == page2)
    {
        page2->loadTableView();
    }
}

bool LibraryGrabberWizard::validateCurrentPage()
{
    if(currentPage() == page2)
    {
       int pid = page2->selectedPid();
       if(pid > -1)
       {
           page3->setPid(pid);
           return true;
       }
       else
       {
           return false;
       }
    }
    else if(currentPage() == page3)
    {
        m_libraries = page3->selectedLibraries();
    }
    return true;
}

void LibraryGrabberWizard::accept()
{
    QWizard::accept();
    emit selectedLibraries(m_libraries);
}

