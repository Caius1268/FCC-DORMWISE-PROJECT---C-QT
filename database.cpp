
#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QPasswordDigestor>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QVariant>

// ==================================================
// HELPER FUNCTIONS
// ==================================================

namespace
{
QByteArray generateSalt()
{
    QByteArray salt;

    for (int i = 0; i < 4; ++i) {
        quint32 value = QRandomGenerator::system()->generate();

        salt.append(char(value >> 24));
        salt.append(char(value >> 16));
        salt.append(char(value >> 8));
        salt.append(char(value));
    }

    return salt;
}

QByteArray hashPassword(
    const QString &password,
    const QByteArray &salt
    )
{
    return QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha256,
        password.toUtf8(),
        salt,
        210000,
        32
        );
}
}

// ==================================================
// DATABASE INITIALIZATION
// ==================================================

bool Database::initialize(QString &errorMessage)
{
    QString folder = QStandardPaths::writableLocation(
        QStandardPaths::AppLocalDataLocation
        );

    if (!QDir().mkpath(folder)) {
        errorMessage = "Could not create database folder.";
        return false;
    }

    QString databasePath = folder + "/dormwise.db";

    if (!QSqlDatabase::contains("DormWiseConnection")) {

        QSqlDatabase newDb = QSqlDatabase::addDatabase(
            "QSQLITE",
            "DormWiseConnection"
            );

        newDb.setDatabaseName(databasePath);

        if (!newDb.open()) {
            errorMessage = newDb.lastError().text();
            return false;
        }
    }

    QSqlDatabase db = QSqlDatabase::database(
        "DormWiseConnection"
        );

    if (!db.isOpen() && !db.open()) {
        errorMessage = db.lastError().text();
        return false;
    }

    // ENABLE FOREIGN KEY SUPPORT

    QSqlQuery foreignKeyQuery(db);

    if (!foreignKeyQuery.exec("PRAGMA foreign_keys = ON")) {
        errorMessage = foreignKeyQuery.lastError().text();
        return false;
    }

    QSqlQuery query(db);

    // CREATE USERS TABLE

    QString userSql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            password_hash TEXT NOT NULL,
            salt TEXT NOT NULL,
            role TEXT NOT NULL DEFAULT 'Admin',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(userSql)) {
        errorMessage = query.lastError().text();
        return false;
    }

    // CREATE ROOMS TABLE

    QString roomSql = R"(
        CREATE TABLE IF NOT EXISTS rooms (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            room_number TEXT NOT NULL UNIQUE,
            room_type TEXT NOT NULL,
            capacity INTEGER NOT NULL CHECK(capacity > 0),
            monthly_rate REAL NOT NULL CHECK(monthly_rate >= 0),
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(roomSql)) {
        errorMessage = query.lastError().text();
        return false;
    }

    // CREATE TENANTS TABLE

    QString tenantSql = R"(
        CREATE TABLE IF NOT EXISTS tenants (
            id INTEGER PRIMARY KEY AUTOINCREMENT,

            first_name TEXT NOT NULL,
            last_name TEXT NOT NULL,

            phone TEXT NOT NULL,
            email TEXT,

            emergency_contact_name TEXT,
            emergency_contact_phone TEXT,

            room_id INTEGER,

            status TEXT NOT NULL DEFAULT 'Active'
                CHECK(status IN ('Active', 'Checked Out')),

            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,

            FOREIGN KEY (room_id)
                REFERENCES rooms(id)
                ON DELETE RESTRICT
        )
    )";

    if (!query.exec(tenantSql)) {
        errorMessage = query.lastError().text();
        return false;
    }

    // CREATE TENANT ROOM INDEX

    if (!query.exec(
            "CREATE INDEX IF NOT EXISTS idx_tenants_room_id "
            "ON tenants(room_id)"
            )) {
        errorMessage = query.lastError().text();
        return false;
    }

    return true;
}

// ==================================================
// ROOM MANAGEMENT
// ==================================================

// ADD ROOM

bool Database::addRoom(
    const Room &room,
    QString &errorMessage
    )
{
    if (room.roomNumber.trimmed().isEmpty()) {
        errorMessage = "Room number cannot be empty.";
        return false;
    }

    if (room.capacity <= 0 || room.monthlyRate < 0) {
        errorMessage = "Invalid room capacity or monthly rate.";
        return false;
    }

    QSqlQuery query(
        QSqlDatabase::database("DormWiseConnection")
        );

    query.prepare(R"(
        INSERT INTO rooms
        (room_number, room_type, capacity, monthly_rate)
        VALUES (:number, :type, :capacity, :rate)
    )");

    query.bindValue(":number", room.roomNumber.trimmed());
    query.bindValue(":type", room.roomType);
    query.bindValue(":capacity", room.capacity);
    query.bindValue(":rate", room.monthlyRate);

    if (!query.exec()) {
        errorMessage = query.lastError().text();
        return false;
    }

    return true;
}

// UPDATE ROOM

bool Database::updateRoom(
    const Room &room,
    QString &errorMessage
    )
{
    if (room.roomNumber.trimmed().isEmpty()) {
        errorMessage = "Room number cannot be empty.";
        return false;
    }

    if (room.capacity <= 0 || room.monthlyRate < 0) {
        errorMessage = "Invalid room capacity or monthly rate.";
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(
        "DormWiseConnection"
        );

    if (!db.transaction()) {
        errorMessage = db.lastError().text();
        return false;
    }

    // Prevent reducing capacity below current occupancy.

    QSqlQuery countQuery(db);

    countQuery.prepare(R"(
        SELECT COUNT(*)
        FROM tenants
        WHERE room_id = :roomId
        AND status = 'Active'
    )");

    countQuery.bindValue(":roomId", room.id);

    if (!countQuery.exec() || !countQuery.next()) {
        errorMessage = countQuery.lastError().text();
        db.rollback();
        return false;
    }

    int occupants = countQuery.value(0).toInt();

    if (room.capacity < occupants) {
        errorMessage =
            "Cannot reduce capacity below the current number of active tenants.";
        db.rollback();
        return false;
    }

    QSqlQuery query(db);

    query.prepare(R"(
        UPDATE rooms
        SET room_number = :number,
            room_type = :type,
            capacity = :capacity,
            monthly_rate = :rate
        WHERE id = :id
    )");

    query.bindValue(":number", room.roomNumber.trimmed());
    query.bindValue(":type", room.roomType);
    query.bindValue(":capacity", room.capacity);
    query.bindValue(":rate", room.monthlyRate);
    query.bindValue(":id", room.id);

    if (!query.exec()) {
        errorMessage = query.lastError().text();
        db.rollback();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        errorMessage = "Room was not found or no changes were made.";
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        errorMessage = db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}

// DELETE ROOM

bool Database::deleteRoom(
    int roomId,
    QString &errorMessage
    )
{
    QSqlQuery query(
        QSqlDatabase::database("DormWiseConnection")
        );

    query.prepare(
        "DELETE FROM rooms WHERE id = :id"
        );

    query.bindValue(":id", roomId);

    if (!query.exec()) {
        errorMessage = query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        errorMessage = "Room was not found.";
        return false;
    }

    return true;
}

// GET ALL ROOMS

bool Database::getRooms(
    QVector<Room> &rooms,
    QString &errorMessage
    )
{
    rooms.clear();

    QSqlQuery query(
        QSqlDatabase::database("DormWiseConnection")
        );

    if (!query.exec(R"(
        SELECT id, room_number, room_type, capacity, monthly_rate
        FROM rooms
        ORDER BY room_number ASC
    )")) {
        errorMessage = query.lastError().text();
        return false;
    }

    while (query.next()) {
        Room room;

        room.id = query.value("id").toInt();
        room.roomNumber = query.value("room_number").toString();
        room.roomType = query.value("room_type").toString();
        room.capacity = query.value("capacity").toInt();
        room.monthlyRate = query.value("monthly_rate").toDouble();

        rooms.append(room);
    }

    return true;
}

// ==================================================
// TENANT MANAGEMENT
// ==================================================

// ADD TENANT

bool Database::addTenant(
    const Tenant &tenant,
    QString &errorMessage
    )
{
    QSqlDatabase db = QSqlDatabase::database(
        "DormWiseConnection"
        );

    if (tenant.firstName.trimmed().isEmpty() ||
        tenant.lastName.trimmed().isEmpty() ||
        tenant.phone.trimmed().isEmpty()) {

        errorMessage =
            "First name, last name, and phone number are required.";

        return false;
    }

    if (!db.transaction()) {
        errorMessage = db.lastError().text();
        return false;
    }

    // CHECK ROOM CAPACITY

    if (tenant.roomId > 0) {

        QSqlQuery roomQuery(db);

        roomQuery.prepare(R"(
            SELECT capacity
            FROM rooms
            WHERE id = :roomId
        )");

        roomQuery.bindValue(":roomId", tenant.roomId);

        if (!roomQuery.exec() || !roomQuery.next()) {
            errorMessage = "The selected room does not exist.";
            db.rollback();
            return false;
        }

        int capacity = roomQuery.value("capacity").toInt();

        QSqlQuery countQuery(db);

        countQuery.prepare(R"(
            SELECT COUNT(*)
            FROM tenants
            WHERE room_id = :roomId
            AND status = 'Active'
        )");

        countQuery.bindValue(":roomId", tenant.roomId);

        if (!countQuery.exec() || !countQuery.next()) {
            errorMessage = countQuery.lastError().text();
            db.rollback();
            return false;
        }

        int currentOccupants = countQuery.value(0).toInt();

        if (currentOccupants >= capacity) {
            errorMessage =
                "This room has reached its maximum capacity.";

            db.rollback();
            return false;
        }
    }

    // INSERT TENANT

    QSqlQuery query(db);

    query.prepare(R"(
        INSERT INTO tenants (
            first_name,
            last_name,
            phone,
            email,
            emergency_contact_name,
            emergency_contact_phone,
            room_id,
            status
        )
        VALUES (
            :firstName,
            :lastName,
            :phone,
            :email,
            :emergencyName,
            :emergencyPhone,
            :roomId,
            'Active'
        )
    )");

    query.bindValue(":firstName", tenant.firstName.trimmed());
    query.bindValue(":lastName", tenant.lastName.trimmed());
    query.bindValue(":phone", tenant.phone.trimmed());
    query.bindValue(":email", tenant.email.trimmed());

    query.bindValue(
        ":emergencyName",
        tenant.emergencyContactName.trimmed()
        );

    query.bindValue(
        ":emergencyPhone",
        tenant.emergencyContactPhone.trimmed()
        );

    if (tenant.roomId > 0) {
        query.bindValue(":roomId", tenant.roomId);
    } else {
        query.bindValue(":roomId", QVariant());
    }

    if (!query.exec()) {
        errorMessage = query.lastError().text();
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        errorMessage = db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}

// GET ALL TENANTS

bool Database::getTenants(
    QVector<Tenant> &tenants,
    QString &errorMessage
    )
{
    tenants.clear();

    QSqlQuery query(
        QSqlDatabase::database("DormWiseConnection")
        );

    if (!query.exec(R"(
        SELECT
            tenants.id,
            tenants.first_name,
            tenants.last_name,
            tenants.phone,
            tenants.email,
            tenants.emergency_contact_name,
            tenants.emergency_contact_phone,
            tenants.room_id,
            rooms.room_number,
            tenants.status
        FROM tenants
        LEFT JOIN rooms
            ON tenants.room_id = rooms.id
        ORDER BY tenants.last_name ASC,
                 tenants.first_name ASC
    )")) {
        errorMessage = query.lastError().text();
        return false;
    }

    while (query.next()) {
        Tenant tenant;

        tenant.id = query.value("id").toInt();
        tenant.firstName = query.value("first_name").toString();
        tenant.lastName = query.value("last_name").toString();
        tenant.phone = query.value("phone").toString();
        tenant.email = query.value("email").toString();

        tenant.emergencyContactName =
            query.value("emergency_contact_name").toString();

        tenant.emergencyContactPhone =
            query.value("emergency_contact_phone").toString();

        tenant.roomId = query.value("room_id").toInt();
        tenant.roomNumber = query.value("room_number").toString();
        tenant.status = query.value("status").toString();

        tenants.append(tenant);
    }

    return true;
}


bool Database::updateTenant(
    const Tenant &tenant,
    QString &errorMessage)
{
    if (tenant.id <= 0) {
        errorMessage = "Invalid tenant ID.";
        return false;
    }

    if (tenant.firstName.trimmed().isEmpty() ||
        tenant.lastName.trimmed().isEmpty() ||
        tenant.phone.trimmed().isEmpty()) {

        errorMessage = "First name, last name, and phone are required.";
        return false;
    }

    QSqlDatabase db =
        QSqlDatabase::database("DormWiseConnection");

    if (!db.transaction()) {
        errorMessage = db.lastError().text();
        return false;
    }

    // Check whether the tenant exists and is still active.
    QSqlQuery tenantCheck(db);

    tenantCheck.prepare(
        "SELECT status FROM tenants WHERE id = ?"
        );

    tenantCheck.addBindValue(tenant.id);

    if (!tenantCheck.exec()) {
        errorMessage = tenantCheck.lastError().text();
        db.rollback();
        return false;
    }

    if (!tenantCheck.next()) {
        errorMessage = "Tenant not found.";
        db.rollback();
        return false;
    }

    if (tenantCheck.value(0).toString() != "Active") {
        errorMessage = "Only active tenants can be edited.";
        db.rollback();
        return false;
    }

    // Check the selected room, if one was assigned.
    if (tenant.roomId > 0) {

        QSqlQuery roomCheck(db);

        roomCheck.prepare(
            "SELECT capacity FROM rooms WHERE id = ?"
            );

        roomCheck.addBindValue(tenant.roomId);

        if (!roomCheck.exec()) {
            errorMessage = roomCheck.lastError().text();
            db.rollback();
            return false;
        }

        if (!roomCheck.next()) {
            errorMessage = "Selected room does not exist.";
            db.rollback();
            return false;
        }

        int capacity = roomCheck.value(0).toInt();

        // Count other active tenants in the selected room.
        // Exclude the tenant currently being edited.
        QSqlQuery occupancyCheck(db);

        occupancyCheck.prepare(
            "SELECT COUNT(*) FROM tenants "
            "WHERE room_id = ? "
            "AND status = 'Active' "
            "AND id != ?"
            );

        occupancyCheck.addBindValue(tenant.roomId);
        occupancyCheck.addBindValue(tenant.id);

        if (!occupancyCheck.exec()) {
            errorMessage = occupancyCheck.lastError().text();
            db.rollback();
            return false;
        }

        occupancyCheck.next();

        int occupiedSpaces =
            occupancyCheck.value(0).toInt();

        if (occupiedSpaces >= capacity) {
            errorMessage = "The selected room is already full.";
            db.rollback();
            return false;
        }
    }

    // Update tenant information.
    QSqlQuery query(db);

    query.prepare(
        "UPDATE tenants SET "
        "first_name = ?, "
        "last_name = ?, "
        "phone = ?, "
        "email = ?, "
        "emergency_contact_name = ?, "
        "emergency_contact_phone = ?, "
        "room_id = ? "
        "WHERE id = ? AND status = 'Active'"
        );

    query.addBindValue(tenant.firstName.trimmed());
    query.addBindValue(tenant.lastName.trimmed());
    query.addBindValue(tenant.phone.trimmed());
    query.addBindValue(tenant.email.trimmed());
    query.addBindValue(tenant.emergencyContactName.trimmed());
    query.addBindValue(tenant.emergencyContactPhone.trimmed());

    if (tenant.roomId > 0) {
        query.addBindValue(tenant.roomId);
    } else {
        query.addBindValue(QVariant());
    }

    query.addBindValue(tenant.id);

    if (!query.exec()) {
        errorMessage = query.lastError().text();
        db.rollback();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        errorMessage = "No tenant information was updated.";
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        errorMessage = db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}

// CHECK OUT TENANT

bool Database::checkOutTenant(
    int tenantId,
    QString &errorMessage
    )
{
    QSqlQuery query(
        QSqlDatabase::database("DormWiseConnection")
        );

    query.prepare(R"(
        UPDATE tenants
        SET status = 'Checked Out',
            room_id = NULL
        WHERE id = :id
        AND status = 'Active'
    )");

    query.bindValue(":id", tenantId);

    if (!query.exec()) {
        errorMessage = query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        errorMessage =
            "Tenant was not found or has already checked out.";

        return false;
    }

    return true;
}

// ==================================================
// ADMINISTRATOR MANAGEMENT
// ==================================================

// CHECK IF ADMIN EXISTS

bool Database::adminExists()
{
    QSqlQuery query(
        QSqlDatabase::database("DormWiseConnection")
        );

    query.prepare(
        "SELECT COUNT(*) FROM users WHERE role = 'Admin'"
        );

    if (!query.exec() || !query.next()) {
        return false;
    }

    return query.value(0).toInt() > 0;
}

// CREATE ADMINISTRATOR

bool Database::createAdmin(
    const QString &username,
    const QString &password,
    QString &errorMessage
    )
{
    if (adminExists()) {
        errorMessage =
            "An administrator account already exists.";

        return false;
    }

    QString cleanUsername = username.trimmed();

    if (cleanUsername.isEmpty() || password.isEmpty()) {
        errorMessage = "Please complete all fields.";
        return false;
    }

    if (cleanUsername.length() < 4) {
        errorMessage =
            "Username must have at least 4 characters.";

        return false;
    }

    if (password.length() < 8) {
        errorMessage =
            "Password must have at least 8 characters.";

        return false;
    }

    QByteArray salt = generateSalt();
    QByteArray passwordHash = hashPassword(password, salt);

    QSqlQuery query(
        QSqlDatabase::database("DormWiseConnection")
        );

    query.prepare(R"(
        INSERT INTO users
        (username, password_hash, salt, role)
        VALUES (:username, :password_hash, :salt, 'Admin')
    )");

    query.bindValue(":username", cleanUsername);
    query.bindValue(":password_hash", passwordHash.toHex());
    query.bindValue(":salt", salt.toHex());

    if (!query.exec()) {
        errorMessage = query.lastError().text();
        return false;
    }

    return true;
}

// VERIFY ADMINISTRATOR LOGIN

bool Database::verifyLogin(
    const QString &username,
    const QString &password
    )
{
    QSqlQuery query(
        QSqlDatabase::database("DormWiseConnection")
        );

    query.prepare(R"(
        SELECT password_hash, salt
        FROM users
        WHERE username = :username
        AND role = 'Admin'
    )");

    query.bindValue(":username", username.trimmed());

    if (!query.exec() || !query.next()) {
        return false;
    }

    QByteArray storedHash = QByteArray::fromHex(
        query.value("password_hash").toByteArray()
        );

    QByteArray salt = QByteArray::fromHex(
        query.value("salt").toByteArray()
        );

    QByteArray enteredHash = hashPassword(password, salt);

    return storedHash == enteredHash;
}
