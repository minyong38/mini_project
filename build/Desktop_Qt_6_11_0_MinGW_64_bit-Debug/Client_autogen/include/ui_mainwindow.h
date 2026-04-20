/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "CustomCalendarWidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *mainLayout;
    QFrame *navBar;
    QHBoxLayout *navLayout;
    QLabel *titleLabel;
    QSpacerItem *navSpacer;
    QPushButton *chatBtn;
    QLabel *userPrefixLabel;
    QComboBox *userCombo;
    QFrame *onlineBar;
    QHBoxLayout *onlineLayout;
    QLabel *onlineLabel;
    QWidget *calendarWrapper;
    QVBoxLayout *wrapperLayout;
    QFrame *calendarCard;
    QVBoxLayout *cardLayout;
    CustomCalendarWidget *calendarWidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(980, 720);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        mainLayout = new QVBoxLayout(centralwidget);
        mainLayout->setSpacing(0);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(0, 0, 0, 0);
        navBar = new QFrame(centralwidget);
        navBar->setObjectName("navBar");
        navBar->setMinimumSize(QSize(0, 60));
        navBar->setMaximumSize(QSize(16777215, 60));
        navLayout = new QHBoxLayout(navBar);
        navLayout->setSpacing(14);
        navLayout->setObjectName("navLayout");
        navLayout->setContentsMargins(24, 0, 24, 0);
        titleLabel = new QLabel(navBar);
        titleLabel->setObjectName("titleLabel");

        navLayout->addWidget(titleLabel);

        navSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        navLayout->addItem(navSpacer);

        chatBtn = new QPushButton(navBar);
        chatBtn->setObjectName("chatBtn");
        chatBtn->setMinimumSize(QSize(86, 36));

        navLayout->addWidget(chatBtn);

        userPrefixLabel = new QLabel(navBar);
        userPrefixLabel->setObjectName("userPrefixLabel");

        navLayout->addWidget(userPrefixLabel);

        userCombo = new QComboBox(navBar);
        userCombo->setObjectName("userCombo");
        userCombo->setMinimumSize(QSize(130, 36));

        navLayout->addWidget(userCombo);


        mainLayout->addWidget(navBar);

        onlineBar = new QFrame(centralwidget);
        onlineBar->setObjectName("onlineBar");
        onlineBar->setMinimumSize(QSize(0, 30));
        onlineBar->setMaximumSize(QSize(16777215, 30));
        onlineLayout = new QHBoxLayout(onlineBar);
        onlineLayout->setObjectName("onlineLayout");
        onlineLayout->setContentsMargins(24, 0, 24, 0);
        onlineLabel = new QLabel(onlineBar);
        onlineLabel->setObjectName("onlineLabel");

        onlineLayout->addWidget(onlineLabel);


        mainLayout->addWidget(onlineBar);

        calendarWrapper = new QWidget(centralwidget);
        calendarWrapper->setObjectName("calendarWrapper");
        wrapperLayout = new QVBoxLayout(calendarWrapper);
        wrapperLayout->setObjectName("wrapperLayout");
        wrapperLayout->setContentsMargins(16, 16, 16, 16);
        calendarCard = new QFrame(calendarWrapper);
        calendarCard->setObjectName("calendarCard");
        cardLayout = new QVBoxLayout(calendarCard);
        cardLayout->setSpacing(0);
        cardLayout->setObjectName("cardLayout");
        cardLayout->setContentsMargins(0, 0, 0, 0);
        calendarWidget = new CustomCalendarWidget(calendarCard);
        calendarWidget->setObjectName("calendarWidget");
        calendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
        calendarWidget->setGridVisible(true);

        cardLayout->addWidget(calendarWidget);


        wrapperLayout->addWidget(calendarCard);


        mainLayout->addWidget(calendarWrapper);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 980, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "\354\272\230\353\246\260\353\215\224", nullptr));
        MainWindow->setStyleSheet(QCoreApplication::translate("MainWindow", "QMainWindow { background-color: #F2F2F7; }", nullptr));
        navBar->setStyleSheet(QCoreApplication::translate("MainWindow", "QFrame#navBar { background-color: #FFFFFF; border-bottom: 1px solid #E5E5EA; }", nullptr));
        titleLabel->setText(QCoreApplication::translate("MainWindow", "\360\237\223\205 \352\263\265\354\234\240 \354\272\230\353\246\260\353\215\224", nullptr));
        titleLabel->setStyleSheet(QCoreApplication::translate("MainWindow", "QLabel { font-size: 20px; font-weight: bold; color: #1C1C1E; background: transparent; border: none; }", nullptr));
        chatBtn->setText(QCoreApplication::translate("MainWindow", "\360\237\222\254 \354\261\204\355\214\205", nullptr));
        chatBtn->setStyleSheet(QCoreApplication::translate("MainWindow", "QPushButton { background: #FFCD00; color: #3C1E1E; border: none; border-radius: 18px; padding: 0 16px; font-size: 13px; font-weight: bold; } QPushButton:hover { background: #FFBA00; } QPushButton:pressed { background: #F0A800; }", nullptr));
        userPrefixLabel->setText(QCoreApplication::translate("MainWindow", "\353\263\264\352\270\260:", nullptr));
        userPrefixLabel->setStyleSheet(QCoreApplication::translate("MainWindow", "QLabel { font-size: 13px; color: #8E8E93; background: transparent; border: none; }", nullptr));
        userCombo->setStyleSheet(QCoreApplication::translate("MainWindow", "QComboBox { padding: 4px 12px; border: 1.5px solid #E5E5EA; border-radius: 18px; font-size: 13px; background: #F2F2F7; color: #1C1C1E; } QComboBox:hover { border-color: #007AFF; background: #FFFFFF; } QComboBox::drop-down { border: none; width: 20px; } QComboBox QAbstractItemView { border: 1px solid #E5E5EA; border-radius: 10px; background: #FFFFFF; selection-background-color: #E5F0FF; selection-color: #007AFF; font-size: 13px; }", nullptr));
        onlineBar->setStyleSheet(QCoreApplication::translate("MainWindow", "QFrame#onlineBar { background-color: #F0FFF4; border-bottom: 1px solid #C3E6CB; }", nullptr));
        onlineLabel->setText(QCoreApplication::translate("MainWindow", "\354\240\221\354\206\215 \354\244\221: \354\227\260\352\262\260 \354\244\221...", nullptr));
        onlineLabel->setStyleSheet(QCoreApplication::translate("MainWindow", "QLabel { font-size: 12px; color: #2D6A4F; background: transparent; border: none; }", nullptr));
        calendarWrapper->setStyleSheet(QCoreApplication::translate("MainWindow", "QWidget#calendarWrapper { background: #F2F2F7; }", nullptr));
        calendarCard->setStyleSheet(QCoreApplication::translate("MainWindow", "QFrame#calendarCard { background: #FFFFFF; border-radius: 16px; border: none; }", nullptr));
        calendarWidget->setStyleSheet(QCoreApplication::translate("MainWindow", "\n"
"CustomCalendarWidget {\n"
"    background-color: #FFFFFF;\n"
"    border: none;\n"
"    border-radius: 16px;\n"
"}\n"
"QCalendarWidget QToolButton {\n"
"    color: #1C1C1E;\n"
"    background: transparent;\n"
"    border: none;\n"
"    font-size: 15px;\n"
"    font-weight: 700;\n"
"    padding: 8px 14px;\n"
"    border-radius: 8px;\n"
"}\n"
"QCalendarWidget QToolButton:hover {\n"
"    background-color: #F2F2F7;\n"
"}\n"
"QCalendarWidget QToolButton#qt_calendar_prevmonth,\n"
"QCalendarWidget QToolButton#qt_calendar_nextmonth {\n"
"    color: #007AFF;\n"
"    font-size: 18px;\n"
"    font-weight: bold;\n"
"}\n"
"QCalendarWidget QWidget#qt_calendar_navigationbar {\n"
"    background-color: #FFFFFF;\n"
"    padding: 10px 8px 6px 8px;\n"
"    border-bottom: 1px solid #F2F2F7;\n"
"    border-top-left-radius: 16px;\n"
"    border-top-right-radius: 16px;\n"
"}\n"
"QCalendarWidget QAbstractItemView {\n"
"    background-color: #FFFFFF;\n"
"    selection-background-color: transparent;\n"
"    selection-color: #007AF"
                        "F;\n"
"    font-size: 13px;\n"
"    color: #1C1C1E;\n"
"    outline: none;\n"
"    gridline-color: #F2F2F7;\n"
"}\n"
"QCalendarWidget QAbstractItemView:disabled {\n"
"    color: #D1D1D6;\n"
"}\n"
"QCalendarWidget QWidget {\n"
"    alternate-background-color: #FFFFFF;\n"
"}\n"
"             ", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
