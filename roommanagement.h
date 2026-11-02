
#ifndef ROOMMANAGEMENT_H
#define ROOMMANAGEMENT_H

#include <QWidget>
#include <QVector>
#include "database.h"

class QTableWidget;

class RoomManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit RoomManagementPage(QWidget *parent = nullptr);

private:
    QTableWidget *roomTable;
    QVector<Room> rooms;

    void loadRooms();
    void addRoom();
    void editRoom();
    void deleteRoom();
};

#endif
