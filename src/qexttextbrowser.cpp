#include "qexttextbrowser.h"

#include <QNetworkAccessManager>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QNetworkReply>
#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QFile>

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
                m_nam.get(QNetworkRequest(name));
            }
        }
    }

    return QPixmap();
}

void QExtTextBrowser::imgDownloaded(QNetworkReply *reply)
{
    if (!reply) return;

    QUrl url = reply->request().url();
    m_inFlightUrls.remove(url);

    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray ba = reply->readAll();

        if (!ba.isEmpty())
        {
            QByteArray urlHash = QCryptographicHash::hash(url.url().toUtf8(), QCryptographicHash::Sha1).toHex();
            QString filename = m_imgPath + "/" + urlHash;

            QFile file(filename);
            if (file.open(QIODevice::WriteOnly | QIODevice::Truncate) == true)
                file.write(ba);

            QPixmap mPixmap;
            mPixmap.loadFromData(ba);
            if (!mPixmap.isNull()) {
                document()->addResource(QTextDocument::ImageResource, url, mPixmap);
                document()->markContentsDirty(0, document()->characterCount());
                viewport()->update();
            }
        }
    }

    reply->deleteLater();
}
