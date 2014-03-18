#ifndef MESSAGELIST_H
#define MESSAGELIST_H

#include <QtWidgets/QListWidget>
#include <QDesignerCustomWidgetInterface>
#include <QObject>

class MessageList : public QListWidget, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
    MessageList(QObject *parent = 0);
};

#endif // MESSAGELIST_H
