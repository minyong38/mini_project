#include "CustomCalendarWidget.h"
#include <QPainter>
#include <QFontMetrics>
#include <QHash>

// ── 공휴일 ────────────────────────────────────────────────────────────

QString CustomCalendarWidget::holidayName(const QDate& date)
{
    // 고정 공휴일 (월, 일)
    static const QHash<int, QString> fixed = {
        { 101,  "신정"         },
        { 301,  "삼일절"       },
        { 505,  "어린이날"     },
        { 606,  "현충일"       },
        { 815,  "광복절"       },
        { 1003, "개천절"       },
        { 1009, "한글날"       },
        { 1225, "크리스마스"   },
    };

    int key = date.month() * 100 + date.day();
    if (fixed.contains(key)) return fixed[key];

    // 음력 기반 공휴일 (연도별 양력 날짜 하드코딩)
    // 설날 연휴(전날·당일·다음날), 추석 연휴, 부처님오신날
    static const QHash<QDate, QString> lunar = {
        // ── 2024 ──
        { QDate(2024,  2,  9), "설날 연휴"       },
        { QDate(2024,  2, 10), "설날"            },
        { QDate(2024,  2, 11), "설날 연휴"       },
        { QDate(2024,  5, 15), "부처님오신날"    },
        { QDate(2024,  9, 16), "추석 연휴"       },
        { QDate(2024,  9, 17), "추석"            },
        { QDate(2024,  9, 18), "추석 연휴"       },
        // ── 2025 ──
        { QDate(2025,  1, 28), "설날 연휴"       },
        { QDate(2025,  1, 29), "설날"            },
        { QDate(2025,  1, 30), "설날 연휴"       },
        { QDate(2025,  5,  5), "부처님오신날"    },  // 어린이날과 겹침
        { QDate(2025, 10,  5), "추석 연휴"       },
        { QDate(2025, 10,  6), "추석"            },
        { QDate(2025, 10,  7), "추석 연휴"       },
        // ── 2026 ──
        { QDate(2026,  2, 16), "설날 연휴"       },
        { QDate(2026,  2, 17), "설날"            },
        { QDate(2026,  2, 18), "설날 연휴"       },
        { QDate(2026,  5, 24), "부처님오신날"    },
        { QDate(2026,  9, 24), "추석 연휴"       },
        { QDate(2026,  9, 25), "추석"            },
        { QDate(2026,  9, 26), "추석 연휴"       },
        // ── 2027 ──
        { QDate(2027,  2,  5), "설날 연휴"       },
        { QDate(2027,  2,  6), "설날"            },
        { QDate(2027,  2,  7), "설날 연휴"       },
        { QDate(2027,  5, 13), "부처님오신날"    },
        { QDate(2027, 10, 14), "추석 연휴"       },
        { QDate(2027, 10, 15), "추석"            },
        { QDate(2027, 10, 16), "추석 연휴"       },
        // ── 2028 ──
        { QDate(2028,  1, 25), "설날 연휴"       },
        { QDate(2028,  1, 26), "설날"            },
        { QDate(2028,  1, 27), "설날 연휴"       },
        { QDate(2028,  5,  2), "부처님오신날"    },
        { QDate(2028, 10,  2), "추석 연휴"       },
        { QDate(2028, 10,  3), "추석"            },
        { QDate(2028, 10,  4), "추석 연휴"       },
        // ── 2029 ──
        { QDate(2029,  2, 12), "설날 연휴"       },
        { QDate(2029,  2, 13), "설날"            },
        { QDate(2029,  2, 14), "설날 연휴"       },
        { QDate(2029,  5, 20), "부처님오신날"    },
        { QDate(2029,  9, 21), "추석 연휴"       },
        { QDate(2029,  9, 22), "추석"            },
        { QDate(2029,  9, 23), "추석 연휴"       },
        // ── 2030 ──
        { QDate(2030,  2,  2), "설날 연휴"       },
        { QDate(2030,  2,  3), "설날"            },
        { QDate(2030,  2,  4), "설날 연휴"       },
        { QDate(2030,  5,  9), "부처님오신날"    },
        { QDate(2030,  9, 11), "추석 연휴"       },
        { QDate(2030,  9, 12), "추석"            },
        { QDate(2030,  9, 13), "추석 연휴"       },
    };

    return lunar.value(date, QString());
}

// ─────────────────────────────────────────────────────────────────────

CustomCalendarWidget::CustomCalendarWidget(QWidget *parent)
    : QCalendarWidget(parent)
{
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
}

void CustomCalendarWidget::setMonthSchedules(const QMap<QDate, QStringList> &schedules)
{
    m_schedules = schedules;
    updateCells();
}

void CustomCalendarWidget::clearSchedules()
{
    m_schedules.clear();
    updateCells();
}

// content = "제목\t시작\t종료\t상세\t색상태그"
static QString titleOf(const QString& content)  { return content.split('\t').value(0); }
static QString colorTagOf(const QString& content) { return content.split('\t').value(4); }

static QColor chipBgColor(const QString& tag) {
    if (tag == "red")    return QColor("#FFCDD2");
    if (tag == "orange") return QColor("#FFE0B2");
    if (tag == "yellow") return QColor("#FFF9C4");
    if (tag == "green")  return QColor("#C8E6C9");
    if (tag == "purple") return QColor("#E1BEE7");
    return QColor("#E8F3FF");
}

static QColor chipTextColor(const QString& tag) {
    if (tag == "red")    return QColor("#C62828");
    if (tag == "orange") return QColor("#E65100");
    if (tag == "yellow") return QColor("#827717");
    if (tag == "green")  return QColor("#1B5E20");
    if (tag == "purple") return QColor("#6A1B9A");
    return QColor("#0051D5");
}

void CustomCalendarWidget::paintCell(QPainter *painter, const QRect &rect, QDate date) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setClipRect(rect);

    bool isCurrentMonth = (date.month() == monthShown() && date.year() == yearShown());
    bool isToday        = (date == QDate::currentDate());
    bool isSelected     = (date == selectedDate());
    bool isSunday       = (date.dayOfWeek() == 7);
    bool isSaturday     = (date.dayOfWeek() == 6);

    // ── 셀 배경 ──────────────────────────────────────
    const QColor bgBase    = m_darkMode ? QColor("#2C2C2E") : Qt::white;
    const QColor bgSelected= m_darkMode ? QColor("#1A3A5C") : QColor("#F0F6FF");
    painter->fillRect(rect, isSelected && !isToday ? bgSelected : bgBase);

    // ── 날짜 숫자 원 ──────────────────────────────────
    int circleD = 28;
    QRect circleRect(
        rect.left() + (rect.width() - circleD) / 2,
        rect.top() + 5,
        circleD, circleD
    );

    if (isToday) {
        painter->setBrush(QColor("#007AFF"));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(circleRect);
    } else if (isSelected) {
        painter->setBrush(QColor("#E0EDFF"));
        painter->setPen(QPen(QColor("#007AFF"), 1.5));
        painter->drawEllipse(circleRect);
    }

    // ── 날짜 숫자 ─────────────────────────────────────
    QString holiday = isCurrentMonth ? holidayName(date) : QString();
    bool isHoliday  = !holiday.isEmpty();

    QColor dateColor;
    if (isToday)              dateColor = Qt::white;
    else if (isSelected)      dateColor = QColor("#007AFF");
    else if (!isCurrentMonth) dateColor = m_darkMode ? QColor("#48484A") : QColor("#D1D1D6");
    else if (isSunday || isHoliday) dateColor = QColor("#FF3B30");
    else if (isSaturday)      dateColor = QColor("#007AFF");
    else                      dateColor = m_darkMode ? QColor("#FFFFFF") : QColor("#1C1C1E");

    QFont dateFont = painter->font();
    dateFont.setPixelSize(13);
    dateFont.setBold(isToday);
    painter->setFont(dateFont);
    painter->setPen(dateColor);
    painter->drawText(circleRect, Qt::AlignCenter, QString::number(date.day()));

    // ── 공휴일 이름 ───────────────────────────────────
    if (isHoliday) {
        QFont hf = painter->font();
        hf.setPixelSize(8);
        hf.setBold(false);
        painter->setFont(hf);
        painter->setPen(isToday ? Qt::white : QColor("#FF3B30"));
        QRect hRect(rect.left() + 1, circleRect.bottom() + 1, rect.width() - 2, 10);
        QFontMetrics hfm(hf);
        QString elided = hfm.elidedText(holiday, Qt::ElideRight, hRect.width());
        painter->drawText(hRect, Qt::AlignHCenter | Qt::AlignTop, elided);
    }

    // ── 일정 칩 ──────────────────────────────────────
    if (!isCurrentMonth) { painter->restore(); return; }

    auto it = m_schedules.find(date);
    if (it == m_schedules.end() || it.value().isEmpty()) { painter->restore(); return; }

    const QStringList& list = it.value();

    QFont chipFont = painter->font();
    chipFont.setPixelSize(10);
    painter->setFont(chipFont);
    QFontMetrics fm(chipFont);

    const int chipH  = 14;
    const int chipX  = rect.left() + 4;
    const int chipW  = rect.width() - 8;
    int startY       = circleRect.bottom() + (isHoliday ? 13 : 5);

    for (int i = 0; i < qMin(list.size(), 2); i++) {
        int chipY = startY + i * (chipH + 3);
        if (chipY + chipH > rect.bottom() - 2) break;

        QRect chipRect(chipX, chipY, chipW, chipH);

        if (i == 1 && list.size() > 2) {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QColor("#8E8E93"));
            painter->drawText(chipRect, Qt::AlignLeft | Qt::AlignVCenter,
                              QString("+%1 더").arg(list.size() - 1));
        } else {
            QString tag  = colorTagOf(list[i]);
            QColor  bg   = chipBgColor(tag);
            QColor  text = chipTextColor(tag);

            painter->setBrush(bg);
            painter->setPen(Qt::NoPen);
            painter->drawRoundedRect(chipRect, 3, 3);

            painter->setPen(text);
            QString title = fm.elidedText(titleOf(list[i]), Qt::ElideRight, chipW - 6);
            painter->drawText(chipRect.adjusted(4, 0, -2, 0),
                              Qt::AlignLeft | Qt::AlignVCenter, title);
        }
    }

    painter->restore();
}
