
#include "mainwindow.h"
#include "database.h"
#include "dashboard.h"

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QStackedWidget>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("DormWise");
    resize(1000, 650);

    pages = new QStackedWidget(this);
    setCentralWidget(pages);

    pages->addWidget(createLoginPage());
    pages->addWidget(createRegistrationPage());

    // First-time setup or normal login
    if (Database::adminExists()) {
        showLoginPage();
    } else {
        showRegistrationPage();
    }
}

MainWindow::~MainWindow()
{
}

// --------------------------------------------------
// LOGIN PAGE
// --------------------------------------------------

QWidget *MainWindow::createLoginPage()
{
    QWidget *page = new QWidget();

    QHBoxLayout *mainLayout = new QHBoxLayout(page);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Left panel
    QFrame *leftPanel = new QFrame();
    leftPanel->setStyleSheet("background-color: #183B35;");

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(50, 50, 50, 50);

    QLabel *brand = new QLabel("DormWise");
    brand->setStyleSheet(
        "color: white;"
        "font-size: 34px;"
        "font-weight: bold;"
        );

    QLabel *description = new QLabel(
        "Smarter Dormitory Management\n"
        "Organized spaces. Better living."
        );

    description->setStyleSheet(
        "color: #D0E3DA;"
        "font-size: 16px;"
        );

    leftLayout->addWidget(brand);
    leftLayout->addSpacing(20);
    leftLayout->addWidget(description);
    leftLayout->addStretch();

    QLabel *footer = new QLabel(
        "DORMITORY MANAGEMENT SYSTEM"
        );

    footer->setStyleSheet(
        "color: #AFC9BC;"
        "font-size: 11px;"
        );

    leftLayout->addWidget(footer);

    // Right panel
    QWidget *rightPanel = new QWidget();
    rightPanel->setStyleSheet(
        "background-color: white;"
        "font-family: 'Segoe UI';"
        );

    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(70, 70, 70, 70);
    rightLayout->setSpacing(15);

    QLabel *welcome = new QLabel("Welcome Back!");
    welcome->setStyleSheet(
        "font-size: 28px;"
        "font-weight: bold;"
        "color: #183B35;"
        );

    QLabel *subtitle = new QLabel(
        "Sign in to access your DormWise account."
        );

    subtitle->setStyleSheet(
        "color: #78858C;"
        "font-size: 13px;"
        );

    QLabel *usernameLabel = new QLabel("Username");

    loginUsername = new QLineEdit();
    loginUsername->setPlaceholderText("Enter your username");
    loginUsername->setMinimumHeight(42);

    QLabel *passwordLabel = new QLabel("Password");

    loginPassword = new QLineEdit();
    loginPassword->setPlaceholderText("Enter your password");
    loginPassword->setEchoMode(QLineEdit::Password);
    loginPassword->setMinimumHeight(42);

    QString fieldStyle =
        "QLineEdit {"
        "border: 1px solid #DCE3E5;"
        "border-radius: 6px;"
        "padding: 10px;"
        "background-color: #FAFCFC;"
        "color: #263238;"
        "}"
        "QLineEdit:focus {"
        "border: 2px solid #39866B;"
        "}";

    loginUsername->setStyleSheet(fieldStyle);
    loginPassword->setStyleSheet(fieldStyle);

    QPushButton *loginButton = new QPushButton("SIGN IN");
    loginButton->setMinimumHeight(46);

    loginButton->setStyleSheet(
        "QPushButton {"
        "background-color: #287A55;"
        "color: white;"
        "border: none;"
        "border-radius: 6px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #206344;"
        "}"
        );

    QPushButton *registerButton = new QPushButton(
        "Register another account"
        );

    registerButton->setStyleSheet(
        "QPushButton {"
        "color: #39866B;"
        "border: none;"
        "background: transparent;"
        "}"
        "QPushButton:hover {"
        "text-decoration: underline;"
        "}"
        );

    loginMessage = new QLabel();
    loginMessage->setStyleSheet("color: #C0392B;");

    rightLayout->addStretch();
    rightLayout->addWidget(welcome);
    rightLayout->addWidget(subtitle);
    rightLayout->addSpacing(25);
    rightLayout->addWidget(usernameLabel);
    rightLayout->addWidget(loginUsername);
    rightLayout->addWidget(passwordLabel);
    rightLayout->addWidget(loginPassword);
    rightLayout->addSpacing(15);
    rightLayout->addWidget(loginButton);
    rightLayout->addWidget(loginMessage);
    rightLayout->addWidget(registerButton);
    rightLayout->addStretch();

    mainLayout->addWidget(leftPanel, 1);
    mainLayout->addWidget(rightPanel, 1);

    connect(loginButton, &QPushButton::clicked,
            this, &MainWindow::handleLogin);

    // Registration is only available after initial setup.
    // We will implement authorized account creation later.
    registerButton->hide();

    connect(loginPassword, &QLineEdit::returnPressed,
            this, &MainWindow::handleLogin);

    return page;
}

// --------------------------------------------------
// REGISTRATION PAGE
// --------------------------------------------------

QWidget *MainWindow::createRegistrationPage()
{
    QWidget *page = new QWidget();

    QHBoxLayout *mainLayout = new QHBoxLayout(page);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *leftPanel = new QFrame();
    leftPanel->setStyleSheet("background-color: #183B35;");

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(50, 50, 50, 50);

    QLabel *brand = new QLabel("DormWise");
    brand->setStyleSheet(
        "color: white;"
        "font-size: 34px;"
        "font-weight: bold;"
        );

    QLabel *description = new QLabel(
        "Your dormitory.\n"
        "Managed smarter."
        );

    description->setStyleSheet(
        "color: #D0E3DA;"
        "font-size: 16px;"
        );

    leftLayout->addWidget(brand);
    leftLayout->addSpacing(20);
    leftLayout->addWidget(description);
    leftLayout->addStretch();

    QLabel *footer = new QLabel(
        "FIRST-TIME ADMINISTRATOR SETUP"
        );

    footer->setStyleSheet(
        "color: #AFC9BC;"
        "font-size: 11px;"
        );

    leftLayout->addWidget(footer);

    QWidget *rightPanel = new QWidget();
    rightPanel->setStyleSheet(
        "background-color: white;"
        "font-family: 'Segoe UI';"
        );

    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(70, 45, 70, 45);
    rightLayout->setSpacing(12);

    QLabel *heading = new QLabel("Create Administrator");
    heading->setStyleSheet(
        "font-size: 27px;"
        "font-weight: bold;"
        "color: #183B35;"
        );

    QLabel *subtitle = new QLabel(
        "Set up your first DormWise administrator account."
        );

    subtitle->setStyleSheet(
        "color: #78858C;"
        "font-size: 13px;"
        );

    QString fieldStyle =
        "QLineEdit {"
        "border: 1px solid #DCE3E5;"
        "border-radius: 6px;"
        "padding: 10px;"
        "background-color: #FAFCFC;"
        "color: #263238;"
        "}"
        "QLineEdit:focus {"
        "border: 2px solid #39866B;"
        "}";

    QLabel *usernameLabel = new QLabel("Username");

    registerUsername = new QLineEdit();
    registerUsername->setPlaceholderText("Create a username");
    registerUsername->setMinimumHeight(42);
    registerUsername->setStyleSheet(fieldStyle);

    QLabel *passwordLabel = new QLabel("Password");

    registerPassword = new QLineEdit();
    registerPassword->setPlaceholderText("Create a password");
    registerPassword->setEchoMode(QLineEdit::Password);
    registerPassword->setMinimumHeight(42);
    registerPassword->setStyleSheet(fieldStyle);

    QLabel *confirmLabel = new QLabel("Confirm Password");

    confirmPassword = new QLineEdit();
    confirmPassword->setPlaceholderText("Re-enter your password");
    confirmPassword->setEchoMode(QLineEdit::Password);
    confirmPassword->setMinimumHeight(42);
    confirmPassword->setStyleSheet(fieldStyle);

    QLabel *passwordHint = new QLabel(
        "Password must contain at least 8 characters."
        );

    passwordHint->setStyleSheet(
        "color: #78858C;"
        "font-size: 11px;"
        );

    QPushButton *createButton = new QPushButton(
        "CREATE ADMINISTRATOR"
        );

    createButton->setMinimumHeight(46);

    createButton->setStyleSheet(
        "QPushButton {"
        "background-color: #287A55;"
        "color: white;"
        "border: none;"
        "border-radius: 6px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #206344;"
        "}"
        );

    registerMessage = new QLabel();
    registerMessage->setWordWrap(true);

    registerMessage->setStyleSheet(
        "color: #C0392B;"
        "font-size: 12px;"
        );

    rightLayout->addStretch();
    rightLayout->addWidget(heading);
    rightLayout->addWidget(subtitle);
    rightLayout->addSpacing(15);
    rightLayout->addWidget(usernameLabel);
    rightLayout->addWidget(registerUsername);
    rightLayout->addWidget(passwordLabel);
    rightLayout->addWidget(registerPassword);
    rightLayout->addWidget(confirmLabel);
    rightLayout->addWidget(confirmPassword);
    rightLayout->addWidget(passwordHint);
    rightLayout->addSpacing(10);
    rightLayout->addWidget(createButton);
    rightLayout->addWidget(registerMessage);
    rightLayout->addStretch();

    mainLayout->addWidget(leftPanel, 1);
    mainLayout->addWidget(rightPanel, 1);

    connect(createButton, &QPushButton::clicked,
            this, &MainWindow::handleRegistration);

    connect(confirmPassword, &QLineEdit::returnPressed,
            this, &MainWindow::handleRegistration);

    return page;
}

// --------------------------------------------------
// PAGE NAVIGATION
// --------------------------------------------------

void MainWindow::showLoginPage()
{
    pages->setCurrentIndex(0);
    setWindowTitle("DormWise - Login");
}

void MainWindow::showRegistrationPage()
{
    pages->setCurrentIndex(1);
    setWindowTitle("DormWise - Administrator Setup");
}

// --------------------------------------------------
// LOGIN
// --------------------------------------------------


void MainWindow::handleLogin()
{
    QString username = loginUsername->text();
    QString password = loginPassword->text();

    if (username.trimmed().isEmpty() || password.isEmpty()) {
        loginMessage->setText(
            "Please enter your username and password."
            );
        return;
    }

    if (Database::verifyLogin(username, password)) {

        DashboardWindow *dashboard = new DashboardWindow();

        dashboard->show();

        this->close();

    } else {
        loginMessage->setText(
            "Incorrect username or password."
            );
    }
}

// --------------------------------------------------
// ADMINISTRATOR REGISTRATION
// --------------------------------------------------

void MainWindow::handleRegistration()
{
    QString username = registerUsername->text();
    QString password = registerPassword->text();
    QString confirmation = confirmPassword->text();

    if (username.trimmed().isEmpty() ||
        password.isEmpty() ||
        confirmation.isEmpty()) {

        registerMessage->setText("Please complete all fields.");
        return;
    }

    if (password != confirmation) {
        registerMessage->setText("Passwords do not match.");
        return;
    }

    QString errorMessage;

    if (!Database::createAdmin(username, password, errorMessage)) {
        registerMessage->setText(errorMessage);
        return;
    }

    QMessageBox::information(
        this,
        "Account Created",
        "Your administrator account has been created successfully!"
        );

    loginUsername->setText(username.trimmed());
    loginPassword->clear();
    registerUsername->clear();
    registerPassword->clear();
    confirmPassword->clear();
    registerMessage->clear();

    showLoginPage();
}
