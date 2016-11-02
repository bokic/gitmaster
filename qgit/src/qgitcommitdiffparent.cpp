#include "qgitcommitdiffparent.h"
#include <QDebug>


QGitCommitDiffParent::QGitCommitDiffParent(QByteArray commitHash)
    : m_commitHash(commitHash)
{
}

QGitCommitDiffParent::QGitCommitDiffParent(const QGitCommitDiffParent &other)
    : m_commitHash(other.m_commitHash)
    , m_files(other.m_files)
{
}

QGitCommitDiffParent &QGitCommitDiffParent::operator=(QGitCommitDiffParent &&other)
{
    qSwap(m_commitHash, other.m_commitHash);
    qSwap(m_files, other.m_files);

    return *this;
}

QGitCommitDiffParent &QGitCommitDiffParent::operator=(const QGitCommitDiffParent &other)
{
    m_commitHash = other.m_commitHash;
    m_files = other.m_files;

    return *this;
}

QByteArray QGitCommitDiffParent::commitHash() const
{
    return m_commitHash;
}

QList<QGitDiffFile> QGitCommitDiffParent::files() const
{
    return m_files;
}

void QGitCommitDiffParent::addFile(const git_diff_delta *delta)
{
    /*if (!hasFile(file.new_file().path()))
    {
        m_files.append(file);
    }
    else
    {
        qDebug() << "File already added!";
    }*/

    /*if (!dest->hasFile(QString::fromUtf8(delta->new_file.path))) {

        uint32_t newDiff_flags = delta->flags;
        uint16_t newDiff_nfiles = delta->nfiles;
        uint16_t newDiff_simularity = delta->similarity;
        git_delta_t newDiff_status = delta->status;

        QString new_file_path = QString::fromUtf8(delta->new_file.path);
        QByteArray new_file_id = QByteArray(reinterpret_cast<const char *>(delta->new_file.id.id), sizeof(delta->new_file.id.id));
        uint16_t new_file_mode = delta->new_file.mode;
        uint32_t new_file_flags = delta->new_file.flags;
        git_off_t new_file_size = delta->new_file.size;
        QGitDiffFileItem newDiff_new_file = QGitDiffFileItem(new_file_path, new_file_id, new_file_mode, new_file_flags, new_file_size);


        QString old_file_path = QString::fromUtf8(delta->old_file.path);
        QByteArray old_file_id = QByteArray(reinterpret_cast<const char *>(delta->old_file.id.id), sizeof(delta->old_file.id.id));
        uint16_t old_file_mode = delta->old_file.mode;
        uint32_t old_file_flags = delta->old_file.flags;
        git_off_t old_file_size = delta->old_file.size;
        QGitDiffFileItem newDiff_old_file = QGitDiffFileItem(old_file_path, old_file_id, old_file_mode, old_file_flags, old_file_size);

        QList<QGitDiffHunk> newDiff_hunks;

        dest->addFile(QGitDiffFile(newDiff_new_file, newDiff_old_file, newDiff_flags, newDiff_nfiles, newDiff_simularity, newDiff_status, newDiff_hunks));
    }
    else
    {
        qDebug() << "File already exists(git_diff_foreach)!";
    }*/
}

void QGitCommitDiffParent::addBinary(const git_diff_delta *delta, const git_diff_binary *binary)
{
    // TODO: implement QGitCommitDiffParent::addBinary()

    Q_UNIMPLEMENTED();
}

void QGitCommitDiffParent::addHunk(const git_diff_delta *delta, const git_diff_hunk *hunk)
{
    // TODO: implement QGitCommitDiffParent::addHulk()

    Q_UNIMPLEMENTED();
}

void QGitCommitDiffParent::addLine(const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line)
{
    // TODO: Implement void QGitCommitDiffParent::addHunk()

    Q_UNIMPLEMENTED();
}

bool QGitCommitDiffParent::hasFile(const QString &file)
{
    for(auto item: m_files)
    {
        if (item.new_file().path() == file)
        {
            return true;
        }
    }

    return false;
}
