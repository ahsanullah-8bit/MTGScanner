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

OutputWindow::OutputWindow(bool transparent, bool frameless, NameplateModel *model, QWidget *parent)
    : QWidget(parent, frameless ? Qt::Window | Qt::FramelessWindowHint : Qt::Window)
{
    setAttribute(Qt::WA_TranslucentBackground, transparent);

    m_listview = new QListView(this);
    m_listview->setModel(model);
    m_listview->setItemDelegate(new NameplateDelegate(m_listview));
    m_listview->setResizeMode(QListView::Adjust);
    // Make listview transparent
    m_listview->setAutoFillBackground(false);
    m_listview->setAttribute(Qt::WA_TranslucentBackground, true);
    if (m_listview->viewport()) {
        m_listview->viewport()->setAutoFillBackground(false);
        m_listview->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
    }
    m_listview->setFrameShape(QFrame::NoFrame);

    setLayout(new QVBoxLayout(this));
    layout()->addWidget(m_listview);
}

OutputWindow::~OutputWindow()
{}

void OutputWindow::setModel(NameplateModel *model)
{
    m_listview->setModel(model);
}

void OutputWindow::open(QWindow *transientParent)
{
    show();
    windowHandle()->setTransientParent(transientParent);
}

NameplateModel *OutputWindow::model() const
{
    return qobject_cast<NameplateModel*>(m_listview->model());
}

}