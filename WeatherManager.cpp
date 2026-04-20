#include "WeatherManager.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QTimeZone>
#include <QUrl>
#include <QUrlQuery>

WeatherManager::WeatherManager(const QString& apiKey, QObject* parent)
    : QObject(parent), m_nam(new QNetworkAccessManager(this)), m_apiKey(apiKey)
{}

void WeatherManager::fetchWeather(double lat, double lon)
{
    auto baseQuery = [&]() {
        QUrlQuery q;
        q.addQueryItem("lat",   QString::number(lat, 'f', 4));
        q.addQueryItem("lon",   QString::number(lon, 'f', 4));
        q.addQueryItem("appid", m_apiKey);
        q.addQueryItem("units", "metric");
        q.addQueryItem("lang",  "kr");
        return q;
    };

    // 현재 날씨 (오늘)
    QUrl cu("https://api.openweathermap.org/data/2.5/weather");
    cu.setQuery(baseQuery());
    auto* r1 = m_nam->get(QNetworkRequest(cu));
    connect(r1, &QNetworkReply::finished, this, [this, r1]() {
        r1->deleteLater();
        if (r1->error() == QNetworkReply::NoError)
            parseCurrentWeather(r1->readAll());
        emit weatherUpdated();
    });

    // 5일 예보 (3시간 간격 × 40 ≈ 5일)
    QUrl fu("https://api.openweathermap.org/data/2.5/forecast");
    QUrlQuery fq = baseQuery();
    fq.addQueryItem("cnt", "40");
    fu.setQuery(fq);
    auto* r2 = m_nam->get(QNetworkRequest(fu));
    connect(r2, &QNetworkReply::finished, this, [this, r2]() {
        r2->deleteLater();
        if (r2->error() == QNetworkReply::NoError)
            parseForecast(r2->readAll());
        emit weatherUpdated();
    });
}

QString WeatherManager::conditionEmoji(int id)
{
    if (id >= 200 && id < 300) return "⛈";
    if (id >= 300 && id < 400) return "🌦";
    if (id == 500 || id == 501) return "🌧";
    if (id >= 502 && id < 512) return "⛈";
    if (id >= 520 && id < 532) return "🌧";
    if (id >= 600 && id < 700) return "❄";
    if (id >= 700 && id < 800) return "🌫";
    if (id == 800)              return "☀";
    if (id == 801)              return "🌤";
    if (id == 802)              return "⛅";
    if (id >= 803)              return "☁";
    return "🌡";
}

void WeatherManager::parseCurrentWeather(const QByteArray& json)
{
    QJsonObject root = QJsonDocument::fromJson(json).object();
    if (!root.contains("main")) return;

    WeatherInfo wi;
    QJsonObject main = root["main"].toObject();
    wi.tempMin   = main["temp_min"].toDouble();
    wi.tempMax   = main["temp_max"].toDouble();
    wi.humidity  = main["humidity"].toInt();
    wi.pressure  = main["pressure"].toInt();
    wi.windSpeed = root["wind"].toObject()["speed"].toDouble();
    wi.clouds    = root["clouds"].toObject()["all"].toInt();
    wi.rainMm    = root["rain"].toObject()["1h"].toDouble();
    wi.snowMm    = root["snow"].toObject()["1h"].toDouble();

    QJsonArray weather = root["weather"].toArray();
    if (!weather.isEmpty()) {
        auto w     = weather[0].toObject();
        wi.emoji       = conditionEmoji(w["id"].toInt());
        wi.description = w["description"].toString();
    }

    m_data[QDate::currentDate()] = wi;
}

void WeatherManager::parseForecast(const QByteArray& json)
{
    QJsonObject root = QJsonDocument::fromJson(json).object();
    QJsonArray  list = root["list"].toArray();

    struct DayAcc {
        double tMin = 999, tMax = -999;
        int    humSum = 0, pressSum = 0, cloudSum = 0, count = 0;
        double windMax = 0, popMax = 0, rainSum = 0, snowSum = 0;
        int    condId = 800;
        QString condDesc;
        bool   hasMidDay = false;
    };
    QMap<QDate, DayAcc> acc;

    // KST = UTC+9 (Windows IANA 지원 여부에 무관하게 오프셋 직접 사용)
    const QTimeZone kst(9 * 3600);

    for (const QJsonValue& v : list) {
        QJsonObject entry = v.toObject();
        qint64   dt    = static_cast<qint64>(entry["dt"].toDouble());
        QDateTime loc  = QDateTime::fromSecsSinceEpoch(dt, kst);
        QDate    date  = loc.date();
        int      hour  = loc.time().hour();

        DayAcc& a = acc[date];
        QJsonObject main = entry["main"].toObject();
        double tmin = main["temp_min"].toDouble();
        double tmax = main["temp_max"].toDouble();
        if (tmin < a.tMin) a.tMin = tmin;
        if (tmax > a.tMax) a.tMax = tmax;
        a.humSum   += main["humidity"].toInt();
        a.pressSum += main["pressure"].toInt();
        a.cloudSum += entry["clouds"].toObject()["all"].toInt();
        a.count++;

        double ws = entry["wind"].toObject()["speed"].toDouble();
        if (ws > a.windMax) a.windMax = ws;
        double pop = entry["pop"].toDouble();
        if (pop > a.popMax) a.popMax = pop;
        a.rainSum += entry["rain"].toObject()["3h"].toDouble();
        a.snowSum += entry["snow"].toObject()["3h"].toDouble();

        QJsonArray weather = entry["weather"].toArray();
        if (!weather.isEmpty()) {
            bool isMid = (hour >= 11 && hour <= 14);
            if (isMid || !a.hasMidDay) {
                a.condId   = weather[0].toObject()["id"].toInt();
                a.condDesc = weather[0].toObject()["description"].toString();
                if (isMid) a.hasMidDay = true;
            }
        }
    }

    QDate today = QDate::currentDate();
    for (auto it = acc.cbegin(); it != acc.cend(); ++it) {
        // 오늘은 현재날씨 API 결과를 우선 사용
        if (it.key() == today && m_data.contains(today)) continue;
        const DayAcc& a = it.value();
        if (a.count == 0) continue;

        WeatherInfo wi;
        wi.tempMin   = a.tMin;
        wi.tempMax   = a.tMax;
        wi.humidity  = a.humSum   / a.count;
        wi.pressure  = a.pressSum / a.count;
        wi.clouds    = a.cloudSum / a.count;
        wi.windSpeed = a.windMax;
        wi.pop       = a.popMax;
        wi.rainMm    = a.rainSum;
        wi.snowMm    = a.snowSum;
        wi.emoji     = conditionEmoji(a.condId);
        wi.description = a.condDesc;
        m_data[it.key()] = wi;
    }
}
