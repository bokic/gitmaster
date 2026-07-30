#include "qcomboboxgitbase.h"

class QComboBoxGitDiffOptions : public QComboBoxGitBase
{
    Q_OBJECT
public:
    explicit QComboBoxGitDiffOptions(QWidget *parent = nullptr);
    ~QComboBoxGitDiffOptions() override = default;

    bool ignoreWhitespace() const;
    int linesOfContent() const;

signals:
    void optionsChanged();

protected:
    void updateIconColor() override;

private Q_SLOTS:
    void activated(int index);

private:
    QIcon m_icon;
    QIcon m_iconChecked;
    QIcon m_iconUnchecked;
    bool m_showIcons = false;
};
