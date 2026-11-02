
#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QVector>

// Room information
struct Room
{
    int id = 0;
    QString roomNumber;
    QString roomType;
    int capacity = 0;
    double monthlyRate = 0.0;
};

// Tenant information
struct Tenant
{
    int id = 0;

    QString firstName;
    QString lastName;
    QString phone;
    QString email;

    QString emergencyContactName;
    QString emergencyContactPhone;

    int roomId = 0;
    QString roomNumber;

    QString status;
};

class Database
{
public:

    // Database initialization
    static bool initialize(QString &errorMessage);

    // Admin account
    static bool adminExists();

    static bool createAdmin(
        const QString &username,
        const QString &password,
        QString &errorMessage
        );

    static bool verifyLogin(
        const QString &username,
        const QString &password
        );

    // Room management
    static bool addRoom(
        const Room &room,
        QString &errorMessage
        );

    static bool updateRoom(
        const Room &room,
        QString &errorMessage
        );

    static bool deleteRoom(
        int roomId,
        QString &errorMessage
        );

    static bool getRooms(
        QVector<Room> &rooms,
        QString &errorMessage
        );

    // Tenant management
    static bool addTenant(const Tenant &tenant, QString &errorMessage);

    static bool getTenants(QVector<Tenant> &tenants, QString &errorMessage);

    static bool updateTenant(const Tenant &tenant, QString &errorMessage);

    static bool checkOutTenant(int tenantId, QString &errorMessage);
};

#endif // DATABASE_H
