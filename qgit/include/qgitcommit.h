#pragma once

#include "qgitsignature.h"
#include "qgitdifffile.h"

#include <QDateTime>
#include <QString>
#include <QList>


class QGitCommit {
public:
    QGitCommit();
    QGitCommit(const QList<QGitDiffFile> &diff, const QString &id, const QList<QString> &parents, const QDateTime &time, const QGitSignature &author, const QGitSignature &commiter, const QString &message);
    QGitCommit(const QGitCommit &other);

    QGitCommit &operator=(QGitCommit &&other);
    QGitCommit &operator=(const QGitCommit &other);

    QList<QGitDiffFile> diff() const;
    QString id() const;
    QList<QString> parents() const;
    QDateTime time() const;
    QGitSignature author() const;
    QGitSignature commiter() const;
    QString message() const;

private:
    QList<QGitDiffFile> m_diff;
    QString m_id;
    QList<QString> m_parents;
    QDateTime m_time;
    QGitSignature m_author;
    QGitSignature m_commiter;
    QString m_message;
};
