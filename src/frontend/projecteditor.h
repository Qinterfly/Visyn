#ifndef PROJECTEDITOR_H
#define PROJECTEDITOR_H

#include <QWidget>

#include "aliasdata.h"
#include "customlineedit.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QGroupBox)
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(QPlainTextEdit)

namespace Backend::Core
{
struct Project;
}

namespace Frontend
{

class CustomTable;

class ProjectEditor : public QWidget
{
    Q_OBJECT

public:
    ProjectEditor(QSettings& settings, Backend::Core::Project& project, QWidget* pParent = nullptr);
    virtual ~ProjectEditor() = default;

    QSize sizeHint() const override;

    void readResponses(QString const& pathFile);
    void readSpectrums(QString const& pathFile);
    void solve();
    void refresh();

signals:
    void edited();
    void requestPlot();

private:
    // Content
    void createContent();
    void createConnections();
    QGroupBox* createInputGroupBox();
    QGroupBox* createSolverGroupBox();
    QGroupBox* createInfoGroupBox();
    QGroupBox* createExportGroupBox();

    // Dialogs
    void openResponseDialog();
    void openSpectrumDialog();
    void exportDialog();

    // Slots
    void setOptions();
    void setInfo();
    void showIntervalEditor();

private:
    QSettings& mSettings;
    Backend::Core::Project& mProject;

    // Input
    Edit1s* mpResponsePathEdit;
    Edit1s* mpSpectrumPathEdit;

    // Solver
    QComboBox* mpResponseComboBox;
    QComboBox* mpSpectrumComboBox;
    Edit1d* mpSmoothFactorEdit;
    Edit1i* mpNumIterEdit;
    Edit1i* mpNumAveragesEdit;
    Edit1i* mpNumSkipPeriodsEdit;
    Edit1d* mpMaxFreqEdit;
    Edit1d* mpLevelAmplitudeEdit;
    QPlainTextEdit* mpInfoEdit;
    QComboBox* mpExportComboBox;
};

class IntervalEditor : public QWidget
{
    Q_OBJECT

public:
    IntervalEditor(QList<Backend::Core::PairDouble>& intervals, QWidget* pParent = nullptr);
    virtual ~IntervalEditor() = default;

    QSize sizeHint() const override;

    void refresh();

signals:
    void edited();

private:
    // Widgets
    void createContent();
    void createConnections();

    // Slots
    void setCount();
    void setData();

private:
    QList<Backend::Core::PairDouble>& mIntervals;

    // Widgets
    Edit1i* mpCountEdit;
    CustomTable* mpDataTable;
};
}

#endif // PROJECTEDITOR_H
