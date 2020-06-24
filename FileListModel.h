#ifndef FILELISTMODEL_H
#define FILELISTMODEL_H

#include <QStringListModel>

class FileListModel : public QStringListModel
{
    Q_OBJECT
public:
    explicit FileListModel(QObject *parent = nullptr);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

signals:
    void listDataChanged(int row, const QString &value);

public slots:
};

#endif // FILELISTMODEL_H
