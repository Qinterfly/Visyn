#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <QWidget>

namespace ads
{
class CDockManager;
class CDockWidget;
class CDockAreaWidget;
}

namespace Frontend
{

class ViewManager : public QWidget
{
    Q_OBJECT

public:
    ViewManager(QWidget* pParent = nullptr);
    virtual ~ViewManager();

    QSize sizeHint() const override;
    void refresh();
    void plot();
    void clear();

private:
    // Content
    void createContent();
    void createDockManager();

    // Helpers
    void addTab(QWidget* pWidget, QIcon const& icon, QString const& title);
    ads::CDockAreaWidget* currentDockArea();
    ads::CDockWidget* currentDockWidget();
    ads::CDockWidget* findWidget(QWidget* pWidget);
    void setCurrentWidget(QWidget* pWidget, QString const& title = QString());

private:
    ads::CDockManager* mpDockManager;
};

}

#endif // VIEWMANAGER_H
