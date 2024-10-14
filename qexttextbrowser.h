#pragma once

#include <QNetworkAccessManager>
#include <QTextBrowser>
#include <QVariant>
#include <QString>
#include <QUrl>


class QExtTextBrowser : public QTextBrowser
{
public:
    explicit QExtTextBrowser(QWidget* parent = nullptr);
    QVariant loadResource(int type, const QUrl &name) override;

private slots:
    void imgDownloaded(QNetworkReply *reply);

private:    
    QNetworkAccessManager m_nam;
    QString m_imgPath;
};
