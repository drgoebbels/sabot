#ifndef MESSAGELIST_H
#define MESSAGELIST_H

#include <QtWidgets/QListWidget>
#include <QDesignerCustomWidgetInterface>

class MessageList : public QDesignerCustomWidgetInterface, public QListWidget
{
public:
    MessageList();
};

#endif // MESSAGELIST_H
