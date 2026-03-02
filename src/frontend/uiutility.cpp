#include <QTableWidgetItem>

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
}
