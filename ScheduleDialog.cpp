#include "ScheduleDialog.h"
#include "ui_scheduledialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QTime>
#include <QMessageBox>
#include <QListWidgetItem>

// ─────────────────────────────────────────────────────────────────────
//  색상 태그 유틸
// ─────────────────────────────────────────────────────────────────────

static const QStringList COLOR_NAMES = {"", "red", "orange", "yellow", "green", "purple"};
static const QStringList COLOR_CODES = {"#007AFF", "#FF3B30", "#FF9500", "#FFCC00", "#34C759", "#AF52DE"};

static QColor colorFromTag(const QString& tag) {
    int idx = COLOR_NAMES.indexOf(tag);
    if (idx > 0 && idx < COLOR_CODES.size())
        return QColor(COLOR_CODES[idx]);
    return QColor("#007AFF");
}

// ─────────────────────────────────────────────────────────────────────
//  내부 헬퍼: content 인코딩 / 디코딩 / 표시 포맷
//  content = "제목\t시작시간\t종료시간\t상세내용\t색상태그"
// ─────────────────────────────────────────────────────────────────────

static QString encodeContent(const QString& title, const QString& start,
                             const QString& end, const QString& desc,
                             const QString& color = "")
{
    return title + "\t" + start + "\t" + end + "\t" + desc + "\t" + color;
}

static QString displayText(const QString& content)
{
    QStringList p = content.split('\t');
    QString title = p.value(0);
    QString start = p.value(1);
    QString end   = p.value(2);
    QString desc  = p.value(3);

    QString text = title;
    if (!start.isEmpty())
        text += QString("  (%1 ~ %2)").arg(start, end.isEmpty() ? start : end);
    if (!desc.isEmpty())
        text += "\n    " + desc;
    return text;
}

// ─────────────────────────────────────────────────────────────────────
//  AddScheduleDialog
// ─────────────────────────────────────────────────────────────────────

AddScheduleDialog::AddScheduleDialog(QWidget* parent) : QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(380, 440);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    auto* card = new QFrame(this);
    card->setStyleSheet("QFrame { background: #FFFFFF; border-radius: 16px; }");
    outer->addWidget(card);

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    // ── 헤더 ─────────────────────────────────────────────────
    auto* header = new QFrame;
    header->setFixedHeight(56);
    header->setStyleSheet(
        "QFrame { background: #FFFFFF;"
        " border-top-left-radius: 16px; border-top-right-radius: 16px;"
        " border-bottom: 1px solid #F2F2F7; }"
        );
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(20, 0, 12, 0);

    m_headerLabel = new QLabel("일정 추가");
    m_headerLabel->setStyleSheet(
        "QLabel { font-size: 18px; font-weight: bold; color: #000;"
        " background: transparent; border: none; }"
        );
    headerLayout->addWidget(m_headerLabel);
    headerLayout->addStretch();
    cardLayout->addWidget(header);

    // ── 본문 ─────────────────────────────────────────────────
    auto* body = new QWidget;
    body->setStyleSheet("background: transparent;");
    auto* bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(20, 14, 20, 8);
    bodyLayout->setSpacing(10);

    // 제목
    auto* titleLabel = new QLabel("제목");
    titleLabel->setStyleSheet(
        "QLabel { font-size: 13px; font-weight: 600; color: #3C3C43;"
        " background: transparent; border: none; }"
        );
    m_titleEdit = new QLineEdit;
    m_titleEdit->setPlaceholderText("제목을 입력하세요 (필수)");
    m_titleEdit->setFixedHeight(40);
    m_titleEdit->setStyleSheet(
        "QLineEdit { background: #F2F2F7; border: none; border-radius: 10px;"
        " padding: 0 12px; font-size: 15px; color: #000; }"
        "QLineEdit:focus { background: #E5F0FF; }"
        );
    bodyLayout->addWidget(titleLabel);
    bodyLayout->addWidget(m_titleEdit);

    // 색상 태그
    auto* colorLabel = new QLabel("색상");
    colorLabel->setStyleSheet(
        "QLabel { font-size: 13px; font-weight: 600; color: #3C3C43;"
        " background: transparent; border: none; }"
        );
    bodyLayout->addWidget(colorLabel);

    auto* colorRow = new QWidget;
    colorRow->setStyleSheet("background: transparent;");
    auto* colorRowLayout = new QHBoxLayout(colorRow);
    colorRowLayout->setContentsMargins(0, 0, 0, 0);
    colorRowLayout->setSpacing(10);

    m_colorBtns = new QButtonGroup(this);
    for (int i = 0; i < COLOR_NAMES.size(); i++) {
        auto* btn = new QPushButton(colorRow);
        btn->setFixedSize(28, 28);
        btn->setCheckable(true);
        btn->setStyleSheet(
            QString("QPushButton {"
                    "  background: %1;"
                    "  border: 3px solid transparent;"
                    "  border-radius: 14px;"
                    "}"
                    "QPushButton:checked {"
                    "  border: 3px solid #1C1C1E;"
                    "}").arg(COLOR_CODES[i])
            );
        m_colorBtns->addButton(btn, i);
        colorRowLayout->addWidget(btn);
    }
    m_colorBtns->button(0)->setChecked(true);
    colorRowLayout->addStretch();
    bodyLayout->addWidget(colorRow);

    // 시간 체크박스
    m_timeCheck = new QCheckBox("시간 설정");
    m_timeCheck->setStyleSheet(
        "QCheckBox { font-size: 13px; font-weight: 600; color: #3C3C43;"
        " background: transparent; spacing: 6px; }"
        "QCheckBox::indicator { width: 18px; height: 18px;"
        " border-radius: 4px; border: 1.5px solid #C7C7CC; background: white; }"
        "QCheckBox::indicator:checked { background: #007AFF; border-color: #007AFF; }"
        );
    bodyLayout->addWidget(m_timeCheck);

    // 시간 입력 행
    auto* timeRow = new QWidget;
    timeRow->setStyleSheet("background: transparent;");
    auto* timeRowLayout = new QHBoxLayout(timeRow);
    timeRowLayout->setContentsMargins(0, 0, 0, 0);
    timeRowLayout->setSpacing(8);

    const QString timeStyle =
        "QTimeEdit { background: #F2F2F7; border: none; border-radius: 10px;"
        " font-size: 15px; color: #000; padding: 0 8px; }"
        "QTimeEdit:disabled { color: #C7C7CC; }";

    m_startEdit = new QTimeEdit;
    m_startEdit->setDisplayFormat("HH:mm");
    m_startEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_startEdit->setTime(QTime::currentTime());
    m_startEdit->setFixedHeight(40);
    m_startEdit->setStyleSheet(timeStyle);
    m_startEdit->setEnabled(false);

    auto* sepLabel = new QLabel("~");
    sepLabel->setStyleSheet(
        "QLabel { color: #8E8E93; font-size: 16px; background: transparent; border: none; }"
        );
    sepLabel->setAlignment(Qt::AlignCenter);

    m_endEdit = new QTimeEdit;
    m_endEdit->setDisplayFormat("HH:mm");
    m_endEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_endEdit->setTime(QTime::currentTime().addSecs(3600));
    m_endEdit->setFixedHeight(40);
    m_endEdit->setStyleSheet(timeStyle);
    m_endEdit->setEnabled(false);

    timeRowLayout->addWidget(m_startEdit);
    timeRowLayout->addWidget(sepLabel);
    timeRowLayout->addWidget(m_endEdit);
    bodyLayout->addWidget(timeRow);

    connect(m_timeCheck, &QCheckBox::toggled, this, [this](bool on) {
        m_startEdit->setEnabled(on);
        m_endEdit->setEnabled(on);
    });

    // 상세 내용
    auto* descLabel = new QLabel("상세 내용");
    descLabel->setStyleSheet(
        "QLabel { font-size: 13px; font-weight: 600; color: #3C3C43;"
        " background: transparent; border: none; }"
        );
    m_descEdit = new QPlainTextEdit;
    m_descEdit->setPlaceholderText("상세 내용을 입력하세요 (선택사항)");
    m_descEdit->setFixedHeight(72);
    m_descEdit->setStyleSheet(
        "QPlainTextEdit { background: #F2F2F7; border: none; border-radius: 10px;"
        " padding: 8px 12px; font-size: 14px; color: #000; }"
        "QPlainTextEdit:focus { background: #E5F0FF; }"
        );
    bodyLayout->addWidget(descLabel);
    bodyLayout->addWidget(m_descEdit);
    cardLayout->addWidget(body, 1);

    // ── 푸터 ─────────────────────────────────────────────────
    auto* footer = new QFrame;
    footer->setFixedHeight(64);
    footer->setStyleSheet(
        "QFrame { background: #FFFFFF;"
        " border-top: 1px solid #F2F2F7;"
        " border-bottom-left-radius: 16px; border-bottom-right-radius: 16px; }"
        );
    auto* footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(16, 0, 16, 0);
    footerLayout->setSpacing(8);

    auto* cancelBtn = new QPushButton("취소");
    cancelBtn->setFixedSize(80, 38);
    cancelBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #8E8E93;"
        " border: 1.5px solid #C7C7CC; border-radius: 19px;"
        " font-size: 14px; font-weight: 500; }"
        "QPushButton:hover { background: #F2F2F7; }"
        );

    auto* saveBtn = new QPushButton("저장");
    saveBtn->setFixedSize(80, 38);
    saveBtn->setStyleSheet(
        "QPushButton { background: #007AFF; color: white; border: none;"
        " border-radius: 19px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #0066D6; }"
        "QPushButton:pressed { background: #0055B3; }"
        );

    footerLayout->addWidget(cancelBtn);
    footerLayout->addStretch();
    footerLayout->addWidget(saveBtn);
    cardLayout->addWidget(footer);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        if (m_titleEdit->text().trimmed().isEmpty()) {
            m_titleEdit->setStyleSheet(
                "QLineEdit { background: #FFF0EF; border: 1.5px solid #FF3B30;"
                " border-radius: 10px; padding: 0 12px; font-size: 15px; color: #000; }"
                );
            m_titleEdit->setPlaceholderText("⚠ 제목을 입력해주세요");
            return;
        }
        accept();
    });
}

void AddScheduleDialog::setValues(const QString& title, const QString& startTime,
                                  const QString& endTime, const QString& description,
                                  const QString& colorTag)
{
    m_headerLabel->setText("일정 수정");
    m_titleEdit->setText(title);

    if (!startTime.isEmpty() || !endTime.isEmpty()) {
        m_timeCheck->setChecked(true);
        if (!startTime.isEmpty()) m_startEdit->setTime(QTime::fromString(startTime, "HH:mm"));
        if (!endTime.isEmpty())   m_endEdit->setTime(QTime::fromString(endTime, "HH:mm"));
    }
    m_descEdit->setPlainText(description);

    int idx = COLOR_NAMES.indexOf(colorTag);
    if (idx >= 0) m_colorBtns->button(idx)->setChecked(true);
}

QString AddScheduleDialog::title()       const { return m_titleEdit->text().trimmed(); }
QString AddScheduleDialog::startTime()   const {
    return m_timeCheck->isChecked() ? m_startEdit->time().toString("HH:mm") : QString();
}
QString AddScheduleDialog::endTime()     const {
    return m_timeCheck->isChecked() ? m_endEdit->time().toString("HH:mm") : QString();
}
QString AddScheduleDialog::description() const {
    return m_descEdit->toPlainText().replace('\t', ' ').replace('\n', ' ').trimmed();
}
QString AddScheduleDialog::colorTag()    const {
    int id = m_colorBtns->checkedId();
    return (id >= 0 && id < COLOR_NAMES.size()) ? COLOR_NAMES[id] : "";
}

// ─────────────────────────────────────────────────────────────────────
//  ScheduleDialog
// ─────────────────────────────────────────────────────────────────────

ScheduleDialog::ScheduleDialog(const QDate& date, QWidget *parent)
    : QDialog(parent), ui(new Ui::ScheduleDialog), m_date(date)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    static const QString dayNames[] = {"일", "월", "화", "수", "목", "금", "토"};
    ui->dateLabel->setText(date.toString("M월 d일 ") + "(" + dayNames[date.dayOfWeek() % 7] + ")");

    ui->listWidget->setSpacing(1);

    connect(ui->addBtn,    &QPushButton::clicked, this, &ScheduleDialog::onAdd);
    connect(ui->editBtn,   &QPushButton::clicked, this, &ScheduleDialog::onEdit);
    connect(ui->deleteBtn, &QPushButton::clicked, this, &ScheduleDialog::onDelete);
    connect(ui->listWidget, &QListWidget::itemSelectionChanged, this, &ScheduleDialog::updateButtons);
    connect(ui->closeBtn,  &QPushButton::clicked, this, &QDialog::accept);

    updateButtons();
}

ScheduleDialog::~ScheduleDialog() { delete ui; }

void ScheduleDialog::refreshSchedules(const QStringList& contents, const QList<qint64>& rowids)
{
    m_rowids   = rowids;
    m_contents = contents;
    ui->listWidget->clear();

    for (const QString& c : contents) {
        auto* item = new QListWidgetItem(displayText(c));
        QString tag = c.split('\t').value(4);
        if (!tag.isEmpty()) {
            QColor col = colorFromTag(tag);
            item->setForeground(col.darker(130));
        }
        ui->listWidget->addItem(item);
    }

    ui->emptyLabel->setVisible(contents.isEmpty());
    updateButtons();
}

void ScheduleDialog::onAdd()
{
    AddScheduleDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    QString content = encodeContent(dlg.title(), dlg.startTime(),
                                    dlg.endTime(), dlg.description(),
                                    dlg.colorTag());
    emit addRequested(content);
}

void ScheduleDialog::onEdit()
{
    int row = ui->listWidget->currentRow();
    if (row < 0 || row >= m_rowids.size()) return;

    QStringList parts = m_contents[row].split('\t');
    AddScheduleDialog dlg(this);
    dlg.setValues(parts.value(0), parts.value(1), parts.value(2),
                  parts.value(3), parts.value(4));
    if (dlg.exec() != QDialog::Accepted) return;

    QString newContent = encodeContent(dlg.title(), dlg.startTime(),
                                       dlg.endTime(), dlg.description(),
                                       dlg.colorTag());
    emit editRequested(m_rowids[row], newContent);
}

void ScheduleDialog::onDelete()
{
    int row = ui->listWidget->currentRow();
    if (row < 0 || row >= m_rowids.size()) return;

    QString title = m_contents[row].split('\t').value(0);
    auto reply = QMessageBox::question(this, "삭제 확인",
                                       "'" + title + "'\n\n이 일정을 삭제하시겠습니까?",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
        emit deleteRequested(m_rowids[row]);
}

void ScheduleDialog::setReadOnly(bool readOnly)
{
    ui->addBtn->setVisible(!readOnly);
    ui->editBtn->setVisible(!readOnly);
    ui->deleteBtn->setVisible(!readOnly);
}

void ScheduleDialog::updateButtons()
{
    bool sel = ui->listWidget->currentRow() >= 0;
    ui->editBtn->setEnabled(sel);
    ui->deleteBtn->setEnabled(sel);
}
