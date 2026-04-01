#include "avatarcache.h"

#include <QDebug>

// 20 MB cache budget in bytes
static constexpr qsizetype AVATAR_CACHE_MAX_BYTES = 20LL * 1024 * 1024;

AvatarCache& AvatarCache::instance()
{
    static AvatarCache s_instance;
    return s_instance;
}

AvatarCache::AvatarCache()
    : m_cache(AVATAR_CACHE_MAX_BYTES)
{
    qDebug() << "[AvatarCache] Initialised — budget:"
             << AVATAR_CACHE_MAX_BYTES / (1024 * 1024) << "MB";
}

const QPixmap* AvatarCache::find(const QString& url) const
{
    return m_cache.object(url);
}

bool AvatarCache::insert(const QString& url, const QPixmap& pixmap)
{
    if (pixmap.isNull() || url.isEmpty()) return false;

    const qsizetype cost = pixmapCost(pixmap);

    // Allocate heap copy; QCache takes ownership and deletes when evicting
    bool inserted = m_cache.insert(url, new QPixmap(pixmap), cost);

    if (inserted) {
        qDebug() << "[AvatarCache] Cached avatar for URL:"
                 << url.right(50)
                 << "| cost:" << cost / 1024 << "KB"
                 << "| total:" << totalCostKB() << "KB"
                 << "/" << maxCostKB() << "KB"
                 << "| entries:" << count();
    } else {
        qWarning() << "[AvatarCache] Insert FAILED — pixmap cost"
                   << cost / 1024 << "KB exceeds maxCost"
                   << maxCostKB() << "KB";
    }

    return inserted;
}

void AvatarCache::remove(const QString& url)
{
    m_cache.remove(url);
}

void AvatarCache::clear()
{
    m_cache.clear();
    qDebug() << "[AvatarCache] Cleared.";
}

int AvatarCache::count() const
{
    return static_cast<int>(m_cache.count());
}

int AvatarCache::totalCostKB() const
{
    return static_cast<int>(m_cache.totalCost() / 1024);
}

int AvatarCache::maxCostKB() const
{
    return static_cast<int>(m_cache.maxCost() / 1024);
}

// Returns byte footprint: width × height × (depth / 8)
// Example: 56×56 at 32 bpp = 56 × 56 × 4 = 12,544 bytes
qsizetype AvatarCache::pixmapCost(const QPixmap& pm)
{
    if (pm.isNull()) return 1;
    const qsizetype bytes =
        static_cast<qsizetype>(pm.width())
        * static_cast<qsizetype>(pm.height())
        * static_cast<qsizetype>(pm.depth() / 8);
    return qMax(bytes, qsizetype(1));
}
