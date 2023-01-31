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
}

// FIXME: loadResource is been called 6 times for one URL!
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
            QNetworkReply* reply = m_nam.get(QNetworkRequest(name));

            connect(
                reply, &QNetworkReply::finished,
                this, [this, filename, reply]() {
                    QByteArray ba = reply->readAll();

                    if (!ba.isEmpty())
                    {
                        QFile file(filename);
                        file.open(QIODevice::WriteOnly | QIODevice::Truncate);
                        file.write(ba);
                        file.close();

                        QExtTextBrowser::reload(); // TODO: Reload do not work!
                    }
                }
            );
        }
    }

    return QPixmap();
}
