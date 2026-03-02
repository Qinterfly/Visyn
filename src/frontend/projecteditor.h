#ifndef PROJECTEDITOR_H
#define PROJECTEDITOR_H

#include <QWidget>

#include "customlineedit.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QGroupBox)
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QSettings)

namespace Backend::Core
{
struct Project;
}

namespace Frontend
{

class ProjectEditor : public QWidget
{
    Q_OBJECT

public:
    ProjectEditor(QSettings& settings, Backend::Core::Project& project, QWidget* pParent = nullptr);
    virtual ~ProjectEditor() = default;

    QSize sizeHint() const;

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
    QGroupBox* createResponseGroupBox();
    QGroupBox* createSpectrumGroupBox();
    QGroupBox* createSolverGroupBox();

    // Dialogs
    void openResponseDialog();
    void openSpectrumDialog();

    // Slots
    void setOptions();

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
};
}

#endif // PROJECTEDITOR_H
