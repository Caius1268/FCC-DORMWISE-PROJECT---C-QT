
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QStackedWidget;
class QLineEdit;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget *pages;

    QWidget *createLoginPage();
    QWidget *createRegistrationPage();

    void showLoginPage();
    void showRegistrationPage();

    void handleLogin();
    void handleRegistration();

    QLineEdit *loginUsername;
    QLineEdit *loginPassword;

    QLineEdit *registerUsername;
    QLineEdit *registerPassword;
    QLineEdit *confirmPassword;

    QLabel *loginMessage;
    QLabel *registerMessage;
};

#endif
