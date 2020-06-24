#include "MainWindow.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <QDebug>
#include <QStandardPaths>
#include <QPlainTextEdit>
#include <QDir>
#include <QProcess>
#include <QDesktopServices>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>
#include <sys/stat.h>
#include "ui_MainWindow.h"

#include "PrivilegedCmdDispatcher.h"
#include "WaitDialog.h"

//to remove old menu items
// cd /usr/share/applications
// sudo rm whatever.desktop
//you have to re-login

namespace {

const QString APP_NAME("DebMaker");
static QtMessageHandler s_message_handler;
static QStatusBar *s_status_bar;
static QPlainTextEdit *s_log_view;

struct Progress
{
    Progress(QProgressBar *bar)
        : m_bar(bar)
    {
        m_bar->setValue(0);
    }
    ~Progress()
    {
        m_bar->setValue(100);
    }
    QProgressBar *m_bar;
};

QString appDirectory()
{
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + APP_NAME;
}

QString iniFilePath()
{
    return appDirectory() + "/" + APP_NAME + ".ini";
}

QString createProjectDirectory(const QString &project)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QDir doc;
    if(!QFileInfo(path).exists())
    {
        doc.mkpath(path);
    }

    QDir dir(path);
    dir.mkdir(APP_NAME);
    path = path + "/" + APP_NAME;
    dir = QDir(path);
    dir.mkdir(project);
    dir.mkdir(project + "/DEBIAN");
    if(!QDir(dir.absolutePath() + "/" + project + "/DEBIAN").exists())
    {
        return "";
    }
    path = path + "/" + project;
    return path;
}

QString writeControlFile(const QString &project_path, const QString &control_string)
{
    QString control_file_path(project_path + "/DEBIAN/control");
    QFile f(control_file_path);
    if(f.open(QIODevice::WriteOnly))
    {
        f.write(control_string.toUtf8().data(), control_string.length());
        f.close();
        return control_file_path;
    }
    return "";
}

QString makeTargetDirectories(const QString &project_path, const QString &target_path)
{
    QDir dir(project_path);
    QStringList tokens = target_path.split("/");
    for(auto token : tokens)
    {
        if(token.isEmpty())
        {
            continue;
        }
        qDebug() << "make" << dir.absolutePath() + "/" + token;
        dir.mkdir(token);
        dir = QDir(dir.absolutePath() + "/" + token);
        if(!dir.exists())
        {
            qDebug() << "failed";
            return "";
        }
    }
    return dir.absolutePath();
}

QStringList fileListWithCorrectPackageName(const QString &package_name, const QStringList &file_list)
{
    QStringList return_list;
    for(QString file : file_list)
    {
        return_list.append(file.replace("<package_name>", package_name));
    }
    return return_list;
}

bool chrpath(const QString &install_dir, const QString &target_file)
{
    QFile f("/usr/bin/chrpath");
    if(!f.exists())
    {
        qDebug() << "You need chrpath, run command: sudo apt-get install chrpath";
        return false;
    }
    QProcess chrpath;
    //chrpath -r /usr/local/bin/bf-booby-screen-recorder ScreenRecorder
    QStringList args;
    args << "-r" << install_dir << target_file;
    qDebug() << "chrpath -r" << install_dir << target_file;
    chrpath.start("/usr/bin/chrpath", args);
    chrpath.waitForFinished();

    return true;
}

void moveLibqxcbToPlatforms(
        const QString &target_path,
        const QString &package_name)
{
    QDir dir(target_path + "/" + package_name);
    dir.mkdir("platforms");
    QFile file(target_path + "/" + package_name + "/libqxcb.so");
    QString dst(target_path + "/" + package_name + "/platforms/libqxcb.so");
    if(!file.rename(dst))
    {
        return;
    }

    chrpath("$ORIGIN/../", dst);
}

void moveImageLibsToImageformats(
        const QString &target_path,
        const QString &package_name)
{
    QDir dir(target_path + "/" + package_name);
    dir.mkdir("imageformats");

    auto imagelibs = QStringList()
        <<"libqgif.so"
        <<"libqicns.so"
        <<"libqico.so"
        <<"libqjpeg.so"
        <<"libqsvg.so"
        <<"libqtga.so"
        <<"libqtiff.so"
        <<"libqwbmp.so"
        <<"libqwebp.so";

    for(const auto &sofile : imagelibs)
    {
        QFile file(target_path + "/" + package_name + "/" + sofile);
        QString dst(target_path + "/" + package_name + "/imageformats/" + sofile);
        if(!file.rename(dst))
        {
            continue;
        }
        chrpath("$ORIGIN/../", dst);
    }
}

bool ldconfigsymlinks(const QString &install_dir)
{

    QFile f("/sbin/ldconfig");
    if(!f.exists())
    {
        return false;
    }
    QProcess ldconfig;
     QStringList args;
    args << "-nN" << install_dir;
    qDebug() << "ldconfig -Nn " << install_dir;
    ldconfig.start("/sbin/ldconfig", args);
    ldconfig.waitForFinished();

    return true;
}
bool strip(const QString &file)
{
    QFile f("/usr/bin/strip");
    if(!f.exists())
    {
        return false;
    }
    QProcess strip;
    qDebug() << "strip " << file;
    strip.start("/usr/bin/strip", QStringList() << file);
    strip.waitForFinished();

    return true;
}
bool copyFiles(const QString &project_path,
               const QStringList &files,
               QProgressBar *progress)
{
    Progress p(progress);
    int count = 0;
    for(auto line : files)
    {
        auto tokens = line.split("->");
        if(tokens.count() !=2 )
        {
            qDebug() << "invalid tokens";
            return false;
        }
        QString source = tokens[0];
        QString target = tokens[1];

        QDir dir;
        QFileInfo info(target);
        auto path = project_path + info.absoluteDir().absolutePath();
        dir.mkpath(path);
        QFile file(source);
        QString dst_path(path + "/" + QFileInfo(source).fileName());
        QFile dst_f(dst_path);
        if(dst_f.exists())
        {
            dst_f.remove();
        }
        qDebug() << "cp " << source << " " << dst_path;
        if(!file.copy(dst_path))
        {
            qDebug() << "failed to copy" << source << "to" << dst_path;
            return false;
        }
        if(!strip(dst_path))
        {
            qDebug() << "failed to strip file" << dst_path;
            return false;
        }
        if(!chrpath(info.absoluteDir().absolutePath(), dst_path))
        {
            qDebug() << "failed to chrpath file" << source;
            return false;
        }
        progress->setValue((++count) * 100 / files.count());
        QApplication::processEvents();
    }
    return true;
}

bool makeIcons(const QString &icon_file, const QString &target_dir)
{
    QImage img(icon_file);
    if(img.isNull() || img.height() == 0 || img.width() ==0)
    {
        qDebug() << "Invalid image for icon";
        return false;
    }
    QDir dir(target_dir);
    dir.mkdir("icons");
    QString icons_dir(target_dir + "/icons");
    if(!QFile(icons_dir).exists())
    {
        qDebug() << "Could not create icons directory" << icons_dir;
        return false;
    }
    bool ret = img.scaled(QSize(32, 32)).save(icons_dir + "/icon-32.png");
    ret &= img.scaled(QSize(48, 48)).save(icons_dir + "/icon-48.png");
    ret &= img.scaled(QSize(64, 64)).save(icons_dir + "/icon-64.png");
    ret &= img.scaled(QSize(128, 128)).save(icons_dir + "/icon-128.png");
    ret &= img.scaled(QSize(256, 256)).save(icons_dir + "/icon-256.png");
    return ret;
}

QString removeMenuItemScript(const QString &package_name,
                          const QString &version)
{
    return QString(
                "#!/bin/bash\n"
                "#xdg-desktop-menu uninstall %1-%2.desktop\n"
                "#xdg-icon-resource uninstall --size  32 %1-%2\n"
                "#xdg-icon-resource uninstall --size  48 %1-%2\n"
                "#xdg-icon-resource uninstall --size  64 %1-%2\n"
                "#xdg-icon-resource uninstall --size 128 %1-%2\n"
                "#xdg-icon-resource uninstall --size 256 %1-%2\n"
                "\n").
                    arg(package_name).
                    arg(version);

}
QString addMenuItemScript(const QString &package_name,
                          const QString &project_name,
                          const QString &version,
                          const QString &run_dir,
                          const QString &app_name,
                          const QString &menu_category)
{
    return QString(
        "#!/bin/bash\n"
        "#\n"
        "\n"
        "BIN_DIR=`pwd`\n"
        "ICON_NAME=icon-256\n"
        "TMP_DIR=`mktemp --directory`\n"
        "DESKTOP_FILE=$TMP_DIR/%1-%2.desktop\n"    //$TMP_DIR/package_name-version.desktop
        "cat << EOF > $DESKTOP_FILE\n"
        "[Desktop Entry]\n"
        "Version=%2\n"
        "Encoding=UTF-8\n"
        "Name=%3\n"                             //project_name
        "Keywords=\n"
        "GenericName=%3\n"                      //project_name
        "Type=Application\n"
        "Categories=%6\n"
        "Terminal=false\n"
        "StartupNotify=true\n"
        "Exec=\"%4/%5\" %u\n"                   //run_dir/app_name
        "MimeType=x-scheme-handler/%5\n"
        "Icon=$ICON_NAME.png\n"
        "EOF\n"
        "\n"
        "xdg-desktop-menu install $DESKTOP_FILE\n"
        "xdg-icon-resource install --size  32 \"%4/icons/icon-32.png\"  $ICON_NAME\n"
        "xdg-icon-resource install --size  48 \"%4/icons/icon-48.png\"  $ICON_NAME\n"
        "xdg-icon-resource install --size  64 \"%4/icons/icon-64.png\"  $ICON_NAME\n"
        "xdg-icon-resource install --size 128 \"%4/icons/icon-128.png\" $ICON_NAME\n"
        "xdg-icon-resource install --size 256 \"%4/icons/icon-256.png\" $ICON_NAME\n"
        "\n"
        "rm $DESKTOP_FILE\n"
        "rm -R $TMP_DIR\n"
        "\n").
            arg(package_name).
            arg(version).
            arg(project_name).
            arg(run_dir).
            arg(app_name).
            arg(menu_category);
}

bool writePostInstallScripts(const QString &debian_dir,
                             const QString &target_dir,
                             const QString &run_dir,
                             QMap<QString, QString> &scripts)
{
    qDebug() << "write post install scripts"
             << debian_dir << target_dir << run_dir << scripts.first();
//    make scripts_dir
//    copy add-menuitem.sh and remove-menuitem.sh to scripts_dir
//    create postinst in debian_dir with contents: <run_dir>/add-menuitem.sh

    QDir dir(target_dir);
    dir.mkdir("scripts");
    QString scripts_dir(target_dir + "/scripts");
    if(!QFile(scripts_dir).exists())
    {
        qDebug() << "Could not create scripts directory" << scripts_dir;
        return false;
    }

    QStringList run_paths;
    for(auto filename : scripts.keys())
    {
        QString script = scripts.value(filename);
        QString path = scripts_dir + "/" + filename;
        QFile file(path);
        if(!file.open(QIODevice::WriteOnly))
        {
            qDebug() << "Could not open script file" << path;
            return false;
        }
        if(file.write(script.toUtf8()) <= 0)
        {
            qDebug() << "Could not write script file" << path;
            return false;
        }
        file.close();
        file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner|
                            QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ExeUser|
                            QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ExeGroup|
                            QFileDevice::ReadOther|QFileDevice::ExeOther);
        run_paths << (run_dir + "/scripts/" + filename);
    }

    QString postinst("#!/bin/sh\n");
    for(auto run_path: run_paths)
    {
        postinst.append(run_path + "\n");
    }

    QFile file(debian_dir + "/postinst");
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Could not open postinst file" << (debian_dir + "/postinst");
        return false;
    }
    if(file.write(postinst.toUtf8()) <= 0)
    {
        qDebug() << "Could not write postinst file" << (debian_dir + "/postinst");
        return false;
    }
    file.close();
    file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner|
                        QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ExeUser|
                        QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ExeGroup|
                        QFileDevice::ReadOther|QFileDevice::ExeOther);
    return true;
}

void makeSymlinks(const QString &libs_dir, QProgressBar *progress)
{
    QDirIterator it(libs_dir,
                    QStringList()/* << "*.jpg"*/,
                    QDir::Files/* | QDir::Dirs*/,
                    QDirIterator::Subdirectories);
    QStringList sources;
    {
        Progress p(progress);
        int count = 0;
        while (it.hasNext())
        {
            auto file = it.next();
            sources << file;

            progress->setValue((++count) * 100 / sources.size());
            QApplication::processEvents();
        }
    }
    sources.sort();
    {
        Progress p(progress);
        int count = 0;
        for(int i = 1; i < sources.size(); ++i)
        {
            auto path0 = sources.at(i - 1);
            auto path1 = sources.at(i);

            if(path1.contains(path0))
            {
                QFileInfo info0(path0);
                QFileInfo info1(path1);

                QFile file0(path0);
                file0.remove();

                QProcess ln;
                qDebug() << "ln -s " << info1.fileName() << info0.fileName();
                ln.setWorkingDirectory(info0.absolutePath());
                ln.start("/bin/ln", QStringList() << "-s" << info1.fileName() << info0.fileName());
                ln.waitForFinished();

                progress->setValue((++count) * 100 / sources.size());
                QApplication::processEvents();
            }
        }
    }
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    s_message_handler(type, context, msg);
    s_status_bar->showMessage(msg);
    s_log_view->appendPlainText(msg);
//    QByteArray localMsg = msg.toLocal8Bit();
//    switch (type) {
//    case QtDebugMsg:
//        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        break;
//    case QtInfoMsg:
//        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        break;
//    case QtWarningMsg:
//        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        break;
//    case QtCriticalMsg:
//        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        break;
//    case QtFatalMsg:
//        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        abort();
//    }
}
}//namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_dpkg_deb(nullptr)
    , m_wait_dlg(new WaitDialog(this))
    , m_cmd_dispatcher(std::make_unique<PrivilegedCmdDispatcher>(this))
    , m_connected_to_privileged_child(false)
{
    ui->setupUi(this);
    s_status_bar = ui->statusBar;
    s_log_view = ui->logWidget->plainTextEdit();
    s_message_handler = qInstallMessageHandler(myMessageOutput);

    connect(ui->fileListWidget, &FileListWidget::progress,
            this, &MainWindow::progress);
    connect(ui->controlWidget, &ControlWidget::loadProject,
            this, &MainWindow::loadProjectFromFile);

    enum {
        SAVE_PROJECT,
        SAVE_AS_PROJECT,
        LOAD_PROJECT,
        LOAD_RECENT_PROJECT,
    };
    auto menus = ui->menuBar->findChildren<QMenu*>();
    connect(menus.first()->actions()[LOAD_PROJECT], &QAction::triggered,
            this, &MainWindow::loadProject);
    connect(menus.first()->actions()[LOAD_RECENT_PROJECT], &QAction::triggered,
            this, &MainWindow::loadLastProject);
    connect(menus.first()->actions()[SAVE_PROJECT], &QAction::triggered,
            this, static_cast<void(MainWindow::*)()>(&MainWindow::saveProject));
    connect(menus.first()->actions()[SAVE_AS_PROJECT], &QAction::triggered,
            this, &MainWindow::saveProjectAs);

    m_wait_dlg->setModal(true);
    connect(m_wait_dlg, &WaitDialog::cancel,
            this, &MainWindow::processCancelled);

    connect(m_cmd_dispatcher.get(), &PrivilegedCmdDispatcher::connected,
            [this](){
        m_cmd = "hello";
        m_cmd_dispatcher->sendCmd(QString("hello:world").toUtf8());
        m_connected_to_privileged_child = true;
    });
    connect(m_cmd_dispatcher.get(), &PrivilegedCmdDispatcher::cmdProcessed,
            [this](bool processed){
        if(m_cmd == "chown")
        {
            m_wait_dlg->show();
            startDpkgDeb(m_package_name);
        }
        qDebug() << "cmdProcessed: " << QString(m_cmd) << " " << processed;
    });
    m_cmd_dispatcher->connectToServer();
}

MainWindow::~MainWindow()
{
    qInstallMessageHandler(nullptr);
    delete ui;
}

bool MainWindow::chmod755(const QString &project_path,
        int file_count)
{
    Progress p(ui->progressBar);

    if(chmod(project_path.toUtf8().data(), 0755) < 0) { qDebug() << "could not chmod 0755" << project_path; return false; }
//    if(chown(project_path.toUtf8().data(), 2, 2) < 0) { qDebug() << "could not chown 2:2" << project_path; return false; }

    int count = 0;
    QDirIterator it(project_path,
                    QStringList()/* << "*.jpg"*/,
                    QDir::Files | QDir::Dirs,
                    QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        auto file = it.next();
        if(chmod(file.toUtf8().data(), 0755) < 0) { qDebug() << "could not chmod 0755" << file; return false; }
//        if(chown(file.toUtf8().data(), 2, 2) < 0) { qDebug() << "could not chown 2:2" << file; return false; }

        ui->progressBar->setValue((++count) * 100 / file_count);
        QApplication::processEvents();
    }
    return true;
}

void MainWindow::startDpkgDeb(const QString &package_name)
{
    //    dpkg-deb --build myprogram
    //    sudo dpkg -i myprogram.deb
    //    http://www.king-foo.com/2011/11/creating-debianubuntu-deb-packages/

    m_dpkg_deb = new QProcess;
    m_dpkg_deb->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_dpkg_deb, &QProcess::readyReadStandardOutput,
            this, &MainWindow::dpkgDebProcessOutput);
    connect(m_dpkg_deb, &QProcess::readyReadStandardError,
            this, &MainWindow::dpkgDebProcessOutput);
    connect(m_dpkg_deb, static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),
            this, &MainWindow::dpkgDebProcessFinished);

    QStringList args;
    args << "-D" << "-v" << "--build" << (appDirectory() + "/" + package_name);
    qDebug() << "Building DEB package now in" << appDirectory();
    qDebug() << "/usr/bin/dpkg-deb" << args;
//    /usr/bin/dpkg-deb -D -v --build appDirectory() + "/" + package_name;
    m_dpkg_deb->start("/usr/bin/dpkg-deb", args);
}

void MainWindow::dpkgDebProcessOutput()
{
    auto output = m_dpkg_deb->readAllStandardOutput();
    auto error = m_dpkg_deb->readAllStandardError();
    if(!error.isEmpty()) { qDebug() << error; }
    if(!output.isEmpty()) { qDebug() << output; }
}

void MainWindow::dpkgDebProcessFinished()
{
    dpkgDebProcessOutput();
    delete m_dpkg_deb;
    m_dpkg_deb = nullptr;
    m_wait_dlg->hide();
    qDebug() << "rename "
             << QString("%1/%2.deb").
                arg(appDirectory()).
                arg(m_package_name)
             << " to "
             << QString("%1/%2%3.deb").
                arg(appDirectory()).
                arg(m_package_name).
                arg(ui->controlWidget->version());
    QFile deb(QString("%1/%2.deb").
              arg(appDirectory()).
              arg(m_package_name));
    deb.rename(QString("%1/%2%3.deb").
               arg(appDirectory()).
               arg(m_package_name).
               arg(ui->controlWidget->version()));
    QDesktopServices::openUrl(QUrl::fromLocalFile(appDirectory()));
}

void MainWindow::processCancelled()
{
    if(!m_dpkg_deb)
    {
        return;
    }
    m_dpkg_deb->kill();
    m_dpkg_deb->waitForFinished();
    m_wait_dlg->hide();
}


void MainWindow::progress(int percent)
{
    ui->progressBar->setValue(percent);
}

void MainWindow::saveProject()
{
    QString package_name = ui->controlWidget->packageName();
    if(package_name.isEmpty())
    {
        qDebug() << "Need package name";
        return;
    }

    QString path = m_loaded_file.isEmpty() ?
                (appDirectory() + "/" + package_name + ".deb.mkr") :
                m_loaded_file;
    saveProject(path, projectAsJson());
}

void MainWindow::saveProjectAs()
{
    QString path = QFileDialog::getSaveFileName(this,
                                       tr("Save project as"),
                                       appDirectory(),
                                       "DebMaker Files (*.deb.mkr);;All files (*.*)");
    saveProject(path, projectAsJson());
}

void MainWindow::saveProject(const QString &path, const QString &json)
{
    if(json.isEmpty())
    {
        qDebug() << "Error parsing json in order to save project";
        return;
    }
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Error opening file" << path;
        return;
    }
    file.write(json.toUtf8());
    file.close();
    saveRecentFilePath(path);
}

QString MainWindow::projectAsJson()
{
    QString json = QString(
                "{\n"
                "\"icon\":\"%1\",\n"
                "\"files\":%2,\n"
                "\"control\":%3,\n"
                "\"startup\":\"%4\"\n"
                "}\n"
                ).
            arg(ui->iconWidget->iconFile()).
            arg(ui->fileListWidget->dataAsJsonString()).
            arg(ui->controlWidget->dataAsJsonString()).
            arg(ui->startupScriptWidget->dataAsJsonString())
            ;

    QJsonParseError error;
    QJsonDocument::fromJson(json.toUtf8(), &error).object();
    if(error.error != QJsonParseError::ParseError::NoError)
    {
        qDebug() << "Error parsing json file" << error.error << error.errorString();
        return "";
    }

    return json;
}

void MainWindow::loadProject()
{
    QString path = QFileDialog::getOpenFileName(this,
                                 tr("Load project file"),
                                 appDirectory(),
                                 "DebMaker Files (*.deb.mkr);;All files (*.*)");
//    QString path(appDirectory() + "/myprogram" + ".deb.mkr");
    loadProjectFromFile(path);

}

void MainWindow::loadProjectFromFile(const QString &path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Error opening file" << path;
        return;
    }
    QString json = file.readAll();
    file.close();

    if(json.isEmpty())
    {
        qDebug() << "File is empty";
        return;
    }

    QJsonParseError error;
    auto obj = QJsonDocument::fromJson(json.toUtf8(), &error).object();
    if(error.error != QJsonParseError::ParseError::NoError)
    {
        qDebug() << "Error parsing json file" << error.error << error.errorString();
        return;
    }
    saveRecentFilePath(path);

    if(obj.contains("icon"))
    {
        ui->iconWidget->setIconFile(obj.value("icon").toString());
    }
    if(obj.contains("files"))
    {
        auto array = obj.value("files").toArray();
        QStringList files;
        for(auto file : array)
        {
            files << file.toString();
        }
        ui->fileListWidget->setData(files);
    }
    if(obj.contains("control"))
    {
        auto map = obj.value("control").toObject().toVariantMap();
        ui->controlWidget->setData(map);
    }
    if(obj.contains("startup"))
    {
        auto script = obj.value("startup").toString();
        ui->startupScriptWidget->setData(script);
    }
}

void MainWindow::saveRecentFilePath(const QString &path)
{
    if(!path.isEmpty())
    {
        m_loaded_file = path;

        QFile ini(iniFilePath());
        if(ini.open(QIODevice::WriteOnly))
        {
            ini.write(path.toUtf8());
            ini.close();
        }
    }
}

QString MainWindow::readRecentFilePath() const
{
    QString path;
    QFile ini(iniFilePath());
    if(ini.open(QIODevice::ReadOnly))
    {
        path = ini.readAll();
        ini.close();
    }
    return path;
}

void MainWindow::loadLastProject()
{
    auto path = readRecentFilePath();
    if(path.isEmpty())
    {
        qDebug() << "No recently saved project";
        return;
    }
    loadProjectFromFile(path);
}

void MainWindow::on_pushButtonMakeDeb_clicked()
{
    QString control_string = ui->controlWidget->controlString(ui->iconWidget->iconFile());
    if(control_string.isEmpty())
    {
        qDebug() << "All fields need fillout out";
        return;
    }
    m_package_name = ui->controlWidget->packageName();
    if(m_package_name.isEmpty())
    {
        qDebug() << "Need package name";
        return;
    }
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);

    QString project_path = createProjectDirectory(m_package_name);
    QDir(project_path).removeRecursively();
    project_path = createProjectDirectory(m_package_name);
    QString control_file_path = writeControlFile(project_path, control_string);
    if(control_file_path.isEmpty())
    {
        qDebug() << "Could not write control file " << (project_path + "/DEBIAN/control");
        return;
    }
    QString target_path = makeTargetDirectories(project_path, ui->fileListWidget->installDirectory());
    if(target_path.isEmpty())
    {
        qDebug() << "Could not make target directories";
        return;
    }
    if(!copyFiles(project_path,
                  fileListWithCorrectPackageName(m_package_name, ui->fileListWidget->fileList()),
                  ui->progressBar))
    {
        qDebug() << "Copy failed";
        return;
    }

    makeSymlinks(target_path + "/" + m_package_name, ui->progressBar);
    if(!ldconfigsymlinks(target_path + "/" + m_package_name))
    {
         qDebug() << "Could not create symlinks using ldconfig " << (target_path + "/" + m_package_name);
         return;
    }

    makeIcons(ui->iconWidget->iconFile(), target_path + "/" + m_package_name);

    QString run_dir(ui->fileListWidget->installDirectory() + "/" + m_package_name);
    QMap<QString, QString> scripts;
    scripts.insert("removeMenuItemScript.sh", removeMenuItemScript(
                       ui->controlWidget->packageName(),
                       ui->controlWidget->version()
                       ));
    scripts.insert("addMenuItemScript.sh", addMenuItemScript(
                       ui->controlWidget->packageName(),
                       ui->controlWidget->projectName(),
                       ui->controlWidget->version(),
                       run_dir,
                       ui->controlWidget->projectName(),
                       ui->controlWidget->menuCategory()));
    scripts.insert("autoLaunchScript.sh", ui->startupScriptWidget->script());

    if(!writePostInstallScripts(project_path + "/DEBIAN",
                                target_path + "/" + m_package_name,
                                run_dir,
                                scripts))
    {
        qDebug() << "Could not write post install scripts";
        return;
    }
    moveLibqxcbToPlatforms(target_path, m_package_name);
    moveImageLibsToImageformats(target_path, m_package_name);
    /* launch process with sudo, then at startup, fork a child process, while parent process drops its previlige to user level
     * and the child process keeps running as root. The parent process can communicate to child process and ask it to perform
     * root level operations */

    if(!chmod755(project_path, ui->fileListWidget->fileList().count()))
    {
        return;
    }

    if(!m_connected_to_privileged_child)
    {
        qDebug() << "Failed to chown root:root";
        return;
    }
    m_cmd = "chown";
    auto cmd = QString("chown:%1").arg(project_path);
    m_cmd_dispatcher->sendCmd(cmd.toUtf8());

    qDebug() << "#######################";
}

void MainWindow::closeEvent(QCloseEvent *)
{
    qDebug() << "Send the quit command";
    if(m_connected_to_privileged_child)
    {
        m_cmd = "quit";
        m_cmd_dispatcher->sendCmd("quit:now");
    }
}
