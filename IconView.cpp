#include "IconView.h"
#include "ui_IconView.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>

IconView::IconView(QWidget *parent) :
    QLabel(parent),
    ui(new Ui::IconView)
{
    ui->setupUi(this);
    setAcceptDrops(true);
}

IconView::~IconView()
{
    delete ui;
}

QString IconView::iconFile() const
{
    return m_fileName;
}

void IconView::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void IconView::dropEvent(QDropEvent *e)
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
        img = img.scaledToWidth(128);
        setPixmap(QPixmap::fromImage(img));
        break;
    }
}

void IconView::dragMoveEvent(QDragMoveEvent *event)
{

}
