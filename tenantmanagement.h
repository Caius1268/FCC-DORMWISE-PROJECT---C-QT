
#ifndef TENANTMANAGEMENT_H
#define TENANTMANAGEMENT_H

#include <QWidget>
#include <QVector>
#include "database.h"

class QTableWidget;
class QLineEdit;

class TenantManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit TenantManagementPage(QWidget *parent = nullptr);

private:
    QTableWidget *tenantTable;
    QLineEdit *searchInput;

    QVector<Tenant> tenants;


    void loadTenants();
    void filterTenants();
    void addTenant();
    void editTenant();
    void checkOutTenant();
};

#endif
