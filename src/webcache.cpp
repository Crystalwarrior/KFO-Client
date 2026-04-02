#include "webcache.h"

#include "aoapplication.h"
#include "file_functions.h"
#include "options.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QUrl>

WebCache::WebCache(AOApplication *parent)
    : QObject(parent)
    , ao_app(parent)
    , m_network_manager(new QNetworkAccessManager(this))
{
  connect(m_network_manager, &QNetworkAccessManager::finished, this, &WebCache::onDownloadFinished);
}

WebCache::~WebCache()
{}

QString WebCache::cacheDir() const
{
  return QCoreApplication::applicationDirPath() + "/webcache/";
}

int WebCache::pendingDownloads() const
{
  return m_pending_downloads.size();
}

QString WebCache::cacheSubdir() const
{
  QString assetUrl = ao_app->asset_url;
  if (assetUrl.isEmpty())
  {
    return QString();
  }

  // Extract host and path from asset URL (e.g., "https://direct.grave.wine/base/" -> "direct.grave.wine/base/")
  QUrl url(assetUrl);
  QString subdir = url.host() + url.path();

  // Ensure it ends with /
  if (!subdir.endsWith('/'))
  {
    subdir += '/';
  }

  return subdir;
}

QString WebCache::getCachedPath(const QString &relativePath, const QStringList &suffixes) const
{
  if (relativePath.isEmpty())
  {
    return QString();
  }

  QString subdir = cacheSubdir();
  if (subdir.isEmpty())
  {
    return QString();
  }

  // Try each suffix
  QStringList effectiveSuffixes = suffixes.isEmpty() ? QStringList{""} : suffixes;
  for (const QString &suffix : effectiveSuffixes)
  {
    // Use lowercase path for cache lookup (no percent-encoding in local paths)
    QString lowerPath = ao_app->lowercasePath(relativePath + suffix);
    QString localPath = cacheDir() + subdir + lowerPath;

    if (!file_exists(localPath))
    {
      continue;
    }

    if (isExpired(localPath))
    {
      continue;
    }

    return localPath;
  }

  return QString();
}

bool WebCache::isExpired(const QString &localPath) const
{
  QFileInfo fileInfo(localPath);
  if (!fileInfo.exists())
  {
    return true;
  }

  int expiryHours = Options::getInstance().webcacheExpiryHours();
  QDateTime expiryTime = fileInfo.lastModified().addSecs(expiryHours * 3600);

  return QDateTime::currentDateTime() > expiryTime;
}

QString WebCache::resolve(const QString &relativePath, const QStringList &suffixes)
{
  if (relativePath.isEmpty())
  {
    return "";
  }

  // Reject paths containing absolute paths (they shouldn't be passed to webcache)
  if (relativePath.startsWith('/') || relativePath.contains("//") || relativePath.contains(":/"))
  {
    return "";
  }

  // Check if webcache is enabled
  if (!Options::getInstance().webcacheEnabled())
  {
    return "";
  }

  // Check if server has an asset URL
  QString assetUrl = ao_app->asset_url;
  if (assetUrl.isEmpty())
  {
    return "";
  }

  // Ensure asset URL ends with /
  if (!assetUrl.endsWith('/'))
  {
    assetUrl += '/';
  }

  // Get cache subdirectory for this server's asset URL
  QString subdir = cacheSubdir();
  if (subdir.isEmpty())
  {
    return "";
  }

  // Try each suffix (like webAO tries multiple extensions)
  QStringList effectiveSuffixes = suffixes.isEmpty() ? QStringList{""} : suffixes;
  for (const QString &suffix : effectiveSuffixes)
  {
    QString fullPath = relativePath + suffix;

    // Lowercase path for local storage and tracking (no percent-encoding)
    QString lowerPath = ao_app->lowercasePath(fullPath);

    // Check if already downloading this path
    if (m_pending_downloads.contains(lowerPath))
    {
      return "";
    }

    // Check if this path previously failed (don't retry within this session)
    if (m_failed_downloads.contains(lowerPath))
    {
      continue; // Try next suffix
    }

    // Local path uses lowercase without percent-encoding
    QString localPath = cacheDir() + subdir + lowerPath;

    // Skip if already cached and not expired
    if (file_exists(localPath) && !isExpired(localPath))
    {
      return ""; // Already have a valid cached file
    }

    // Only try one suffix at a time
    return lowerPath;
  }
  return "";
}

void WebCache::startDownload(const QString &remoteUrl, const QString &localPath, const QString &relativePath)
{
  m_pending_downloads.insert(relativePath, true);
  QNetworkRequest request{QUrl(remoteUrl)};
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

  // Store the local path and relative path in the request for later retrieval
  request.setAttribute(QNetworkRequest::User, localPath);
  request.setAttribute(static_cast<QNetworkRequest::Attribute>(QNetworkRequest::User + 1), relativePath);

  qDebug() << "WebCache: Downloading" << remoteUrl;
  m_network_manager->get(request);
}

void WebCache::onDownloadFinished(QNetworkReply *reply)
{
  qDebug() << "WebCache: Download finished";
  QString localPath = reply->request().attribute(QNetworkRequest::User).toString();
  QString relativePath = reply->request().attribute(static_cast<QNetworkRequest::Attribute>(QNetworkRequest::User + 1)).toString();

  // Remove from pending
  m_pending_downloads.remove(relativePath);

  if (reply->error() != QNetworkReply::NoError)
  {
    qDebug() << "WebCache: Download failed for" << reply->url().toString(QUrl::EncodeSpaces) << "-" << reply->errorString();
    // Notify listeners that a file has failed to be downloaded
    emit downloadFailed(relativePath);
    m_failed_downloads.insert(relativePath);
    reply->deleteLater();
    return;
  }

  // Check HTTP status code
  int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (statusCode != 200)
  {
    qDebug() << "WebCache: Download returned status" << statusCode << "for" << reply->url().toString(QUrl::EncodeSpaces);
    // Notify listeners that a file has failed to be downloaded
    emit downloadFailed(relativePath);
    m_failed_downloads.insert(relativePath);
    reply->deleteLater();
    return;
  }

  // Create directory structure
  QFileInfo fileInfo(localPath);
  QDir dir = fileInfo.absoluteDir();
  if (!dir.exists())
  {
    if (!dir.mkpath("."))
    {
      qWarning() << "WebCache: Failed to create directory" << dir.absolutePath();
      // Notify listeners that a file has failed to be downloaded
      emit downloadFailed(relativePath);
      reply->deleteLater();
      return;
    }
  }

  // Write the file
  QFile file(localPath);
  if (!file.open(QIODevice::WriteOnly))
  {
    qWarning() << "WebCache: Failed to open file for writing:" << localPath;
    // Notify listeners that a file has failed to be downloaded
    emit downloadFailed(relativePath);
    reply->deleteLater();
    return;
  }

  file.write(reply->readAll());
  file.close();

  qDebug() << "WebCache: Successfully cached" << localPath;

  // Notify listeners that a file has been downloaded
  emit fileDownloaded(relativePath);

  reply->deleteLater();
}

void WebCache::clearCache()
{
  QDir dir(cacheDir());
  if (dir.exists())
  {
    if (!dir.removeRecursively())
    {
      qWarning() << "WebCache: Failed to clear cache directory";
    }
    else
    {
      qDebug() << "WebCache: Cache cleared";
    }
  }
}

qint64 WebCache::getCacheSize() const
{
  QDir dir(cacheDir());
  if (!dir.exists())
  {
    return 0;
  }
  return calculateDirSize(dir);
}

qint64 WebCache::calculateDirSize(const QDir &dir) const
{
  qint64 size = 0;

  QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
  for (const QFileInfo &fileInfo : fileList)
  {
    size += fileInfo.size();
  }

  QFileInfoList dirList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
  for (const QFileInfo &dirInfo : dirList)
  {
    size += calculateDirSize(QDir(dirInfo.absoluteFilePath()));
  }

  return size;
}
