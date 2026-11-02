
#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QMainWindow>

class QStackedWidget;
class QLabel;
class QPushButton;

class DashboardWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DashboardWindow(QWidget *parent = nullptr);

private:
    QStackedWidget *pages;
    QLabel *pageTitle;

    QWidget *createOverviewPage();
    QWidget *createPlaceholderPage(
        const QString &title,
        const QString &description
        );

    QPushButton *createNavButton(
        const QString &text,
        int pageIndex
        );

    void changePage(int index, const QString &title);
};

#endif
