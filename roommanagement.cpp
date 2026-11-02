
#include "roommanagement.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QAbstractItemView>

// --------------------------------------------------
// CONSTRUCTOR
// --------------------------------------------------

RoomManagementPage::RoomManagementPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 10, 0, 0);
    layout->setSpacing(15);

    QLabel *heading = new QLabel("Room Inventory");

    heading->setStyleSheet(
        "font-size: 18px;"
        "font-weight: bold;"
        "color: #183B35;"
        );

    QLabel *description = new QLabel(
        "Manage your dormitory rooms, capacity, and monthly rates."
        );

    description->setStyleSheet(
        "color: #78858C;"
        "font-size: 12px;"
        );

    // ACTION BUTTONS

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *addButton = new QPushButton("+ Add Room");
    QPushButton *editButton = new QPushButton("Edit Room");
    QPushButton *deleteButton = new QPushButton("Delete Room");
    QPushButton *refreshButton = new QPushButton("Refresh");

    QString buttonStyle =
        "QPushButton {"
        "background-color: #287A55;"
        "color: white;"
        "border: none;"
        "border-radius: 5px;"
        "padding: 9px 15px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #206344;"
        "}";

    addButton->setStyleSheet(buttonStyle);
    editButton->setStyleSheet(buttonStyle);
    deleteButton->setStyleSheet(buttonStyle);

    refreshButton->setStyleSheet(
        "QPushButton {"
        "background-color: #E5ECE8;"
        "color: #183B35;"
        "border: none;"
        "border-radius: 5px;"
        "padding: 9px 15px;"
        "}"
        "QPushButton:hover {"
        "background-color: #D3E1D9;"
        "}"
        );

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(refreshButton);

    // ROOM TABLE

    roomTable = new QTableWidget();

    roomTable->setColumnCount(5);

    roomTable->setHorizontalHeaderLabels({
        "ID",
        "Room Number",
        "Room Type",
        "Capacity",
        "Monthly Rate"
    });

    roomTable->setColumnHidden(0, true);

    roomTable->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
        );

    roomTable->setSelectionBehavior(
        QAbstractItemView::SelectRows
        );

    roomTable->setSelectionMode(
        QAbstractItemView::SingleSelection
        );

    roomTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );

    roomTable->setAlternatingRowColors(true);

    roomTable->setStyleSheet(
        "QTableWidget {"
        "background-color: white;"
        "border: 1px solid #E4EAEC;"
        "border-radius: 6px;"
        "gridline-color: #EDF0F1;"
        "}"
        "QHeaderView::section {"
        "background-color: #E9F0EC;"
        "color: #183B35;"
        "padding: 10px;"
        "border: none;"
        "font-weight: bold;"
        "}"
        );

    layout->addWidget(heading);
    layout->addWidget(description);
    layout->addLayout(buttonLayout);
    layout->addWidget(roomTable, 1);

    connect(addButton, &QPushButton::clicked,
            this, &RoomManagementPage::addRoom);

    connect(editButton, &QPushButton::clicked,
            this, &RoomManagementPage::editRoom);

    connect(deleteButton, &QPushButton::clicked,
            this, &RoomManagementPage::deleteRoom);

    connect(refreshButton, &QPushButton::clicked,
            this, &RoomManagementPage::loadRooms);

    loadRooms();
}

// --------------------------------------------------
// LOAD ROOMS
// --------------------------------------------------

void RoomManagementPage::loadRooms()
{
    QString errorMessage;

    if (!Database::getRooms(rooms, errorMessage)) {
        QMessageBox::critical(
            this,
            "Database Error",
            errorMessage
            );
        return;
    }

    roomTable->setRowCount(0);

    for (const Room &room : std::as_const(rooms)) {
        int row = roomTable->rowCount();

        roomTable->insertRow(row);

        roomTable->setItem(
            row, 0,
            new QTableWidgetItem(QString::number(room.id))
            );

        roomTable->setItem(
            row, 1,
            new QTableWidgetItem(room.roomNumber)
            );

        roomTable->setItem(
            row, 2,
            new QTableWidgetItem(room.roomType)
            );

        roomTable->setItem(
            row, 3,
            new QTableWidgetItem(QString::number(room.capacity))
            );

        roomTable->setItem(
            row, 4,
            new QTableWidgetItem(
                QString("₱ %1").arg(
                    room.monthlyRate, 0, 'f', 2
                    )
                )
            );
    }
}

// --------------------------------------------------
// ADD ROOM
// --------------------------------------------------

void RoomManagementPage::addRoom()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Add New Room");
    dialog.setMinimumWidth(380);

    QFormLayout *form = new QFormLayout(&dialog);

    QLineEdit *numberInput = new QLineEdit();
    numberInput->setPlaceholderText("Example: A-101");

    QComboBox *typeInput = new QComboBox();

    typeInput->addItems({
        "Single",
        "Double",
        "Shared",
        "Family"
    });

    QSpinBox *capacityInput = new QSpinBox();
    capacityInput->setRange(1, 100);
    capacityInput->setValue(1);

    QDoubleSpinBox *rateInput = new QDoubleSpinBox();
    rateInput->setRange(0, 100000000);
    rateInput->setDecimals(2);
    rateInput->setPrefix("₱ ");
    rateInput->setValue(0);

    form->addRow("Room Number:", numberInput);
    form->addRow("Room Type:", typeInput);
    form->addRow("Capacity:", capacityInput);
    form->addRow("Monthly Rate:", rateInput);

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Save |
        QDialogButtonBox::Cancel
        );

    form->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted,
            &dialog, &QDialog::accept);

    connect(buttons, &QDialogButtonBox::rejected,
            &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Room room;

    room.roomNumber = numberInput->text().trimmed();
    room.roomType = typeInput->currentText();
    room.capacity = capacityInput->value();
    room.monthlyRate = rateInput->value();

    if (room.roomNumber.isEmpty()) {
        QMessageBox::warning(
            this,
            "Invalid Input",
            "Please enter a room number."
            );
        return;
    }

    QString errorMessage;

    if (!Database::addRoom(room, errorMessage)) {
        QMessageBox::warning(
            this,
            "Could Not Add Room",
            "Unable to save the room.\n\n" + errorMessage
            );
        return;
    }

    QMessageBox::information(
        this,
        "Success",
        "Room added successfully."
        );

    loadRooms();
}

// --------------------------------------------------
// EDIT ROOM
// --------------------------------------------------

void RoomManagementPage::editRoom()
{
    int row = roomTable->currentRow();

    if (row < 0) {
        QMessageBox::warning(
            this,
            "No Selection",
            "Please select a room to edit."
            );
        return;
    }

    int roomId = roomTable->item(row, 0)->text().toInt();

    Room selectedRoom;

    bool found = false;

    for (const Room &room : std::as_const(rooms)) {
        if (room.id == roomId) {
            selectedRoom = room;
            found = true;
            break;
        }
    }

    if (!found) {
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Room");
    dialog.setMinimumWidth(380);

    QFormLayout *form = new QFormLayout(&dialog);

    QLineEdit *numberInput = new QLineEdit(
        selectedRoom.roomNumber
        );

    QComboBox *typeInput = new QComboBox();

    typeInput->addItems({
        "Single",
        "Double",
        "Shared",
        "Family"
    });

    typeInput->setCurrentText(selectedRoom.roomType);

    QSpinBox *capacityInput = new QSpinBox();
    capacityInput->setRange(1, 100);
    capacityInput->setValue(selectedRoom.capacity);

    QDoubleSpinBox *rateInput = new QDoubleSpinBox();
    rateInput->setRange(0, 100000000);
    rateInput->setDecimals(2);
    rateInput->setPrefix("₱ ");
    rateInput->setValue(selectedRoom.monthlyRate);

    form->addRow("Room Number:", numberInput);
    form->addRow("Room Type:", typeInput);
    form->addRow("Capacity:", capacityInput);
    form->addRow("Monthly Rate:", rateInput);

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Save |
        QDialogButtonBox::Cancel
        );

    form->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted,
            &dialog, &QDialog::accept);

    connect(buttons, &QDialogButtonBox::rejected,
            &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    selectedRoom.roomNumber = numberInput->text().trimmed();
    selectedRoom.roomType = typeInput->currentText();
    selectedRoom.capacity = capacityInput->value();
    selectedRoom.monthlyRate = rateInput->value();

    if (selectedRoom.roomNumber.isEmpty()) {
        QMessageBox::warning(
            this,
            "Invalid Input",
            "Please enter a room number."
            );
        return;
    }

    QString errorMessage;

    if (!Database::updateRoom(selectedRoom, errorMessage)) {
        QMessageBox::warning(
            this,
            "Could Not Update Room",
            errorMessage
            );
        return;
    }

    QMessageBox::information(
        this,
        "Success",
        "Room updated successfully."
        );

    loadRooms();
}

// --------------------------------------------------
// DELETE ROOM
// --------------------------------------------------

void RoomManagementPage::deleteRoom()
{
    int row = roomTable->currentRow();

    if (row < 0) {
        QMessageBox::warning(
            this,
            "No Selection",
            "Please select a room to delete."
            );
        return;
    }

    int roomId = roomTable->item(row, 0)->text().toInt();

    QString roomNumber = roomTable->item(row, 1)->text();

    QMessageBox::StandardButton choice =
        QMessageBox::question(
            this,
            "Delete Room",
            "Are you sure you want to delete room "
                + roomNumber + "?",
            QMessageBox::Yes | QMessageBox::No
            );

    if (choice != QMessageBox::Yes) {
        return;
    }

    QString errorMessage;

    if (!Database::deleteRoom(roomId, errorMessage)) {
        QMessageBox::warning(
            this,
            "Could Not Delete Room",
            errorMessage
            );
        return;
    }

    QMessageBox::information(
        this,
        "Success",
        "Room deleted successfully."
        );

    loadRooms();
}
