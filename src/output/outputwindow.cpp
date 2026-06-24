#include "outputwindow.h"

#include <QSize>
#include <QObject>
#include <QLayout>
#include <QWindow>
#include <QWidget>
#include <QListView>
#include <QBoxLayout>
#include <QVBoxLayout>

#include <output/nameplatemodel.h>
#include <output/nameplatdelegate.h>

namespace MTGS {

OutputWindow::OutputWindow(const QString &name, const QRect &geometry, QScreen *screen, NameplateModel *model, QObject *parent)
    : QObject(parent)
    , m_window(new QWidget(nullptr, Qt::Window))
{
    m_window->setWindowTitle(name);
    m_window->setGeometry(geometry);

    m_listview = new QListView(m_window);
    m_listview->setModel(model);
    m_listview->setItemDelegate(new NameplateDelegate(m_listview));
    m_listview->setResizeMode(QListView::Adjust);

    m_window->setLayout(new QVBoxLayout(m_window));
    m_window->layout()->addWidget(m_listview);
}

OutputWindow::~OutputWindow()
{
    m_window->close();
    m_window->deleteLater();
}

void OutputWindow::setModel(NameplateModel *model)
{
    m_listview->setModel(model);
}

void OutputWindow::open(QWindow *mainWindow)
{
    m_window->show();
    m_window->windowHandle()->setTransientParent(mainWindow);
}

void OutputWindow::close()
{
    m_window->close();
}

}