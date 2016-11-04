#pragma once

#include <QString>
#include <git2.h>


class QGitDiffLine
{
public:
    QGitDiffLine();
    QGitDiffLine(const git_diff_line *line);
    QGitDiffLine(const QByteArray &content, git_off_t offset, int new_lineno, int num_lines, int old_lineno, char origin);
    QGitDiffLine(const QGitDiffLine &other);

    QGitDiffLine &operator=(QGitDiffLine &&other);
    QGitDiffLine &operator=(const QGitDiffLine &other);

    QByteArray content() const;
    git_off_t offset() const;
    int new_lineno() const;
    int num_lines() const;
    int old_lineno() const;
    char origin() const;

private:
    QByteArray m_content;
    git_off_t m_offset = 0;
    int m_new_lineno = 0;
    int m_num_lines = 0;
    int m_old_lineno = 0;
    char m_origin = '\0';
};
