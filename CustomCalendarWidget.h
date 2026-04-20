#ifndef CUSTOMCALENDARWIDGET_H
#define CUSTOMCALENDARWIDGET_H

#include <QCalendarWidget>
#include <QMap>
#include <QDate>
#include <QStringList>

class CustomCalendarWidget : public QCalendarWidget {
    Q_OBJECT
public:
    explicit CustomCalendarWidget(QWidget *parent = nullptr);

    void setMonthSchedules(const QMap<QDate, QStringList> &schedules);
    void clearSchedules();
    void setDarkMode(bool dark) { m_darkMode = dark; updateCells(); }
    const QMap<QDate, QStringList> &schedules() const { return m_schedules; }

protected:
    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override;

private:
    QMap<QDate, QStringList> m_schedules;
    bool m_darkMode = false;

    static QString holidayName(const QDate& date);
};

#endif
