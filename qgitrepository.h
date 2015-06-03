#ifndef QGITREPOSITORY_H
#define QGITREPOSITORY_H

#include <QWidget>

namespace Ui {
class QGitRepository;
}

class QGitRepository : public QWidget
{
    Q_OBJECT

public:
    explicit QGitRepository(QWidget *parent = 0);
    ~QGitRepository();

private:
    Ui::QGitRepository *ui;
};

#endif // QGITREPOSITORY_H
