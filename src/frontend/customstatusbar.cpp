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
    mpIconLabel = new QLabel;
    mpMessageLabel = new QLabel;
    mpMessageLabel->setWordWrap(true);
    mpIconLabel->setFixedWidth(height() / 2);
    addPermanentWidget(mpIconLabel, 0);
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
                mpIconLabel->clear();
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
    QIcon icon;
    switch (type)
    {
    case QtDebugMsg:
        icon = QIcon(":/icons/dialog-debug.svg");
        break;
    case QtInfoMsg:
        icon = QIcon(":/icons/dialog-info.svg");
        break;
    case QtWarningMsg:
        icon = QIcon(":/icons/dialog-warning.svg");
        break;
    case QtCriticalMsg:
        icon = QIcon(":/icons/dialog-error.svg");
        break;
    case QtFatalMsg:
        icon = QIcon(":/icons/dialog-fatal.svg");
        break;
    }

    // Stop the timer
    if (mpTimer->isActive())
        mpTimer->stop();

    // Set the message
    try
    {
        QPixmap pixmap = icon.pixmap(QSize(mpIconLabel->size()));
        mpIconLabel->setPixmap(pixmap);
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
