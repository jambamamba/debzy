#include "LibraryGrabberWizardPage1.h"
#include "ui_LibraryGrabberWizardPage1.h"

LibraryGrabberWizardPage1::LibraryGrabberWizardPage1(QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::LibraryGrabberWizardPage1)
{
    ui->setupUi(this);
}

LibraryGrabberWizardPage1::~LibraryGrabberWizardPage1()
{
    delete ui;
}

