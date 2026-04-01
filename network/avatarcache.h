#pragma once

#include <QCache>
#include <QPixmap>
#include <QString>

// LRU memory cache for downloaded avatar pixmaps
// Key = avatar URL, Value = heap-allocated QPixmap* (owned by QCache)
// Cost = pixmap byte size; 20 MB budget holds ~1,670 avatars
// Thread-safe for main thread only (all access through ApiClient)
class AvatarCache
{
public:
    [[nodiscard]] static AvatarCache& instance();

    // Returns cached pixmap for url, or nullptr if not cached.
    // Returned pointer is owned by QCache — copy if you need to keep it.
    [[nodiscard]] const QPixmap* find(const QString& url) const;

    // Inserts pixmap into cache. Returns true if successful.
    bool insert(const QString& url, const QPixmap& pixmap);

    // Removes a single entry
    void remove(const QString& url);

    // Clears entire cache
    void clear();

    // Debug helpers
    [[nodiscard]] int count()       const;
    [[nodiscard]] int totalCostKB() const;
    [[nodiscard]] int maxCostKB()   const;

private:
    AvatarCache();
    ~AvatarCache() = default;

    AvatarCache(const AvatarCache&)            = delete;
    AvatarCache& operator=(const AvatarCache&) = delete;

    // Storage: QCache<Key, T> manages T* lifetime via delete
    QCache<QString, QPixmap> m_cache;

    // Returns byte footprint: width × height × (depth / 8)
    [[nodiscard]] static qsizetype pixmapCost(const QPixmap& pm);
};
