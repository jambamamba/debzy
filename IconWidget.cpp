#include "IconWidget.h"
#include "ui_IconWidget.h"

#include <QDebug>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QStandardPaths>

namespace {


QString imagesDirectory()
{
    return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
}


}
IconWidget::IconWidget(QWidget *parent) :
    QLabel(parent),
    ui(new Ui::IconWidget)
{
    ui->setupUi(this);
    setAcceptDrops(true);
}

IconWidget::~IconWidget()
{
    delete ui;
}

QString IconWidget::iconFile() const
{
    return m_fileName;
}

void IconWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void IconWidget::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls())
    {
        m_fileName = url.toLocalFile();
        qDebug() << "dropped" << m_fileName;
        QImage img(m_fileName);
        if(img.isNull())
        {
            continue;
        }
        setPixmap(QPixmap::fromImage(img.scaledToWidth(128)));
        break;
    }
}

void IconWidget::dragMoveEvent(QDragMoveEvent *event)
{

}

void IconWidget::on_pushButtonClearIcon_clicked()
{
    setPixmap(QPixmap::fromImage(QImage()));
}


void IconWidget::setIconFile(const QString &icon_file)
{

    QImage img(icon_file);
    if(img.isNull())
    {
        qDebug() << "Bad icon file";
        return;
    }
    m_fileName = icon_file;
    setPixmap(QPixmap::fromImage(img.scaledToWidth(128)));
}

void IconWidget::on_pushButtonSelectIconFile_clicked()
{
    QString icon_file = QFileDialog::getOpenFileName(this,
                                 tr("Load image file"),
                                 imagesDirectory(),
                                 "PNG Files (*.png);;JPG Files (*.jpg);;All files (*.*)");
    setIconFile(icon_file);
}
