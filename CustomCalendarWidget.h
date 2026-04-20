#ifndef CUSTOMCALENDARWIDGET_H
#define CUSTOMCALENDARWIDGET_H

#include <QCalendarWidget>
#include <QMap>
#include <QDate>
#include <QStringList>
#include "WeatherManager.h"

class CustomCalendarWidget : public QCalendarWidget {
    Q_OBJECT
public:
    explicit CustomCalendarWidget(QWidget *parent = nullptr);

    void setMonthSchedules(const QMap<QDate, QStringList> &schedules);
    void clearSchedules();
    void setDarkMode(bool dark) { m_darkMode = dark; updateCells(); }
    void setWeatherData(const QMap<QDate, WeatherInfo>& data) { m_weatherData = data; updateCells(); }
    const QMap<QDate, QStringList> &schedules() const { return m_schedules; }

protected:
    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override;

private:
    QMap<QDate, QStringList>  m_schedules;
    QMap<QDate, WeatherInfo>  m_weatherData;
    bool m_darkMode = false;

    static QString holidayName(const QDate& date);
};

#endif
