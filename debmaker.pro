#-------------------------------------------------
#
# Project created by QtCreator 2017-11-24T20:43:30
#
#-------------------------------------------------

QT       += core gui network
CONFIG += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = debmaker
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    FileListWidget.cpp \
    ControlWidget.cpp \
    IconWidget.cpp \
    LogWidget.cpp \
    CmdHandler.cpp \
    LibraryGrabberWizardPage1.cpp \
    LibraryGrabberWizardPage2.cpp \
    LibraryGrabberWizardPage3.cpp \
    LibraryGrabberWizard.cpp \
    WaitDialogitdialog.cpp \
    FileListModel.cpp \
    FindReplaceWidget.cpp \
    PrivilegedCmdDispatcher.cpp \
    TcpServer.cpp \
    StartupScript.cpp

HEADERS  += MainWindow.h \
    FileListWidget.h \
    ControlWidget.h \
    IconWidget.h \
    LogWidget.h \
    CmdHandler.h \
    LibraryGrabberWizardPage1.h \
    LibraryGrabberWizardPage2.h \
    LibraryGrabberWizardPage3.h \
    LibraryGrabberWizard.h \
    WaitDialog.h \
    FileListModel.h \
    FindReplaceWidget.h \
    PrivilegedCmdDispatcher.h \
    TcpServer.h \
    StartupScript.h

FORMS    += MainWindow.ui \
    FileListView.ui \
    ControlWidget.ui \
    IconWidget.ui \
    LogWidget.ui \
    LibraryGrabberWizardPage1.ui \
    LibraryGrabberWizardPage2.ui \
    LibraryGrabberWizardPage3.ui \
    WaitDialog.ui \
    FindReplaceWidget.ui \
    StartupScript.ui

RESOURCES += \
    debmaker.qrc
