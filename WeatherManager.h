#ifndef WEATHERMANAGER_H
#define WEATHERMANAGER_H

#include <QObject>
#include <QMap>
#include <QDate>
#include <QNetworkAccessManager>

struct WeatherInfo {
    QString emoji;
    QString description;
    double  tempMin   = 0;
    double  tempMax   = 0;
    int     humidity  = 0;   // %
    double  windSpeed = 0;   // m/s
    double  pop       = 0;   // 강수 확률 0-1
    double  rainMm    = 0;   // mm
    double  snowMm    = 0;   // mm
    int     pressure  = 0;   // hPa
    int     clouds    = 0;   // % 운량
};

class WeatherManager : public QObject {
    Q_OBJECT
public:
    explicit WeatherManager(const QString& apiKey, QObject* parent = nullptr);

    // lat/lon 기본값: 서울
    void fetchWeather(double lat = 37.5665, double lon = 126.9780);
    const QMap<QDate, WeatherInfo>& data() const { return m_data; }

signals:
    void weatherUpdated();

private:
    static QString conditionEmoji(int id);
    void parseCurrentWeather(const QByteArray& json);
    void parseForecast(const QByteArray& json);

    QNetworkAccessManager*   m_nam;
    QString                  m_apiKey;
    QMap<QDate, WeatherInfo> m_data;
};

#endif
