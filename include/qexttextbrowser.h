#pragma once

#include <QNetworkAccessManager>
#ifndef QT_NO_SSL
#include <QSslError>
#endif
#include <QTextBrowser>
#include <QVariant>
#include <QString>
#include <QSet>
#include <QUrl>


class QExtTextBrowser : public QTextBrowser
{
    Q_OBJECT

public:
    explicit QExtTextBrowser(QWidget* parent = nullptr);
    ~QExtTextBrowser() = default;

    QVariant loadResource(int type, const QUrl &name) override;

private slots:
    void imgDownloaded(QNetworkReply *reply);
#ifndef QT_NO_SSL
    void sslErrorsOccurred(QNetworkReply *reply, const QList<QSslError> &errors);
#endif

private:    
    QNetworkAccessManager m_nam;
    QString m_imgPath;
    QSet<QUrl> m_inFlightUrls;
};
