#include "qexttextbrowser.h"

#include <QNetworkAccessManager>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QNetworkReply>
#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QFile>
#ifndef QT_NO_SSL
#include <QSslError>
#endif

QExtTextBrowser::QExtTextBrowser(QWidget* parent)
    : QTextBrowser(parent)
    , m_imgPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).append("/webimgs"))
{
    QDir dir;

    if (!dir.exists(m_imgPath))
    {
        dir.mkpath(m_imgPath);
    }

    connect(&m_nam, &QNetworkAccessManager::finished, this, &QExtTextBrowser::imgDownloaded);
#ifndef QT_NO_SSL
    connect(&m_nam, &QNetworkAccessManager::sslErrors, this, &QExtTextBrowser::sslErrorsOccurred);
#endif
}

QVariant QExtTextBrowser::loadResource(int type, const QUrl &name)
{
    if (type == QTextDocument::ImageResource)
    {
        if (name.scheme().toLower() != QStringLiteral("https"))
        {
            return QPixmap();
        }

        QByteArray urlHash = QCryptographicHash::hash(name.url().toUtf8(), QCryptographicHash::Sha1).toHex();
        QString filename = m_imgPath + "/" + urlHash;

        if(QFile::exists(filename))
        {
            QFile file(filename);
            if (file.open(QIODevice::ReadOnly) == false)
                return QPixmap();

            QByteArray ba = file.readAll();

            if (!ba.isEmpty())
            {
                QPixmap mPixmap;
                mPixmap.loadFromData(ba);
                return mPixmap;
            }
        }
        else
        {
            if (!m_inFlightUrls.contains(name))
            {
                m_inFlightUrls.insert(name);
                QNetworkRequest req(name);
                req.setMaximumRedirectsAllowed(3);
                req.setTransferTimeout(10000);
                m_nam.get(req);
            }
        }
    }

    return QPixmap();
}

void QExtTextBrowser::imgDownloaded(QNetworkReply *reply)
{
    if (!reply) return;
    reply->deleteLater();

    QUrl url = reply->request().url();
    m_inFlightUrls.remove(url);

    if (reply->error() == QNetworkReply::NoError)
    {
        constexpr qint64 kMaxImageDownloadBytes = 2 * 1024 * 1024; // 2 MB
        constexpr qint64 kMaxDiskCacheBytes = 50 * 1024 * 1024;    // 50 MB

        const QVariant lenHeader = reply->header(QNetworkRequest::ContentLengthHeader);
        if (lenHeader.isValid() && lenHeader.toLongLong() > kMaxImageDownloadBytes) {
            qWarning() << "QExtTextBrowser: Image exceeds max download size:" << url;
            return;
        }

        QByteArray ba = reply->read(kMaxImageDownloadBytes + 1);
        if (ba.size() > kMaxImageDownloadBytes) {
            qWarning() << "QExtTextBrowser: Image download truncated at max size:" << url;
            return;
        }

        if (!ba.isEmpty())
        {
            QPixmap mPixmap;
            mPixmap.loadFromData(ba);
            if (!mPixmap.isNull()) {
                QByteArray urlHash = QCryptographicHash::hash(url.url().toUtf8(), QCryptographicHash::Sha1).toHex();
                QString filename = m_imgPath + "/" + urlHash;

                QFile file(filename);
                if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    file.write(ba);
                    file.close();
                }

                // Enforce bounded disk cache by evicting oldest entries first
                QDir cacheDir(m_imgPath);
                const QFileInfoList entries = cacheDir.entryInfoList(QDir::Files, QDir::Time | QDir::Reversed);
                qint64 totalSize = 0;
                for (const QFileInfo &fi : entries) {
                    totalSize += fi.size();
                }
                for (const QFileInfo &fi : entries) {
                    if (totalSize <= kMaxDiskCacheBytes) break;
                    totalSize -= fi.size();
                    QFile::remove(fi.absoluteFilePath());
                }

                document()->addResource(QTextDocument::ImageResource, url, mPixmap);
                document()->markContentsDirty(0, document()->characterCount());
                viewport()->update();
            }
        }
    }
    else
    {
        qWarning() << "QExtTextBrowser: Failed to download image:" << reply->errorString() << "for url:" << url;
    }
}

#ifndef QT_NO_SSL
void QExtTextBrowser::sslErrorsOccurred(QNetworkReply *reply, const QList<QSslError> &errors)
{
    for (const auto &err : errors) {
        qWarning() << "QExtTextBrowser SSL error:" << err.errorString() << "for url:" << (reply ? reply->url().toString() : QString());
    }
}
#endif
