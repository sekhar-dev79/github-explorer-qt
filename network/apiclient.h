#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <functional>

struct ApiResponse {
    bool          success            = false;
    bool          isCancelled        = false;
    QJsonDocument data               = {};
    QString       errorMsg           = {};
    int           statusCode         = 0;
    int           rateLimitRemaining = -1;
    qint64        rateLimitReset     = 0;
};

using ApiCallback = std::function<void(const ApiResponse&)>;

class ApiClient : public QObject
{
    Q_OBJECT
public:
    static ApiClient& instance();

    void get(const QString&                   endpoint,
             const QList<QPair<QString,QString>>& params,
             ApiCallback                       callback);

    void setAuthToken(const QString& token);
    void clearAuthToken();
    [[nodiscard]] bool hasAuthToken() const;

    void cancelPendingRequests();

    [[nodiscard]] int    rateLimitRemaining() const;
    [[nodiscard]] qint64 rateLimitReset()     const;

    void downloadPixmap(const QString& url,
                        std::function<void(const QPixmap&)> callback);

    // Rate limit info updated after every successful response
    struct RateLimitInfo {
        int  remaining  = -1;   // requests remaining in current window
        int  limit      = -1;   // total limit (60 unauth, 5000 auth)
        QDateTime resetAt;      // UTC time when window resets
    };

    [[nodiscard]] RateLimitInfo rateLimitInfo() const;

signals:
    void rateLimitUpdated(int remaining, qint64 resetTimestamp);
    void requestStarted();
    void requestFinished();

private:
    explicit ApiClient(QObject* parent = nullptr);
    ApiClient(const ApiClient&)            = delete;
    ApiClient& operator=(const ApiClient&) = delete;

    [[nodiscard]] QNetworkRequest buildRequest(const QUrl& url) const;
    void processReply(QNetworkReply* reply, const ApiCallback& callback);
    [[nodiscard]] static QString httpErrorToString(int statusCode, const QJsonObject& body);
    [[nodiscard]] static QString networkErrorToString(QNetworkReply::NetworkError error);
    void extractRateLimit(QNetworkReply* reply);

    QNetworkAccessManager* m_nam             = nullptr;
    QString                m_authToken       = {};
    int                    m_rateRemaining   = 60;
    qint64                 m_rateReset       = 0;
    QList<QNetworkReply*>  m_activeReplies;
    RateLimitInfo          m_rateLimit;

    static constexpr int REQUEST_TIMEOUT_MS = 10'000;
};
