
#include "mainwindow.h"
#include "database.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("DormWise");
    a.setOrganizationName("DormWise");

    QString errorMessage;

    if (!Database::initialize(errorMessage)) {
        QMessageBox::critical(
            nullptr,
            "Database Error",
            errorMessage
            );

        return 1;
    }

    MainWindow w;
    w.show();

    return a.exec();
}
