#include "apiclient.h"
#include "constants.h"
#include "avatarcache.h"

#include <QJsonArray>
#include <QNetworkReply>
#include <QDebug>
#include <QUrl>
#include <QPixmap>

ApiClient& ApiClient::instance()
{
    static ApiClient s_instance;
    return s_instance;
}

ApiClient::ApiClient(QObject* parent)
    : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
    m_nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

void ApiClient::get(const QString&                       endpoint,
                    const QList<QPair<QString,QString>>& params,
                    ApiCallback                          callback)
{
    // Build full URL with query parameters
    QUrl url(QString("%1%2")
                 .arg(AppConstants::GITHUB_API_URL)
                 .arg(endpoint));

    if (!params.isEmpty()) {
        QUrlQuery query;
        for (const auto& [key, value] : params)
            query.addQueryItem(key, value);
        url.setQuery(query);
    }

    if (!url.isValid()) {
        ApiResponse resp;
        resp.success  = false;
        resp.errorMsg = "Invalid URL constructed: " + url.toString();
        callback(resp);
        return;
    }

    QNetworkRequest request = buildRequest(url);
    request.setTransferTimeout(REQUEST_TIMEOUT_MS);

    QNetworkReply* reply = m_nam->get(request);
    m_activeReplies.append(reply);

    emit requestStarted();

    // Lambda captures reply pointer + callback; 'this' for m_activeReplies access
    connect(reply, &QNetworkReply::finished,
            this, [this, reply, callback]() {
                m_activeReplies.removeOne(reply);
                processReply(reply, callback);
                emit requestFinished();
            });

    qDebug() << "[ApiClient] GET" << url.toString();
}

QNetworkRequest ApiClient::buildRequest(const QUrl& url) const
{
    QNetworkRequest request(url);

    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    request.setRawHeader("User-Agent",
                         QByteArray("GitHubExplorer/") +
                             QByteArray(AppConstants::APP_VERSION));

    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization",
                             QByteArray("Bearer ") + m_authToken.toUtf8());
    }

    return request;
}

void ApiClient::processReply(QNetworkReply* reply, const ApiCallback& callback)
{
    ApiResponse response;
    extractRateLimit(reply);
    response.rateLimitRemaining = m_rateRemaining;
    response.rateLimitReset     = m_rateReset;

    // Check for deliberate cancellation (abort() sets OperationCanceledError)
    if (reply->error() == QNetworkReply::OperationCanceledError) {
        response.isCancelled = true;
        qDebug() << "[ApiClient] Request cancelled (superseded by new query)";
        reply->deleteLater();
        callback(response);
        return;
    }

    // Read HTTP status code BEFORE checking reply->error()
    // Qt maps HTTP errors (404, 403, etc.) to NetworkError enum values
    const QVariant statusAttr =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if (statusAttr.isValid()) {
        response.statusCode = statusAttr.toInt();
        const QByteArray rawData = reply->readAll();

        QJsonParseError parseError;
        const QJsonDocument doc =
            QJsonDocument::fromJson(rawData, &parseError);

        if (response.statusCode >= 200 && response.statusCode < 300) {
            // HTTP Success
            if (parseError.error != QJsonParseError::NoError) {
                response.success  = false;
                response.errorMsg = QString("Failed to parse response: %1")
                                        .arg(parseError.errorString());
                qWarning() << "[ApiClient] JSON parse error:"
                           << parseError.errorString();
            } else {
                response.success = true;
                response.data    = doc;
                qDebug() << "[ApiClient] Success, status:" << response.statusCode
                         << "rate remaining:" << m_rateRemaining;
            }
        } else {
            // HTTP Error (4xx / 5xx)
            const QJsonObject body =
                doc.isNull() ? QJsonObject{} : doc.object();
            response.success  = false;
            response.errorMsg = httpErrorToString(response.statusCode, body);
            qWarning() << "[ApiClient] HTTP error:" << response.statusCode
                       << response.errorMsg;
        }

        reply->deleteLater();
        callback(response);
        return;
    }

    // Pure network error (no HTTP response: DNS, no internet, SSL, timeout)
    if (reply->error() != QNetworkReply::NoError) {
        response.success  = false;
        response.errorMsg = networkErrorToString(reply->error());
        qWarning() << "[ApiClient] Network error:"
                   << reply->error() << reply->errorString();
        reply->deleteLater();
        callback(response);
        return;
    }

    // Fallback — should never be reached
    response.success  = false;
    response.errorMsg = "Unknown error occurred.";
    reply->deleteLater();
    callback(response);
}

void ApiClient::extractRateLimit(QNetworkReply* reply)
{
    const QByteArray remaining =
        reply->rawHeader("X-RateLimit-Remaining");
    const QByteArray reset =
        reply->rawHeader("X-RateLimit-Reset");

    if (!remaining.isEmpty())
        m_rateRemaining = remaining.toInt();

    if (!reset.isEmpty())
        m_rateReset = reset.toLongLong();

    if (!remaining.isEmpty() || !reset.isEmpty())
        emit rateLimitUpdated(m_rateRemaining, m_rateReset);
}

QString ApiClient::httpErrorToString(int statusCode, const QJsonObject& body)
{
    // GitHub's error body: { "message": "...", "documentation_url": "..." }
    const QString githubMsg = body.value("message").toString();

    switch (statusCode) {
    case 401:
        return "Authentication failed. Check your access token.";

    case 403:
        // GitHub uses 403 for both auth failures AND rate limit exceeded
        if (!githubMsg.isEmpty() &&
            githubMsg.contains("rate limit", Qt::CaseInsensitive))
            return "GitHub rate limit exceeded. Wait a few minutes or add a token.";
        return "Access forbidden. " + (githubMsg.isEmpty()
                                           ? QString("You may have exceeded the rate limit.")
                                           : githubMsg);

    case 404:
        return "User or resource not found. Check the username and try again.";

    case 422:
        return "Invalid request. " + (githubMsg.isEmpty()
                                          ? QString("Check the search query.")
                                          : githubMsg);

    case 500:
    case 502:
    case 503:
        return "GitHub is temporarily unavailable. Try again in a moment.";

    default:
        if (!githubMsg.isEmpty())
            return QString("GitHub error (%1): %2").arg(statusCode).arg(githubMsg);
        return QString("Unexpected error (HTTP %1).").arg(statusCode);
    }
}

QString ApiClient::networkErrorToString(QNetworkReply::NetworkError error)
{
    switch (error) {
    case QNetworkReply::ConnectionRefusedError:
        return "Connection refused by server.";

    case QNetworkReply::RemoteHostClosedError:
        return "Server closed the connection unexpectedly.";

    case QNetworkReply::HostNotFoundError:
        return "Cannot reach GitHub. Check your internet connection.";

    case QNetworkReply::TimeoutError:
    case QNetworkReply::OperationCanceledError:
        return "Request timed out. Check your connection and try again.";

    case QNetworkReply::SslHandshakeFailedError:
        return "Secure connection failed (SSL error).";

    case QNetworkReply::TemporaryNetworkFailureError:
    case QNetworkReply::NetworkSessionFailedError:
        return "Network unavailable. Check your internet connection.";

    case QNetworkReply::UnknownNetworkError:
    default:
        return "A network error occurred. Check your connection.";
    }
}

void ApiClient::cancelPendingRequests()
{
    // Iterate a copy since reply->abort() triggers finished() which modifies m_activeReplies
    const QList<QNetworkReply*> repliesCopy = m_activeReplies;
    for (QNetworkReply* reply : repliesCopy) {
        if (reply && reply->isRunning()) {
            reply->abort();
        }
    }
    qDebug() << "[ApiClient] Cancelled" << repliesCopy.size() << "pending request(s)";
}

void ApiClient::setAuthToken(const QString& token)
{
    m_authToken = token.trimmed();
    qDebug() << "[ApiClient] Auth token" << (m_authToken.isEmpty() ? "cleared" : "set")
             << "| authenticated:" << hasAuthToken();
}

bool ApiClient::hasAuthToken() const
{
    return !m_authToken.isEmpty();
}

ApiClient::RateLimitInfo ApiClient::rateLimitInfo() const
{
    return m_rateLimit;
}

void ApiClient::clearAuthToken()
{
    m_authToken.clear();
    qDebug() << "[ApiClient] Auth token cleared";
}

int    ApiClient::rateLimitRemaining() const { return m_rateRemaining; }
qint64 ApiClient::rateLimitReset()     const { return m_rateReset; }

void ApiClient::downloadPixmap(const QString& url,
                               std::function<void(const QPixmap&)> callback)
{
    if (url.isEmpty()) {
        callback(QPixmap{});
        return;
    }

    // Cache hit: AvatarCache returns pointer owned by QCache, copy immediately
    if (const QPixmap* cached = AvatarCache::instance().find(url)) {
        qDebug() << "[AvatarCache] HIT — serving from cache:" << url.right(40);
        callback(*cached);
        return;
    }

    // Cache miss: download and insert into cache
    qDebug() << "[AvatarCache] MISS — downloading:" << url.right(40);

    QNetworkRequest request{QUrl(url)};
    request.setRawHeader("User-Agent",
                         QByteArray("GitHubExplorer/") + QByteArray(AppConstants::APP_VERSION));
    request.setTransferTimeout(8000);

    QNetworkReply* reply = m_nam->get(request);

    connect(reply, &QNetworkReply::finished, this,
            [reply, url, callback]() {

                QPixmap pixmap;

                if (reply->error() == QNetworkReply::NoError) {
                    const QByteArray data = reply->readAll();
                    pixmap.loadFromData(data);

                    if (!pixmap.isNull()) {
                        AvatarCache::instance().insert(url, pixmap);
                    }
                } else {
                    qWarning() << "[ApiClient] Avatar download failed:"
                               << reply->errorString();
                }

                reply->deleteLater();
                callback(pixmap);
            });
}
