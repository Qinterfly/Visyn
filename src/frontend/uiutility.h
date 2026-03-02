
#ifndef UIUTILITY_H
#define UIUTILITY_H

#include <QFont>
#include <QtGlobal>

#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QTableWidgetItem);

namespace Frontend::Utility
{

// Text
QFont getFont();
QFont getMonospaceFont();

// Widgets
QTableWidgetItem* createTableItem(double value, Qt::AlignmentFlag alignment = Qt::AlignCenter);
QTableWidgetItem* createTableItem(std::vector<double> const& values, Qt::AlignmentFlag alignment = Qt::AlignCenter);
QTableWidgetItem* createTableItem(QString const& text, Qt::AlignmentFlag alignment = Qt::AlignCenter);
}

#endif // UIUTILITY_H
