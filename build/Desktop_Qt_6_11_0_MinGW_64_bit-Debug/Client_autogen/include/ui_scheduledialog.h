/********************************************************************************
** Form generated from reading UI file 'scheduledialog.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SCHEDULEDIALOG_H
#define UI_SCHEDULEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ScheduleDialog
{
public:
    QVBoxLayout *outerLayout;
    QFrame *card;
    QVBoxLayout *cardLayout;
    QFrame *headerFrame;
    QHBoxLayout *headerLayout;
    QLabel *dateLabel;
    QSpacerItem *headerSpacer;
    QPushButton *closeBtn;
    QLabel *emptyLabel;
    QListWidget *listWidget;
    QFrame *footerFrame;
    QHBoxLayout *footerLayout;
    QPushButton *addBtn;
    QSpacerItem *footerSpacer;
    QPushButton *editBtn;
    QPushButton *deleteBtn;

    void setupUi(QDialog *ScheduleDialog)
    {
        if (ScheduleDialog->objectName().isEmpty())
            ScheduleDialog->setObjectName("ScheduleDialog");
        ScheduleDialog->resize(400, 480);
        outerLayout = new QVBoxLayout(ScheduleDialog);
        outerLayout->setSpacing(0);
        outerLayout->setObjectName("outerLayout");
        outerLayout->setContentsMargins(0, 0, 0, 0);
        card = new QFrame(ScheduleDialog);
        card->setObjectName("card");
        cardLayout = new QVBoxLayout(card);
        cardLayout->setSpacing(0);
        cardLayout->setObjectName("cardLayout");
        cardLayout->setContentsMargins(0, 0, 0, 0);
        headerFrame = new QFrame(card);
        headerFrame->setObjectName("headerFrame");
        headerFrame->setMinimumSize(QSize(0, 64));
        headerFrame->setMaximumSize(QSize(16777215, 64));
        headerLayout = new QHBoxLayout(headerFrame);
        headerLayout->setObjectName("headerLayout");
        headerLayout->setContentsMargins(20, 0, 12, 0);
        dateLabel = new QLabel(headerFrame);
        dateLabel->setObjectName("dateLabel");

        headerLayout->addWidget(dateLabel);

        headerSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        headerLayout->addItem(headerSpacer);

        closeBtn = new QPushButton(headerFrame);
        closeBtn->setObjectName("closeBtn");
        closeBtn->setMinimumSize(QSize(32, 32));
        closeBtn->setMaximumSize(QSize(32, 32));

        headerLayout->addWidget(closeBtn);


        cardLayout->addWidget(headerFrame);

        emptyLabel = new QLabel(card);
        emptyLabel->setObjectName("emptyLabel");
        emptyLabel->setAlignment(Qt::AlignCenter);

        cardLayout->addWidget(emptyLabel);

        listWidget = new QListWidget(card);
        listWidget->setObjectName("listWidget");

        cardLayout->addWidget(listWidget);

        footerFrame = new QFrame(card);
        footerFrame->setObjectName("footerFrame");
        footerFrame->setMinimumSize(QSize(0, 64));
        footerFrame->setMaximumSize(QSize(16777215, 64));
        footerLayout = new QHBoxLayout(footerFrame);
        footerLayout->setSpacing(8);
        footerLayout->setObjectName("footerLayout");
        footerLayout->setContentsMargins(16, 0, 16, 0);
        addBtn = new QPushButton(footerFrame);
        addBtn->setObjectName("addBtn");
        addBtn->setMinimumSize(QSize(80, 38));

        footerLayout->addWidget(addBtn);

        footerSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        footerLayout->addItem(footerSpacer);

        editBtn = new QPushButton(footerFrame);
        editBtn->setObjectName("editBtn");
        editBtn->setMinimumSize(QSize(72, 38));

        footerLayout->addWidget(editBtn);

        deleteBtn = new QPushButton(footerFrame);
        deleteBtn->setObjectName("deleteBtn");
        deleteBtn->setMinimumSize(QSize(72, 38));

        footerLayout->addWidget(deleteBtn);


        cardLayout->addWidget(footerFrame);


        outerLayout->addWidget(card);


        retranslateUi(ScheduleDialog);

        QMetaObject::connectSlotsByName(ScheduleDialog);
    } // setupUi

    void retranslateUi(QDialog *ScheduleDialog)
    {
        ScheduleDialog->setWindowTitle(QCoreApplication::translate("ScheduleDialog", "\354\235\274\354\240\225", nullptr));
        ScheduleDialog->setStyleSheet(QCoreApplication::translate("ScheduleDialog", "QDialog { background: transparent; }", nullptr));
        card->setStyleSheet(QCoreApplication::translate("ScheduleDialog", "QFrame#card { background-color: #FFFFFF; border-radius: 16px; border: none; }", nullptr));
        headerFrame->setStyleSheet(QCoreApplication::translate("ScheduleDialog", "QFrame#headerFrame { background: #FFFFFF; border-top-left-radius: 16px; border-top-right-radius: 16px; border-bottom: 1px solid #F2F2F7; }", nullptr));
        dateLabel->setText(QCoreApplication::translate("ScheduleDialog", "\353\202\240\354\247\234", nullptr));
        dateLabel->setStyleSheet(QCoreApplication::translate("ScheduleDialog", "QLabel { font-size: 20px; font-weight: bold; color: #000000; background: transparent; border: none; }", nullptr));
        closeBtn->setText(QCoreApplication::translate("ScheduleDialog", "\342\234\225", nullptr));
        closeBtn->setStyleSheet(QCoreApplication::translate("ScheduleDialog", "QPushButton { background-color: #F2F2F7; color: #8E8E93; border: none; border-radius: 16px; font-size: 14px; font-weight: bold; } QPushButton:hover { background-color: #E5E5EA; }", nullptr));
        emptyLabel->setText(QCoreApplication::translate("ScheduleDialog", "\354\235\274\354\240\225\354\235\264 \354\227\206\354\212\265\353\213\210\353\213\244", nullptr));
        emptyLabel->setStyleSheet(QCoreApplication::translate("ScheduleDialog", "QLabel { font-size: 15px; color: #C7C7CC; background: transparent; border: none; padding: 40px 0; }", nullptr));
        listWidget->setStyleSheet(QCoreApplication::translate("ScheduleDialog", "\n"
"QListWidget {\n"
"    border: none;\n"
"    background: transparent;\n"
"    outline: none;\n"
"    font-size: 15px;\n"
"    color: #000000;\n"
"}\n"
"QListWidget::item {\n"
"    padding: 10px 20px;\n"
"    border-bottom: 1px solid #F2F2F7;\n"
"    min-height: 44px;\n"
"}\n"
"QListWidget::item:selected {\n"
"    background-color: #E5F0FF;\n"
"    color: #007AFF;\n"
"}\n"
"QListWidget::item:hover:!selected {\n"
"    background-color: #F9F9F9;\n"
"}\n"
"         ", nullptr));
        footerFrame->setStyleSheet(QCoreApplication::translate("ScheduleDialog", "QFrame#footerFrame { background: #FFFFFF; border-top: 1px solid #F2F2F7; border-bottom-left-radius: 16px; border-bottom-right-radius: 16px; }", nullptr));
        addBtn->setText(QCoreApplication::translate("ScheduleDialog", "+ \354\266\224\352\260\200", nullptr));
        addBtn->setStyleSheet(QCoreApplication::translate("ScheduleDialog", "QPushButton { background-color: #007AFF; color: white; border: none; border-radius: 19px; font-size: 14px; font-weight: 600; padding: 0 18px; } QPushButton:hover { background-color: #0066D6; } QPushButton:pressed { background-color: #0055B3; }", nullptr));
        editBtn->setText(QCoreApplication::translate("ScheduleDialog", "\354\210\230\354\240\225", nullptr));
        editBtn->setStyleSheet(QCoreApplication::translate("ScheduleDialog", "QPushButton { background-color: transparent; color: #007AFF; border: 1.5px solid #007AFF; border-radius: 19px; font-size: 14px; font-weight: 500; padding: 0 16px; } QPushButton:hover { background-color: #E5F0FF; } QPushButton:disabled { color: #C7C7CC; border-color: #C7C7CC; }", nullptr));
        deleteBtn->setText(QCoreApplication::translate("ScheduleDialog", "\354\202\255\354\240\234", nullptr));
        deleteBtn->setStyleSheet(QCoreApplication::translate("ScheduleDialog", "QPushButton { background-color: transparent; color: #FF3B30; border: 1.5px solid #FF3B30; border-radius: 19px; font-size: 14px; font-weight: 500; padding: 0 16px; } QPushButton:hover { background-color: #FFF0EF; } QPushButton:disabled { color: #C7C7CC; border-color: #C7C7CC; }", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ScheduleDialog: public Ui_ScheduleDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SCHEDULEDIALOG_H
