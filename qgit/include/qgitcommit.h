#pragma once

#include "qgitcommitdiffparent.h"
#include "qgitsignature.h"
#include "qgitdifffile.h"

#include <QDateTime>
#include <QString>
#include <QList>


class QGitCommit {
public:
    QGitCommit() = default;
    QGitCommit(const QString &id, const QList<QGitCommitDiffParent> &parents, const QDateTime &time, const QGitSignature &author, const QGitSignature &commiter, const QString &message);
    QGitCommit(const QGitCommit &other) = default;

    QGitCommit &operator=(QGitCommit &&other) noexcept;
    QGitCommit &operator=(const QGitCommit &other) = default;

    QString id() const;
    QList<QGitCommitDiffParent> parents() const;
    QDateTime time() const;
    QGitSignature author() const;
    QGitSignature commiter() const;
    QString message() const;

private:
    QString m_id;
    QList<QGitCommitDiffParent> m_parents;
    QDateTime m_time;
    QGitSignature m_author;
    QGitSignature m_commiter;
    QString m_message;
};
