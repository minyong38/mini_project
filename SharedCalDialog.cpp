#include "SharedCalDialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

SharedCalDialog::SharedCalDialog(const QStringList& friends, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("공유 캘린더 만들기");
    setFixedWidth(320);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto* root = new QVBoxLayout(this);
    root->setSpacing(12);
    root->setContentsMargins(24, 24, 24, 20);

    auto* title = new QLabel("🗓 새 공유 캘린더", this);
    title->setStyleSheet("font-size:16px; font-weight:700; color:#1C1C1E;");
    root->addWidget(title);

    auto* nameLabel = new QLabel("캘린더 이름", this);
    nameLabel->setStyleSheet("font-size:12px; color:#6E6E73; font-weight:600;");
    root->addWidget(nameLabel);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("예) 가족, 팀 프로젝트");
    m_nameEdit->setStyleSheet(
        "QLineEdit { padding:8px 12px; border:1.5px solid #E5E5EA; border-radius:10px;"
        " font-size:14px; color:#1C1C1E; }"
        "QLineEdit:focus { border-color:#007AFF; }"
        );
    root->addWidget(m_nameEdit);

    if (!friends.isEmpty()) {
        auto* memLabel = new QLabel("초대할 친구 선택", this);
        memLabel->setStyleSheet("font-size:12px; color:#6E6E73; font-weight:600;");
        root->addWidget(memLabel);

        m_friendList = new QListWidget(this);
        m_friendList->setFixedHeight(qMin(friends.size() * 36, 144));
        m_friendList->setStyleSheet(
            "QListWidget { border:1.5px solid #E5E5EA; border-radius:10px; outline:none; }"
            "QListWidget::item { padding:6px 12px; font-size:13px; }"
            "QListWidget::item:selected { background:#E5F0FF; color:#007AFF; border-radius:6px; }"
            );
        for (const QString& f : friends) {
            auto* item = new QListWidgetItem("👤 " + f, m_friendList);
            item->setData(Qt::UserRole, f);
            item->setCheckState(Qt::Unchecked);
            m_friendList->addItem(item);
        }
        root->addWidget(m_friendList);
    }

    root->addSpacing(4);

    auto* btnRow = new QHBoxLayout();
    auto* cancelBtn = new QPushButton("취소", this);
    cancelBtn->setFixedHeight(40);
    cancelBtn->setStyleSheet(
        "QPushButton { background:#F2F2F7; border:none; border-radius:10px;"
        " font-size:14px; color:#1C1C1E; }"
        "QPushButton:hover { background:#E5E5EA; }"
        );
    auto* createBtn = new QPushButton("만들기", this);
    createBtn->setFixedHeight(40);
    createBtn->setDefault(true);
    createBtn->setStyleSheet(
        "QPushButton { background:#007AFF; border:none; border-radius:10px;"
        " font-size:14px; color:#FFFFFF; font-weight:600; }"
        "QPushButton:hover { background:#0071E3; }"
        );
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(createBtn);
    root->addLayout(btnRow);

    connect(cancelBtn,  &QPushButton::clicked,      this, &QDialog::reject);
    connect(createBtn,  &QPushButton::clicked,      this, &QDialog::accept);
    connect(m_nameEdit, &QLineEdit::returnPressed,  this, &QDialog::accept);
}

QString SharedCalDialog::calendarName() const
{
    return m_nameEdit->text().trimmed();
}

QStringList SharedCalDialog::selectedFriends() const
{
    QStringList result;
    if (!m_friendList) return result;
    for (int i = 0; i < m_friendList->count(); ++i) {
        auto* item = m_friendList->item(i);
        if (item->checkState() == Qt::Checked)
            result << item->data(Qt::UserRole).toString();
    }
    return result;
}
