#include <QDebug>

#include "ControlWidget.h"
#include "ui_ControlWidget.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QStringList>
#include <QStringListModel>

namespace {
const QStringList categories = QStringList()
        << "AudioVideo"
        << "Development"
        << "Education"
        << "Game"
        << "Graphics"
        << "Network"
        << "Office"
        << "Settings"
        << "System"
        << "Utility";
//  linitan: https://lintian.debian.org/tags/unknown-section.html
//        admin, comm, cli-mono, database, debug, devel, doc, editors, electronics, embedded, fonts, games,
//        gnome, gnu-r, gnustep, graphics, hamradio, haskell, httpd, interpreters, java, javascript,
//        kde, libdevel, libs, lisp, localization, kernel, mail, math, misc, net, news, ocaml, oldlibs,
//        otherosfs, perl, php, python, ruby, rust, science, shells, sound, tex, text, utils, vcs,
//        video, web, x11, xfce, zope
const QStringList sections = QStringList()
        << "database"
        << "editors"
        << "electronics"
        << "embedded"
        << "fonts"
        << "games"
        << "graphics"
        << "interpreters"
        << "mail"
        << "misc"
        << "net"
        << "news"
        << "science"
        << "sound"
        << "text"
        << "utils"
        << "video"
        << "web";
}//namespace

ControlWidget::ControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlWidget)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    ui->comboBoxSection->setModel(new QStringListModel(sections, this));
    ui->comboBoxMenuCategory->setModel(new QStringListModel(categories, this));
}

ControlWidget::~ControlWidget()
{
    delete ui;
}

QString ControlWidget::projectName() const
{
    return ui->lineEditName->text();
}

QString ControlWidget::packageName() const
{
    auto tokens = ui->lineEditPacakge->text().split(".");
    return (tokens.count() > 1) ? tokens.last() : ui->lineEditPacakge->text();
}

QString ControlWidget::version() const
{
    return ui->lineEditVersion->text();
}

QString ControlWidget::description() const
{
    return ui->plainTextEditDescription->toPlainText();
}

QString ControlWidget::menuCategory() const
{
    if(ui->comboBoxMenuCategory->currentIndex() == -1)
    {
        qDebug() << "Menu category was not selected";
        return "";
    }
    return ui->comboBoxMenuCategory->currentText();
}
QString ControlWidget::controlString(const QString &icon_file) const
{
    if(ui->lineEditPacakge->text().isEmpty() ||
            ui->lineEditName->text().isEmpty() ||
            ui->plainTextEditDescription->toPlainText().isEmpty() ||
            ui->lineEditAuthor->text().isEmpty() ||
            ui->lineEditMaintainer->text().isEmpty() ||
            ui->lineEditVersion->text().isEmpty() ||
            ui->comboBoxSection->currentIndex() == -1 ||
            ui->comboBoxMenuCategory->currentIndex() == -1 ||
            ui->lineEditArchitecture->text().isEmpty() ||
            ui->lineEditHomepage->text().isEmpty() ||
            ui->lineEditWebsite->text().isEmpty() ||
            ui->lineEditApproxSizeMb->text().isEmpty()
            )
    {
        qDebug() << "One or more mandatory fields are missing";
        return "";
    }

    QString control_string = QString(
                "Package: %1\n"
                "Name: %2\n"
                "Description: %3\n"
                "Author: %4\n"
                "Maintainer: %5\n"
                "Version: %6\n"
                "Section: %7\n"
//                "Pre-Depends:\n"
//                "Depends:\n"
//                "Recommends:\n"
//                "Suggests:\n"
//                "Enhances:\n"
//                "Replaces:\n"
//                "Conflicts:\n"
//                "Breaks:\n"
//                "Provides:\n"
                "Architecture: %8\n"
                "Homepage: %9\n"
                "Website: %10\n"
//                "Depiction: \n"
                "Icon:%11\n"
                "Installed-Size:%12\n"
                "Tag:\n\n").
            arg(ui->lineEditPacakge->text()).
            arg(ui->lineEditName->text()).
            arg(ui->plainTextEditDescription->toPlainText()).
            arg(ui->lineEditAuthor->text()).
            arg(ui->lineEditMaintainer->text()).
            arg(ui->lineEditVersion->text()).
            arg(ui->comboBoxSection->currentText()).
            arg(ui->lineEditArchitecture->text()).
            arg(ui->lineEditHomepage->text()).
            arg(ui->lineEditWebsite->text()).
            arg(icon_file).
            arg(ui->lineEditApproxSizeMb->text())
          ;

    return control_string;
}

QString ControlWidget::dataAsJsonString() const
{
    QString control_string = QString(
                "{\n"
                "\"%1\":\"%2\",\n"
                "\"%3\":\"%4\",\n"
                "\"%5\":\"%6\",\n"
                "\"%7\":\"%8\",\n"
                "\"%9\":\"%10\",\n"
                "\"%11\":\"%12\",\n"
                "\"%13\":\"%14\",\n"
                "\"%15\":\"%16\",\n"
                "\"%17\":\"%18\",\n"
                "\"%19\":\"%20\",\n"
                "\"%21\":\"%22\",\n"
                "\"%23\":\"%24\",\n"
                "\"Pre-Depends\":\"\",\n"
                "\"Depends\":\"\",\n"
                "\"Recommends\":\"\",\n"
                "\"Suggests\":\"\",\n"
                "\"Enhances\":\"\",\n"
                "\"Replaces\":\"\",\n"
                "\"Conflicts\":\"\",\n"
                "\"Breaks\":\"\",\n"
                "\"Provides\":\"\",\n"
                "\"Depiction\":\"\",\n"
                "\"Tag\":\"\"\n"
                "}\n").
            arg(ui->labelPackage->text()).arg(ui->lineEditPacakge->text()).
            arg(ui->labelName->text()).arg(ui->lineEditName->text()).
            arg(ui->labelDescription->text()).arg(ui->plainTextEditDescription->toPlainText()).
            arg(ui->labelAuthor->text()).arg(ui->lineEditAuthor->text()).
            arg(ui->labelMaintainer->text()).arg(ui->lineEditMaintainer->text()).
            arg(ui->labelVersion->text()).arg(ui->lineEditVersion->text()).
            arg(ui->labelSection->text()).arg(ui->comboBoxSection->currentText()).
            arg(ui->labelMenuCategory->text()).arg(ui->comboBoxMenuCategory->currentText()).
            arg(ui->labelArchitecture->text()).arg(ui->lineEditArchitecture->text()).
            arg(ui->labelHomepage->text()).arg(ui->lineEditHomepage->text()).
            arg(ui->labelWebsite->text()).arg(ui->lineEditWebsite->text()).
            arg(ui->labelApproxSizeKb->text()).arg(ui->lineEditApproxSizeMb->text())
            ;
    return control_string;
}

void ControlWidget::setData(const QVariantMap &map)
{
    for(auto key : map.keys())
    {
        if(key == ui->labelPackage->text()) { ui->lineEditPacakge->setText(map[key].toString()); }
        if(key == ui->labelName->text()) { ui->lineEditName->setText(map[key].toString()); }
        if(key == ui->labelDescription->text()) { ui->plainTextEditDescription->setPlainText(map[key].toString()); }
        if(key == ui->labelAuthor->text()) { ui->lineEditAuthor->setText(map[key].toString()); }
        if(key == ui->labelMaintainer->text()) { ui->lineEditMaintainer->setText(map[key].toString()); }
        if(key == ui->labelVersion->text()) { ui->lineEditVersion->setText(map[key].toString()); }
        if(key == ui->labelSection->text()) { ui->comboBoxSection->setCurrentText(map[key].toString()); }
        if(key == ui->labelMenuCategory->text()) { ui->comboBoxMenuCategory->setCurrentText(map[key].toString()); }
        if(key == ui->labelArchitecture->text()) { ui->lineEditArchitecture->setText(map[key].toString()); }
        if(key == ui->labelHomepage->text()) { ui->lineEditHomepage->setText(map[key].toString()); }
        if(key == ui->labelWebsite->text()) { ui->lineEditWebsite->setText(map[key].toString()); }
        if(key == ui->labelApproxSizeKb->text()) { ui->lineEditApproxSizeMb->setText(map[key].toString()); }
    }
}

void ControlWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void ControlWidget::dragMoveEvent(QDragMoveEvent *)
{

}

void ControlWidget::dropEvent(QDropEvent *e)
{
    for(auto &url : e->mimeData()->urls())
    {
        QString file = url.toLocalFile();
        if(QFile(file).exists() && file.endsWith(".deb.mkr"))
        {
            emit loadProject(file);
        }
        break;
    }
}
