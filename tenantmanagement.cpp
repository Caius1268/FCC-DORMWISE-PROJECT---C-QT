
#include "tenantmanagement.h"
#include <utility>

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
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QVariant>

// ==================================================
// CONSTRUCTOR
// ==================================================

TenantManagementPage::TenantManagementPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 10, 0, 0);
    layout->setSpacing(15);

    QLabel *heading = new QLabel("Tenant Management");

    heading->setStyleSheet(
        "font-size: 18px;"
        "font-weight: bold;"
        "color: #183B35;"
        );

    QLabel *description = new QLabel(
        "Register tenants, manage room assignments, and monitor tenant status."
        );

    description->setStyleSheet(
        "color: #78858C;"
        "font-size: 12px;"
        );

    // SEARCH FIELD

    searchInput = new QLineEdit(this);

    searchInput->setPlaceholderText(
        "Search by tenant name, phone number, or room..."
        );

    searchInput->setStyleSheet(
        "QLineEdit {"
        "background-color: white;"
        "border: 1px solid #DCE4DF;"
        "border-radius: 5px;"
        "padding: 9px;"
        "}"
        );

    // ACTION BUTTONS

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *addButton = new QPushButton("+ Add Tenant", this);
    QPushButton *editButton = new QPushButton("Edit Tenant", this);
    QPushButton *checkoutButton = new QPushButton("Check Out", this);
    QPushButton *refreshButton = new QPushButton("Refresh", this);

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
    checkoutButton->setStyleSheet(buttonStyle);

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
    buttonLayout->addWidget(checkoutButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(refreshButton);

    // TENANT TABLE

    tenantTable = new QTableWidget(this);

    tenantTable->setColumnCount(7);

    tenantTable->setHorizontalHeaderLabels({
        "ID",
        "Tenant Name",
        "Phone Number",
        "Email",
        "Room",
        "Status",
        "Emergency Contact"
    });

    tenantTable->setColumnHidden(0, true);

    tenantTable->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
        );

    tenantTable->setSelectionBehavior(
        QAbstractItemView::SelectRows
        );

    tenantTable->setSelectionMode(
        QAbstractItemView::SingleSelection
        );

    tenantTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );

    tenantTable->setAlternatingRowColors(true);

    tenantTable->setStyleSheet(
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
    layout->addWidget(searchInput);
    layout->addLayout(buttonLayout);
    layout->addWidget(tenantTable, 1);

    // BUTTON CONNECTIONS

    connect(addButton, &QPushButton::clicked,
            this, &TenantManagementPage::addTenant);

    connect(editButton, &QPushButton::clicked,
            this, &TenantManagementPage::editTenant);

    connect(checkoutButton, &QPushButton::clicked,
            this, &TenantManagementPage::checkOutTenant);

    connect(refreshButton, &QPushButton::clicked,
            this, &TenantManagementPage::loadTenants);

    connect(searchInput, &QLineEdit::textChanged,
            this, &TenantManagementPage::filterTenants);

    loadTenants();
}

// ==================================================
// LOAD TENANTS
// ==================================================

void TenantManagementPage::loadTenants()
{
    QString errorMessage;

    if (!Database::getTenants(tenants, errorMessage)) {
        QMessageBox::critical(
            this,
            "Database Error",
            errorMessage
            );
        return;
    }

    filterTenants();
}

// ==================================================
// FILTER TENANTS
// ==================================================

void TenantManagementPage::filterTenants()
{
    tenantTable->setRowCount(0);

    QString searchText = searchInput->text().trimmed();

    for (const Tenant &tenant : std::as_const(tenants)) {

        QString fullName =
            tenant.firstName + " " + tenant.lastName;

        QString emergencyContact =
            tenant.emergencyContactName + " (" +
            tenant.emergencyContactPhone + ")";

        bool matches =
            fullName.contains(searchText, Qt::CaseInsensitive) ||
            tenant.phone.contains(searchText, Qt::CaseInsensitive) ||
            tenant.roomNumber.contains(searchText, Qt::CaseInsensitive);

        if (!matches) {
            continue;
        }

        int row = tenantTable->rowCount();
        tenantTable->insertRow(row);

        tenantTable->setItem(
            row, 0,
            new QTableWidgetItem(QString::number(tenant.id))
            );

        tenantTable->setItem(
            row, 1,
            new QTableWidgetItem(fullName)
            );

        tenantTable->setItem(
            row, 2,
            new QTableWidgetItem(tenant.phone)
            );

        tenantTable->setItem(
            row, 3,
            new QTableWidgetItem(tenant.email)
            );

        tenantTable->setItem(
            row, 4,
            new QTableWidgetItem(
                tenant.roomNumber.isEmpty()
                    ? "Unassigned"
                    : tenant.roomNumber
                )
            );

        tenantTable->setItem(
            row, 5,
            new QTableWidgetItem(tenant.status)
            );

        tenantTable->setItem(
            row, 6,
            new QTableWidgetItem(emergencyContact)
            );
    }
}

// ==================================================
// ADD TENANT
// ==================================================

void TenantManagementPage::addTenant()
{
    QDialog dialog(this);

    dialog.setWindowTitle("Register New Tenant");
    dialog.setMinimumWidth(420);

    QFormLayout form(&dialog);

    // INPUT FIELDS

    QLineEdit *firstNameInput = new QLineEdit(&dialog);
    QLineEdit *lastNameInput = new QLineEdit(&dialog);
    QLineEdit *phoneInput = new QLineEdit(&dialog);
    QLineEdit *emailInput = new QLineEdit(&dialog);

    QLineEdit *emergencyNameInput = new QLineEdit(&dialog);
    QLineEdit *emergencyPhoneInput = new QLineEdit(&dialog);

    firstNameInput->setPlaceholderText("First name");
    lastNameInput->setPlaceholderText("Last name");
    phoneInput->setPlaceholderText("Contact number");
    emailInput->setPlaceholderText("Email address");

    emergencyNameInput->setPlaceholderText(
        "Emergency contact's name"
        );

    emergencyPhoneInput->setPlaceholderText(
        "Emergency contact's phone"
        );

    // ROOM SELECTION

    QComboBox *roomInput = new QComboBox(&dialog);

    roomInput->addItem("No room assigned", 0);

    QVector<Room> availableRooms;
    QString errorMessage;

    if (!Database::getRooms(availableRooms, errorMessage)) {
        QMessageBox::critical(
            this,
            "Database Error",
            errorMessage
            );
        return;
    }

    for (const Room &room : std::as_const(availableRooms)) {

        roomInput->addItem(
            room.roomNumber + " (" +
                room.roomType + ", capacity " +
                QString::number(room.capacity) + ")",
            room.id
            );
    }

    // ADD FIELDS TO FORM

    form.addRow("First Name:", firstNameInput);
    form.addRow("Last Name:", lastNameInput);
    form.addRow("Phone Number:", phoneInput);
    form.addRow("Email:", emailInput);

    form.addRow("Emergency Contact:", emergencyNameInput);
    form.addRow("Emergency Phone:", emergencyPhoneInput);

    form.addRow("Assign Room:", roomInput);

    // DIALOG BUTTONS

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Save |
            QDialogButtonBox::Cancel,
        &dialog
        );

    form.addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted,
            &dialog, &QDialog::accept);

    connect(buttons, &QDialogButtonBox::rejected,
            &dialog, &QDialog::reject);

    // SHOW DIALOG

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    // COLLECT TENANT INFORMATION

    Tenant tenant;

    tenant.firstName = firstNameInput->text().trimmed();
    tenant.lastName = lastNameInput->text().trimmed();
    tenant.phone = phoneInput->text().trimmed();
    tenant.email = emailInput->text().trimmed();

    tenant.emergencyContactName =
        emergencyNameInput->text().trimmed();

    tenant.emergencyContactPhone =
        emergencyPhoneInput->text().trimmed();

    tenant.roomId = roomInput->currentData().toInt();

    // SAVE TENANT

    if (!Database::addTenant(tenant, errorMessage)) {
        QMessageBox::warning(
            this,
            "Could Not Register Tenant",
            errorMessage
            );
        return;
    }

    QMessageBox::information(
        this,
        "Success",
        "Tenant registered successfully."
        );

    loadTenants();
}

// ==================================================
// EDIT TENANT
// ==================================================

void TenantManagementPage::editTenant()
{
    // CHECK SELECTED ROW

    int row = tenantTable->currentRow();

    if (row < 0) {
        QMessageBox::warning(
            this,
            "No Selection",
            "Please select a tenant to edit."
            );
        return;
    }

    // RETRIEVE TENANT ID

    int tenantId =
        tenantTable->item(row, 0)->text().toInt();

    // FIND TENANT

    Tenant selectedTenant;
    bool found = false;

    for (const Tenant &tenant : std::as_const(tenants)) {

        if (tenant.id == tenantId) {
            selectedTenant = tenant;
            found = true;
            break;
        }
    }

    if (!found) {
        QMessageBox::warning(
            this,
            "Error",
            "Could not find the selected tenant."
            );
        return;
    }

    // PREVENT EDITING CHECKED-OUT TENANTS

    if (selectedTenant.status != "Active") {
        QMessageBox::information(
            this,
            "Cannot Edit",
            "Only active tenants can be edited."
            );
        return;
    }

    // CREATE EDITING DIALOG

    QDialog dialog(this);

    dialog.setWindowTitle("Edit Tenant Information");
    dialog.setMinimumWidth(420);

    QFormLayout form(&dialog);

    // PRE-FILLED INPUT FIELDS

    QLineEdit *firstNameInput =
        new QLineEdit(selectedTenant.firstName, &dialog);

    QLineEdit *lastNameInput =
        new QLineEdit(selectedTenant.lastName, &dialog);

    QLineEdit *phoneInput =
        new QLineEdit(selectedTenant.phone, &dialog);

    QLineEdit *emailInput =
        new QLineEdit(selectedTenant.email, &dialog);

    QLineEdit *emergencyNameInput =
        new QLineEdit(selectedTenant.emergencyContactName, &dialog);

    QLineEdit *emergencyPhoneInput =
        new QLineEdit(selectedTenant.emergencyContactPhone, &dialog);

    // ROOM SELECTION

    QComboBox *roomInput = new QComboBox(&dialog);

    roomInput->addItem("No room assigned", 0);

    QVector<Room> availableRooms;
    QString errorMessage;

    if (!Database::getRooms(availableRooms, errorMessage)) {
        QMessageBox::critical(
            this,
            "Database Error",
            errorMessage
            );
        return;
    }

    for (const Room &room : std::as_const(availableRooms)) {

        roomInput->addItem(
            room.roomNumber + " (" +
                room.roomType + ", capacity " +
                QString::number(room.capacity) + ")",
            room.id
            );
    }

    // SELECT CURRENT ROOM

    int currentRoomIndex =
        roomInput->findData(selectedTenant.roomId);

    if (currentRoomIndex >= 0) {
        roomInput->setCurrentIndex(currentRoomIndex);
    }

    // ADD FIELDS TO FORM

    form.addRow("First Name:", firstNameInput);
    form.addRow("Last Name:", lastNameInput);
    form.addRow("Phone Number:", phoneInput);
    form.addRow("Email:", emailInput);

    form.addRow("Emergency Contact:", emergencyNameInput);
    form.addRow("Emergency Phone:", emergencyPhoneInput);

    form.addRow("Assigned Room:", roomInput);

    // DIALOG BUTTONS

    QDialogButtonBox *buttons =
        new QDialogButtonBox(
            QDialogButtonBox::Save |
                QDialogButtonBox::Cancel,
            &dialog
            );

    form.addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted,
            &dialog, &QDialog::accept);

    connect(buttons, &QDialogButtonBox::rejected,
            &dialog, &QDialog::reject);

    // SHOW DIALOG

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    // COLLECT UPDATED INFORMATION

    Tenant updatedTenant = selectedTenant;

    updatedTenant.firstName =
        firstNameInput->text().trimmed();

    updatedTenant.lastName =
        lastNameInput->text().trimmed();

    updatedTenant.phone =
        phoneInput->text().trimmed();

    updatedTenant.email =
        emailInput->text().trimmed();

    updatedTenant.emergencyContactName =
        emergencyNameInput->text().trimmed();

    updatedTenant.emergencyContactPhone =
        emergencyPhoneInput->text().trimmed();

    updatedTenant.roomId =
        roomInput->currentData().toInt();

    // SAVE UPDATED INFORMATION

    if (!Database::updateTenant(
            updatedTenant,
            errorMessage)) {

        QMessageBox::warning(
            this,
            "Update Failed",
            errorMessage
            );
        return;
    }

    QMessageBox::information(
        this,
        "Success",
        "Tenant information updated successfully."
        );

    loadTenants();
}

// ==================================================
// CHECK OUT TENANT
// ==================================================

void TenantManagementPage::checkOutTenant()
{
    int row = tenantTable->currentRow();

    if (row < 0) {
        QMessageBox::warning(
            this,
            "No Selection",
            "Please select a tenant first."
            );
        return;
    }

    int tenantId =
        tenantTable->item(row, 0)->text().toInt();

    QString tenantName =
        tenantTable->item(row, 1)->text();

    QString status =
        tenantTable->item(row, 5)->text();

    if (status != "Active") {
        QMessageBox::information(
            this,
            "Already Checked Out",
            "This tenant is not currently active."
            );
        return;
    }

    QMessageBox::StandardButton choice =
        QMessageBox::question(
            this,
            "Check Out Tenant",
            "Are you sure you want to check out " +
                tenantName + "?",
            QMessageBox::Yes | QMessageBox::No
            );

    if (choice != QMessageBox::Yes) {
        return;
    }

    QString errorMessage;

    if (!Database::checkOutTenant(tenantId, errorMessage)) {
        QMessageBox::warning(
            this,
            "Could Not Check Out Tenant",
            errorMessage
            );
        return;
    }

    QMessageBox::information(
        this,
        "Success",
        "Tenant checked out successfully."
        );

    loadTenants();
}
