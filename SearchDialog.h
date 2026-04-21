#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>

class SearchDialog : public QDialog {
    Q_OBJECT
public:
    explicit SearchDialog(QWidget* parent = nullptr);

    void setResults(const QList<QPair<QString, QString>>& results); // date, content
    void setDarkMode(bool dark);

signals:
    void searchRequested(const QString& keyword);
    void dateSelected(const QDate& date);

private slots:
    void onSearch();
    void onItemDoubleClicked(QListWidgetItem* item);

private:
    QLineEdit*   m_searchEdit;
    QPushButton* m_searchBtn;
    QListWidget* m_resultList;
    QLabel*      m_statusLabel;
    bool         m_darkMode = false;
};

#endif
