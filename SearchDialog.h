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
    struct SearchResult {
        int     calId;    // 0 = 내 캘린더, >0 = 공유 캘린더 ID
        QString calName;
        QString date;
        QString content;
    };

    explicit SearchDialog(QWidget* parent = nullptr);

    void setResults(const QList<SearchResult>& results);
    void setDarkMode(bool dark);

signals:
    void searchRequested(const QString& keyword);
    void dateSelected(const QDate& date, int calId);

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
