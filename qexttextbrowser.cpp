#include "qexttextbrowser.h"

#include <QNetworkAccessManager>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QNetworkReply>
#include <QApplication>
#include <QByteArray>
#include <QDir>


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
        QByteArray urlHash = QCryptographicHash::hash(name.url().toUtf8(), QCryptographicHash::Sha1).toHex();
        QString filename = m_imgPath + "/" + urlHash;

        if(QFile::exists(filename))
        {
            QFile file(filename);
            file.open(QIODevice::ReadOnly);
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
            m_nam.get(QNetworkRequest(name));
        }
    }

    return QPixmap();
}

void QExtTextBrowser::imgDownloaded(QNetworkReply *reply)
{
    QByteArray ba = reply->readAll();

    if (!ba.isEmpty())
    {
        QByteArray urlHash = QCryptographicHash::hash(reply->request().url().url().toUtf8(), QCryptographicHash::Sha1).toHex();
        QString filename = m_imgPath + "/" + urlHash;

        QFile file(filename);
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);
        file.write(ba);
        file.close();

        //reload();
        setHtml(toHtml());
    }
}
