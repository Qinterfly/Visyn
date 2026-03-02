#include <QLabel>
#include <QTimer>

#include "customstatusbar.h"

using namespace Frontend;

CustomStatusBar::CustomStatusBar(QWidget* pParent)
    : QStatusBar(pParent)
{
    createContent();
    createConnections();
}

//! Create all the widgets
void CustomStatusBar::createContent()
{
    // Create the label
    mpMessageLabel = new QLabel;
    mpMessageLabel->setWordWrap(true);
    addPermanentWidget(mpMessageLabel, 1);

    // Create the timer
    mpTimer = new QTimer(this);
}

//! Create widget signals and slots
void CustomStatusBar::createConnections()
{
    connect(mpTimer, &QTimer::timeout, this,
            [this]()
            {
                mpMessageLabel->clear();
                mpTimer->stop();
            });
}

//! Display the message and destroy it after timeout, if specified
void CustomStatusBar::showMessage(QtMsgType type, QString const& message, int timeout)
{
    // Constants
    QChar kComma = '\"';
    int kMaxLength = 128;

    // Set the data to output
    QString filterMessage = message;
    if (filterMessage.endsWith(kComma))
        filterMessage.removeAt(filterMessage.size() - 1);
    if (filterMessage.startsWith(kComma))
        filterMessage.removeAt(0);
    filterMessage.truncate(kMaxLength);

    // Determine the message color
    QColor color;
    switch (type)
    {
    case QtDebugMsg:
        color = QColor("gray");
        break;
    case QtInfoMsg:
        color = QColor("white");
        break;
    case QtWarningMsg:
        color = QColor("orange");
        break;
    case QtCriticalMsg:
        color = QColor("red");
        break;
    case QtFatalMsg:
        color = QColor("darkred");
        break;
    }

    // Stop the timer
    if (mpTimer->isActive())
        mpTimer->stop();

    // Set the message
    try
    {
        mpMessageLabel->setPalette(QPalette(color));
        mpMessageLabel->setText(filterMessage);
    }
    catch (...)
    {
        return;
    }

    // Run the timer to clear message
    if (timeout > 0)
    {
        mpTimer->setInterval(timeout);
        mpTimer->start();
    }
}
