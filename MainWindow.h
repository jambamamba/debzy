#pragma once

#include <QMainWindow>
#include <memory>

namespace Ui {
class MainWindow;
}

class WaitDialog;
class QProcess;
class PrivilegedCmdDispatcher;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void progress(int percent);
    void loadProjectFromFile(const QString &path);

protected slots:
    void dpkgDebProcessOutput();
    void dpkgDebProcessFinished();
    void loadProject();
    void loadLastProject();
    void saveProject();
    void saveProjectAs();
    void processCancelled();
    void on_pushButtonMakeDeb_clicked();

protected:
    Ui::MainWindow *ui;
    QProcess *m_dpkg_deb;
    WaitDialog *m_wait_dlg;
    QString m_loaded_file;
    std::unique_ptr<PrivilegedCmdDispatcher> m_cmd_dispatcher;
    bool m_connected_to_privileged_child;
    QString m_cmd;
    QString m_package_name;

    void saveProject(const QString &path, const QString &json);
    QString projectAsJson();
    void saveRecentFilePath(const QString &path);
    QString readRecentFilePath() const;
    bool chmod755(const QString &project_path, int file_count);
    void startDpkgDeb(const QString &package_name);
    virtual void closeEvent(QCloseEvent *event) override;
};

