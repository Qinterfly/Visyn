#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QTableWidgetItem>
#include <QToolBar>

#include "customplot.h"
#include "uiconstants.h"
#include "uiutility.h"

namespace Frontend::Utility
{

//! Get a default font
QFont getFont()
{
    QString fontName = "Roboto";
    uint fontSize = 12;
#ifdef Q_OS_WIN
    fontName = "Cambria";
    fontSize = 12;
#endif
    return QFont(fontName, fontSize);
}

//! Get a default monospace font
QFont getMonospaceFont()
{
    QString fontName = "RobotoMono";
    uint fontSize = 12;
#ifdef Q_OS_WIN
    fontName = "monofur";
    fontSize = 14;
#endif
    return QFont(fontName, fontSize);
}

//! Add shortcurt hints to all items contained in a tool bar
void setShortcutHints(QToolBar* pToolBar)
{
    QList<QAction*> actions = pToolBar->actions();
    int numActions = actions.size();
    for (int i = 0; i != numActions; ++i)
    {
        QAction* pAction = actions[i];
        QKeySequence shortcut = pAction->shortcut();
        if (shortcut.isEmpty())
            continue;
        pAction->setToolTip(QString("%1 (%2)").arg(pAction->toolTip(), shortcut.toString()));
    }
}

//! Helper function to get circular index
int getRepeatedIndex(int index, int size)
{
    if (index >= size)
        return index % size;
    return index;
}

//! Substitute a file suffix to the expected one, if necessary
void modifyFileSuffix(QString& pathFile, QString const& expectedSuffix)
{
    QFileInfo info(pathFile);
    QString currentSuffix = info.suffix();
    if (currentSuffix.isEmpty())
        pathFile.append(QString(".%1").arg(expectedSuffix));
    else if (currentSuffix != expectedSuffix)
        pathFile.replace(currentSuffix, expectedSuffix);
}

//! Retrieve last used directory
QDir getLastDirectory(QSettings const& settings)
{
    return QFileInfo(getLastPathFile(settings)).dir();
}

//! Retrieve last used path file
QString getLastPathFile(QSettings const& settings)
{
    return settings.value(Constants::Settings::skLastPathFile, QString()).toString();
}

//! Set last used path file
void setLastPathFile(QSettings& settings, QString const& pathFile)
{
    settings.setValue(Constants::Settings::skLastPathFile, pathFile);
}

//! Set combobox current index by item key
void setIndexByKey(QComboBox* pComboBox, int key)
{
    int numItems = pComboBox->count();
    pComboBox->setCurrentIndex(-1);
    for (int i = 0; i != numItems; ++i)
    {
        if (pComboBox->itemData(i).toInt() == key)
        {
            pComboBox->setCurrentIndex(i);
            break;
        }
    }
}

//! Create table widget item associated with a double value
QTableWidgetItem* createTableItem(double value, Qt::AlignmentFlag alignment)
{
    QString text = QString::number(value);
    QTableWidgetItem* pItem = new QTableWidgetItem(text);
    pItem->setTextAlignment(alignment);
    return pItem;
};

//! Create table widget item associated with double values
QTableWidgetItem* createTableItem(std::vector<double> const& values, Qt::AlignmentFlag alignment)
{
    QString text;
    QTextStream stream(&text);
    int numValues = values.size();
    for (int i = 0; i != numValues; ++i)
    {
        if (i > 0)
            stream << " ";
        stream << values[i];
    }
    return createTableItem(text, alignment);
}

//! Create table widget item associated with a string value
QTableWidgetItem* createTableItem(QString const& text, Qt::AlignmentFlag alignment)
{
    QTableWidgetItem* pItem = new QTableWidgetItem(text);
    pItem->setTextAlignment(alignment);
    return pItem;
}

//! Get icon for legend
QIcon getIcon(QCPScatterStyle const& style, QSize const& size, bool isLine, bool isMarker)
{
    // Construct the pixmap to be drawn at
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    // Create the painter
    QCPPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    style.applyTo(&painter, style.pen());

    // Draw the line style
    if (isLine)
    {
        QLineF line(0, size.height() / 2.0, size.width(), size.height() / 2);
        painter.drawLine(line);
    }

    // Draw the scatter style
    if (isMarker)
    {
        QRectF targetRect(0, 0, size.width(), size.height());
        style.drawShape(&painter, targetRect.center());
    }

    return QIcon(pixmap);
}
}
