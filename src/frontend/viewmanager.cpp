#include <DockAreaWidget.h>
#include <DockManager.h>
#include <QVBoxLayout>

#include "viewmanager.h"

using namespace Frontend;
using namespace ads;

ViewManager::ViewManager(QWidget* pParent)
    : QWidget(pParent)
{
}

ViewManager::~ViewManager()
{
    clear();
}

QSize ViewManager::sizeHint() const
{
    return QSize(800, 1000);
}

//! Destroy all views
void ViewManager::clear()
{
    QList<CDockWidget*> widgets = mpDockManager->openedDockWidgets();
    int numWidgets = widgets.size();
    for (int i = 0; i != numWidgets; ++i)
        widgets[i]->closeDockWidget();
}

//! Create all the widgets and corresponding actions
void ViewManager::createContent()
{
    // Create the dock manager
    createDockManager();

    // Insert the widgets into the main layout
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->addWidget(mpDockManager);
    setLayout(pLayout);
}

//! Construct the docking manager
void ViewManager::createDockManager()
{
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    CDockManager::setConfigFlag(CDockManager::AllTabsHaveCloseButton, true);
    CDockManager::setConfigFlag(CDockManager::DockAreaHasCloseButton, false);
    mpDockManager = new CDockManager(this);
}

//! Add widget as a new tab
void ViewManager::addTab(QWidget* pWidget, QIcon const& icon, QString const& title)
{
    CDockWidget* pDockWidget = new CDockWidget(mpDockManager, title);
    pDockWidget->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    pDockWidget->setWidget(pWidget);
    pDockWidget->setIcon(icon);
    mpDockManager->addDockWidgetTab(ads::CenterDockWidgetArea, pDockWidget);
}

//! Retrieve the current dock area
CDockAreaWidget* ViewManager::currentDockArea()
{
    QList<CDockAreaWidget*> dockAreas = mpDockManager->openedDockAreas();
    if (dockAreas.empty())
        return nullptr;
    return dockAreas.last();
}

//! Retrieve the current dock widget
CDockWidget* ViewManager::currentDockWidget()
{
    CDockAreaWidget* pArea = currentDockArea();
    if (!pArea)
        return nullptr;
    return pArea->currentDockWidget();
}

//! Find the requested widget
CDockWidget* ViewManager::findWidget(QWidget* pWidget)
{
    QList<CDockWidget*> dockWidgets = mpDockManager->openedDockWidgets();
    int numWidgets = dockWidgets.size();
    for (int i = 0; i != numWidgets; ++i)
    {
        if (dockWidgets[i]->widget() == pWidget)
            return dockWidgets[i];
    }
    return nullptr;
}

//! Set the current tab as well as title
void ViewManager::setCurrentWidget(QWidget* pWidget, QString const& title)
{
    CDockWidget* pDockWidget = findWidget(pWidget);
    if (pDockWidget)
    {
        pDockWidget->setAsCurrentTab();
        if (!title.isEmpty())
            pDockWidget->setWindowTitle(title);
    }
}
