#ifndef SCHEDULEDIALOG_H
#define SCHEDULEDIALOG_H

#include <QDialog>
#include <QDate>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QTimeEdit>
#include <QPlainTextEdit>
#include <QButtonGroup>

namespace Ui { class ScheduleDialog; }

// ── 일정 추가 / 수정용 입력 다이얼로그 ──────────────────────────────
class AddScheduleDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddScheduleDialog(QWidget* parent = nullptr);

    void setValues(const QString& title, const QString& startTime,
                   const QString& endTime, const QString& description,
                   const QString& colorTag = "");

    QString title()       const;
    QString startTime()   const;
    QString endTime()     const;
    QString description() const;
    QString colorTag()    const;

private:
    QLabel*         m_headerLabel;
    QLineEdit*      m_titleEdit;
    QCheckBox*      m_timeCheck;
    QTimeEdit*      m_startEdit;
    QTimeEdit*      m_endEdit;
    QPlainTextEdit* m_descEdit;
    QButtonGroup*   m_colorBtns;
};

// ── 날짜별 일정 목록 다이얼로그 ─────────────────────────────────────
class ScheduleDialog : public QDialog {
    Q_OBJECT
public:
    explicit ScheduleDialog(const QDate& date, QWidget *parent = nullptr);
    ~ScheduleDialog();

    void refreshSchedules(const QStringList& contents, const QList<qint64>& rowids);
    void setReadOnly(bool readOnly);

signals:
    void addRequested(const QString& content);
    void editRequested(qint64 rowid, const QString& newContent);
    void deleteRequested(qint64 rowid);

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void updateButtons();

private:
    Ui::ScheduleDialog* ui;
    QDate         m_date;
    QList<qint64> m_rowids;
    QStringList   m_contents;
};

#endif
