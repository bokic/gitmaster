#pragma once

#include <QString>


class QGitDiffLine
{
public:
    QGitDiffLine();
    QGitDiffLine(const QString &content, int offset, int new_lineno, int num_lines, int old_lineno, char origin);
    QGitDiffLine(const QGitDiffLine &other);

    QGitDiffLine &operator=(QGitDiffLine &&other);
    QGitDiffLine &operator=(const QGitDiffLine &other);

    QString content() const;
    int offset() const;
    int new_lineno() const;
    int num_lines() const;
    int old_lineno() const;
    char origin() const;

private:
    QString m_content;
    int m_offset = 0;
    int m_new_lineno = 0;
    int m_num_lines = 0;
    int m_old_lineno = 0;
    char m_origin = '\0';
};
