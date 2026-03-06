#ifndef CUSTOMSTATUSBAR_H
#define CUSTOMSTATUSBAR_H

#include <QStatusBar>

QT_FORWARD_DECLARE_CLASS(QLabel)

namespace Frontend
{

class CustomStatusBar : public QStatusBar
{
public:
    CustomStatusBar(QWidget* pWidget = nullptr);
    virtual ~CustomStatusBar() = default;

    void showMessage(QtMsgType type, QString const& message, int timeout = 0);

private:
    void createContent();
    void createConnections();

private:
    QLabel* mpIconLabel;
    QLabel* mpMessageLabel;
    QTimer* mpTimer;
};

}

#endif // CUSTOMSTATUSBAR_H
