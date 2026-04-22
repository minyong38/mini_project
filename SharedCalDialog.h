#ifndef SHAREDCALDIALOG_H
#define SHAREDCALDIALOG_H

#include <QDialog>
#include <QString>
#include <QStringList>

class QLineEdit;
class QListWidget;

class SharedCalDialog : public QDialog {
    Q_OBJECT
public:
    explicit SharedCalDialog(const QStringList& friends, QWidget* parent = nullptr);

    QString     calendarName()    const;
    QStringList selectedFriends() const;

private:
    QLineEdit*   m_nameEdit   = nullptr;
    QListWidget* m_friendList = nullptr;
};

#endif
