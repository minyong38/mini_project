#include "SearchDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QDate>

SearchDialog::SearchDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("일정 검색");
    setMinimumWidth(400);
    setMinimumHeight(480);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto* root = new QVBoxLayout(this);
    root->setSpacing(12);
    root->setContentsMargins(24, 24, 24, 24);

    auto* title = new QLabel("🔍 일정 검색");
    title->setStyleSheet("font-size:17px; font-weight:700; color:#1C1C1E;");
    root->addWidget(title);

    // 검색 입력창 + 버튼
    auto* row = new QHBoxLayout;
    row->setSpacing(8);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("검색어를 입력하세요");
    m_searchEdit->setFixedHeight(40);
    m_searchEdit->setStyleSheet(
        "QLineEdit { padding:0 14px; border:1.5px solid #E5E5EA; border-radius:10px;"
        " font-size:14px; color:#1C1C1E; background:#F9F9F9; }"
        "QLineEdit:focus { border-color:#007AFF; background:#FFFFFF; }");
    m_searchBtn = new QPushButton("검색");
    m_searchBtn->setFixedSize(70, 40);
    m_searchBtn->setDefault(true);
    m_searchBtn->setStyleSheet(
        "QPushButton { background:#007AFF; border:none; border-radius:10px;"
        " font-size:14px; color:#FFFFFF; font-weight:600; }"
        "QPushButton:hover { background:#0071E3; }");
    row->addWidget(m_searchEdit);
    row->addWidget(m_searchBtn);
    root->addLayout(row);

    // 상태 메시지 (결과 없음 등)
    m_statusLabel = new QLabel;
    m_statusLabel->setStyleSheet("font-size:13px; color:#8E8E93;");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->hide();
    root->addWidget(m_statusLabel);

    // 결과 목록
    m_resultList = new QListWidget;
    m_resultList->setStyleSheet(
        "QListWidget { border:1px solid #E5E5EA; border-radius:12px;"
        " background:#FFFFFF; outline:none; }"
        "QListWidget::item { padding:12px 16px; border-bottom:1px solid #F2F2F7; }"
        "QListWidget::item:selected { background:#E5F0FF; color:#007AFF; border-radius:8px; }"
        "QListWidget::item:hover { background:#F9F9F9; }");
    m_resultList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    root->addWidget(m_resultList);

    auto* hint = new QLabel("결과를 더블클릭하면 해당 날짜로 이동합니다");
    hint->setStyleSheet("font-size:11px; color:#C7C7CC;");
    hint->setAlignment(Qt::AlignCenter);
    root->addWidget(hint);

    connect(m_searchBtn,  &QPushButton::clicked,           this, &SearchDialog::onSearch);
    connect(m_searchEdit, &QLineEdit::returnPressed,        this, &SearchDialog::onSearch);
    connect(m_resultList, &QListWidget::itemDoubleClicked, this, &SearchDialog::onItemDoubleClicked);
}

void SearchDialog::onSearch() {
    QString keyword = m_searchEdit->text().trimmed();
    if (keyword.isEmpty()) return;

    m_resultList->clear();
    m_statusLabel->setText("검색 중...");
    m_statusLabel->show();
    m_searchBtn->setEnabled(false);

    emit searchRequested(keyword);
}

void SearchDialog::setResults(const QList<QPair<QString, QString>>& results) {
    m_searchBtn->setEnabled(true);
    m_resultList->clear();

    if (results.isEmpty()) {
        m_statusLabel->setText("검색 결과가 없습니다.");
        m_statusLabel->show();
        return;
    }

    m_statusLabel->setText(QString("검색 결과 %1건").arg(results.size()));

    for (const auto& r : results) {
        QDate  date    = QDate::fromString(r.first, "yyyy-MM-dd");
        QString title  = r.second.split('\t').value(0); // 탭 구분 제목만 추출
        QString dateStr = date.toString("yyyy년 M월 d일 (ddd)");

        auto* item = new QListWidgetItem;
        item->setText(QString("📅  %1\n     %2").arg(dateStr, title));
        item->setData(Qt::UserRole, r.first); // 날짜 저장
        item->setSizeHint(QSize(0, 56));
        m_resultList->addItem(item);
    }
}

void SearchDialog::onItemDoubleClicked(QListWidgetItem* item) {
    QString dateStr = item->data(Qt::UserRole).toString();
    QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
    if (date.isValid()) {
        emit dateSelected(date);
        accept();
    }
}

void SearchDialog::setDarkMode(bool dark) {
    m_darkMode = dark;
    if (dark) {
        setStyleSheet("QDialog { background:#1C1C1E; color:#FFFFFF; }");
        m_searchEdit->setStyleSheet(
            "QLineEdit { padding:0 14px; border:1.5px solid #3A3A3C; border-radius:10px;"
            " font-size:14px; color:#FFFFFF; background:#2C2C2E; }"
            "QLineEdit:focus { border-color:#007AFF; }");
        m_resultList->setStyleSheet(
            "QListWidget { border:1px solid #3A3A3C; border-radius:12px;"
            " background:#2C2C2E; color:#FFFFFF; outline:none; }"
            "QListWidget::item { padding:12px 16px; border-bottom:1px solid #3A3A3C; }"
            "QListWidget::item:selected { background:#1A3A5C; color:#007AFF; }"
            "QListWidget::item:hover { background:#3A3A3C; }");
    } else {
        setStyleSheet("");
        m_searchEdit->setStyleSheet(
            "QLineEdit { padding:0 14px; border:1.5px solid #E5E5EA; border-radius:10px;"
            " font-size:14px; color:#1C1C1E; background:#F9F9F9; }"
            "QLineEdit:focus { border-color:#007AFF; background:#FFFFFF; }");
        m_resultList->setStyleSheet(
            "QListWidget { border:1px solid #E5E5EA; border-radius:12px;"
            " background:#FFFFFF; outline:none; }"
            "QListWidget::item { padding:12px 16px; border-bottom:1px solid #F2F2F7; }"
            "QListWidget::item:selected { background:#E5F0FF; color:#007AFF; }"
            "QListWidget::item:hover { background:#F9F9F9; }");
    }
}
