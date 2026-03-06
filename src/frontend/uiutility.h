
#ifndef UIUTILITY_H
#define UIUTILITY_H

#include <QFont>
#include <QtGlobal>

#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QTableWidgetItem);
QT_FORWARD_DECLARE_CLASS(QToolBar)
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QDir)
QT_FORWARD_DECLARE_CLASS(QIcon);
QT_FORWARD_DECLARE_CLASS(QSettings)

class QCPScatterStyle;

namespace Frontend::Utility
{

// Text
QFont getFont();
QFont getMonospaceFont();

// Ui
void setShortcutHints(QToolBar* pToolBar);
int getRepeatedIndex(int index, int size);

// File
void modifyFileSuffix(QString& pathFile, QString const& expectedSuffix);
QDir getLastDirectory(QSettings const& settings);
QString getLastPathFile(QSettings const& settings);
void setLastPathFile(QSettings& settings, QString const& pathFile);

// Widgets
void setIndexByKey(QComboBox* pComboBox, int key);
QTableWidgetItem* createTableItem(double value, Qt::AlignmentFlag alignment = Qt::AlignCenter);
QTableWidgetItem* createTableItem(std::vector<double> const& values, Qt::AlignmentFlag alignment = Qt::AlignCenter);
QTableWidgetItem* createTableItem(QString const& text, Qt::AlignmentFlag alignment = Qt::AlignCenter);
QDialog* showAsDialog(QWidget* pWidget, QString const& title = QString(), QWidget* pParent = nullptr, bool isModal = false);

// Icons
QIcon getIcon(QCPScatterStyle const& style, QSize const& size, bool isLine, bool isMarker);
}

#endif // UIUTILITY_H
