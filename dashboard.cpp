
#include "dashboard.h"
#include "roommanagement.h"
#include "tenantmanagement.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QStackedWidget>
#include <QButtonGroup>

// --------------------------------------------------
// DASHBOARD WINDOW
// --------------------------------------------------

DashboardWindow::DashboardWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("DormWise - Administrator Dashboard");
    resize(1200, 750);
    setMinimumSize(950, 600);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    central->setStyleSheet(
        "QWidget {"
        "font-family: 'Segoe UI';"
        "background-color: #F3F6F8;"
        "}"
        );

    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --------------------------------------------------
    // SIDEBAR
    // --------------------------------------------------

    QFrame *sidebar = new QFrame();
    sidebar->setFixedWidth(245);

    sidebar->setStyleSheet(
        "QFrame {"
        "background-color: #183B35;"
        "}"
        );

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(22, 30, 22, 25);
    sidebarLayout->setSpacing(10);

    QLabel *brand = new QLabel("DormWise");
    brand->setStyleSheet(
        "color: white;"
        "font-size: 27px;"
        "font-weight: bold;"
        );

    QLabel *adminLabel = new QLabel("ADMINISTRATOR PANEL");
    adminLabel->setStyleSheet(
        "color: #AFC9BC;"
        "font-size: 10px;"
        "letter-spacing: 1px;"
        );

    sidebarLayout->addWidget(brand);
    sidebarLayout->addWidget(adminLabel);
    sidebarLayout->addSpacing(35);

    QLabel *menuLabel = new QLabel("MAIN MENU");
    menuLabel->setStyleSheet(
        "color: #AFC9BC;"
        "font-size: 10px;"
        "font-weight: bold;"
        );

    sidebarLayout->addWidget(menuLabel);
    sidebarLayout->addSpacing(8);

    QButtonGroup *navGroup = new QButtonGroup(this);
    navGroup->setExclusive(true);

    QPushButton *overviewButton =
        createNavButton("Overview", 0);

    QPushButton *roomsButton =
        createNavButton("Room Management", 1);

    QPushButton *tenantsButton =
        createNavButton("Tenant Management", 2);

    QPushButton *paymentsButton =
        createNavButton("Payments", 3);

    QPushButton *reportsButton =
        createNavButton("Reports", 4);

    QPushButton *settingsButton =
        createNavButton("Settings", 5);

    QPushButton *buttons[] = {
        overviewButton,
        roomsButton,
        tenantsButton,
        paymentsButton,
        reportsButton,
        settingsButton
    };

    for (int i = 0; i < 6; i++) {
        navGroup->addButton(buttons[i], i);
        sidebarLayout->addWidget(buttons[i]);
    }

    overviewButton->setChecked(true);

    sidebarLayout->addStretch();

    QLabel *version = new QLabel("DormWise v1.0");
    version->setStyleSheet(
        "color: #AFC9BC;"
        "font-size: 11px;"
        );

    sidebarLayout->addWidget(version);

    // --------------------------------------------------
    // MAIN CONTENT AREA
    // --------------------------------------------------

    QWidget *content = new QWidget();
    content->setStyleSheet(
        "background-color: #F3F6F8;"
        );

    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(35, 30, 35, 30);
    contentLayout->setSpacing(20);

    pageTitle = new QLabel("Overview");
    pageTitle->setStyleSheet(
        "font-size: 27px;"
        "font-weight: bold;"
        "color: #183B35;"
        );

    QLabel *welcome = new QLabel(
        "Welcome to your DormWise management dashboard."
        );

    welcome->setStyleSheet(
        "font-size: 13px;"
        "color: #78858C;"
        );

    contentLayout->addWidget(pageTitle);
    contentLayout->addWidget(welcome);

    pages = new QStackedWidget();

    pages->addWidget(createOverviewPage());

    pages->addWidget(new RoomManagementPage());

    pages->addWidget(new TenantManagementPage());

    pages->addWidget(createPlaceholderPage(
        "Payments",
        "Record monthly payments and monitor outstanding balances."
        ));

    pages->addWidget(createPlaceholderPage(
        "Reports",
        "View dormitory occupancy and financial reports."
        ));

    pages->addWidget(createPlaceholderPage(
        "Settings",
        "Manage administrator accounts and system preferences."
        ));

    contentLayout->addWidget(pages, 1);

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(content, 1);

    // --------------------------------------------------
    // NAVIGATION CONNECTIONS
    // --------------------------------------------------

    connect(overviewButton, &QPushButton::clicked,
            this, [this]() {
                changePage(0, "Overview");
            });

    connect(roomsButton, &QPushButton::clicked,
            this, [this]() {
                changePage(1, "Room Management");
            });

    connect(tenantsButton, &QPushButton::clicked,
            this, [this]() {
                changePage(2, "Tenant Management");
            });

    connect(paymentsButton, &QPushButton::clicked,
            this, [this]() {
                changePage(3, "Payments");
            });

    connect(reportsButton, &QPushButton::clicked,
            this, [this]() {
                changePage(4, "Reports");
            });

    connect(settingsButton, &QPushButton::clicked,
            this, [this]() {
                changePage(5, "Settings");
            });
}

// --------------------------------------------------
// NAVIGATION BUTTON DESIGN
// --------------------------------------------------

QPushButton *DashboardWindow::createNavButton(
    const QString &text,
    int pageIndex
    )
{
    Q_UNUSED(pageIndex);

    QPushButton *button = new QPushButton(text);

    button->setCheckable(true);
    button->setMinimumHeight(43);
    button->setCursor(Qt::PointingHandCursor);

    button->setStyleSheet(
        "QPushButton {"
        "color: #D0E3DA;"
        "background-color: transparent;"
        "border: none;"
        "border-radius: 6px;"
        "text-align: left;"
        "padding-left: 14px;"
        "font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "background-color: #28564B;"
        "color: white;"
        "}"
        "QPushButton:checked {"
        "background-color: #287A55;"
        "color: white;"
        "font-weight: bold;"
        "}"
        );

    return button;
}

// --------------------------------------------------
// OVERVIEW PAGE
// --------------------------------------------------

QWidget *DashboardWindow::createOverviewPage()
{
    QWidget *page = new QWidget();

    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 10, 0, 0);
    layout->setSpacing(20);

    QLabel *sectionTitle = new QLabel(
        "Dormitory Overview"
        );

    sectionTitle->setStyleSheet(
        "font-size: 17px;"
        "font-weight: bold;"
        "color: #183B35;"
        );

    layout->addWidget(sectionTitle);

    // STATISTIC CARDS

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(15);

    QString cardStyle =
        "QFrame {"
        "background-color: white;"
        "border: 1px solid #E4EAEC;"
        "border-radius: 9px;"
        "}";

    QString valueStyle =
        "font-size: 28px;"
        "font-weight: bold;"
        "color: #183B35;"
        "border: none;";

    QString labelStyle =
        "font-size: 12px;"
        "color: #78858C;"
        "border: none;";

    QString cardTitles[] = {
        "Total Rooms",
        "Occupied Rooms",
        "Vacant Rooms",
        "Registered Tenants"
    };

    QString cardValues[] = {
        "0", "0", "0", "0"
    };

    for (int i = 0; i < 4; i++) {
        QFrame *card = new QFrame();
        card->setStyleSheet(cardStyle);
        card->setMinimumHeight(125);

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(20, 15, 20, 15);

        QLabel *value = new QLabel(cardValues[i]);
        value->setStyleSheet(valueStyle);

        QLabel *title = new QLabel(cardTitles[i]);
        title->setStyleSheet(labelStyle);

        cardLayout->addWidget(value);
        cardLayout->addWidget(title);
        cardLayout->addStretch();

        grid->addWidget(card, i / 2, i % 2);
    }

    layout->addLayout(grid);

    // RECENT ACTIVITY PANEL

    QFrame *activityPanel = new QFrame();
    activityPanel->setStyleSheet(
        "QFrame {"
        "background-color: white;"
        "border: 1px solid #E4EAEC;"
        "border-radius: 9px;"
        "}"
        );

    QVBoxLayout *activityLayout =
        new QVBoxLayout(activityPanel);

    activityLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *activityTitle = new QLabel(
        "Recent Activity"
        );

    activityTitle->setStyleSheet(
        "font-size: 16px;"
        "font-weight: bold;"
        "color: #183B35;"
        "border: none;"
        );

    QLabel *activityDescription = new QLabel(
        "No activity recorded yet.\n"
        "New tenant registrations and payment records "
        "will appear here."
        );

    activityDescription->setStyleSheet(
        "font-size: 12px;"
        "color: #78858C;"
        "border: none;"
        );

    activityLayout->addWidget(activityTitle);
    activityLayout->addSpacing(10);
    activityLayout->addWidget(activityDescription);
    activityLayout->addStretch();

    layout->addWidget(activityPanel, 1);

    QLabel *note = new QLabel(
        "Statistics will update automatically once room and "
        "tenant management are connected to the database."
        );

    note->setWordWrap(true);
    note->setStyleSheet(
        "color: #78858C;"
        "font-size: 11px;"
        );

    layout->addWidget(note);

    return page;
}

// --------------------------------------------------
// PLACEHOLDER PAGES
// --------------------------------------------------

QWidget *DashboardWindow::createPlaceholderPage(
    const QString &title,
    const QString &description
    )
{
    QWidget *page = new QWidget();

    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 25, 0, 0);

    QFrame *panel = new QFrame();

    panel->setStyleSheet(
        "QFrame {"
        "background-color: white;"
        "border: 1px solid #E4EAEC;"
        "border-radius: 9px;"
        "}"
        );

    QVBoxLayout *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(30, 30, 30, 30);

    QLabel *heading = new QLabel(title);

    heading->setStyleSheet(
        "font-size: 21px;"
        "font-weight: bold;"
        "color: #183B35;"
        "border: none;"
        );

    QLabel *details = new QLabel(description);

    details->setWordWrap(true);

    details->setStyleSheet(
        "font-size: 13px;"
        "color: #78858C;"
        "border: none;"
        );

    QLabel *status = new QLabel(
        "This module is currently under development."
        );

    status->setStyleSheet(
        "font-size: 12px;"
        "color: #287A55;"
        "border: none;"
        );

    panelLayout->addWidget(heading);
    panelLayout->addSpacing(10);
    panelLayout->addWidget(details);
    panelLayout->addSpacing(20);
    panelLayout->addWidget(status);
    panelLayout->addStretch();

    layout->addWidget(panel);
    layout->addStretch();

    return page;
}

// --------------------------------------------------
// CHANGE PAGE
// --------------------------------------------------

void DashboardWindow::changePage(
    int index,
    const QString &title
    )
{
    pages->setCurrentIndex(index);
    pageTitle->setText(title);
}
