#pragma once

#include <QObject>
#include <QWindow>
#include <QListView>
#include <QScreen>
#include <QWidget>

#include <output/nameplatemodel.h>

namespace MTGS {

class OutputWindow : public QWidget {
public:
    explicit OutputWindow(bool transparent = true, bool frameless = true, NameplateModel *model = nullptr, QWidget *parent = nullptr);
    virtual ~OutputWindow();
    void setModel(NameplateModel *model);
    void open(QWindow *transientParent);

    NameplateModel *model() const;
private:
    QListView *m_listview = nullptr;
};

} // namespace MTGS