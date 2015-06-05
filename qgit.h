#ifndef QGIT_H
#define QGIT_H

#include <QObject>
#include <QDir>

class QGit : public QObject
{
    Q_OBJECT
public:
    explicit QGit(QObject *parent = 0);
    virtual ~QGit();

    void statusRepository(const QDir &path);

signals:

public slots:
};

#endif // QGIT_H
