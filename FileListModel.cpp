#include "FileListModel.h"

#include <QDebug>

FileListModel::FileListModel(QObject *parent)
    : QStringListModel(parent)
{

}

bool FileListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role = Qt::EditRole)
    {
        qDebug() << "list data changed" << index << value << role;
        emit listDataChanged(index.row(), value.toString());
    }
    return QStringListModel::setData(index, value, role);
}
