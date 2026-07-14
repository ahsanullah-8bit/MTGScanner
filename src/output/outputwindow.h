#pragma once

#include <QObject>
#include <QWindow>
#include <QListView>
#include <QScreen>
#include <QWidget>

#include <output/nameplatemodel.h>

namespace MTGS {

class TransparentWindow : public QWidget {
public:
    TransparentWindow(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
};

class OutputWindow : public QObject {
    Q_OBJECT
public:
    explicit OutputWindow(const QString &name, const QRect &geometry, bool transparent = true, bool frameless = true, QScreen *screen = nullptr, NameplateModel *model = nullptr, QObject *parent = nullptr);
    ~OutputWindow();
    void setModel(NameplateModel *model);
    void open(QWindow *mainWindow);
    void close();
    
    template<typename T>
    T *model() const { return qobject_cast<T*>(m_listview->model()); }
private:
    QListView *m_listview = nullptr;
    QWidget *m_window = nullptr;
};

} // namespace MTGS