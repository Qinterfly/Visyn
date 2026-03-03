#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>

#include "customlineedit.h"
#include "mathutility.h"
#include "project.h"
#include "projecteditor.h"
#include "uiutility.h"

using namespace Backend::Core;
using namespace Frontend;

enum ExportFormat
{
    kMatlab
};

ProjectEditor::ProjectEditor(QSettings& settings, Project& project, QWidget* pParent)
    : QWidget(pParent)
    , mSettings(settings)
    , mProject(project)
{
    setFont(Utility::getFont());
    createContent();
    createConnections();
    refresh();
}

QSize ProjectEditor::sizeHint() const
{
    return QSize(300, 1000);
}

//! Read responses from a file
void ProjectEditor::readResponses(QString const& pathFile)
{
    mpResponsePathEdit->clear();
    QList<Response> responses = ResponseIO::read(pathFile);
    if (!responses.isEmpty())
    {
        mProject.responses = responses;
        mProject.options.iSyncResponse = responses.size() - 1;
        mpResponsePathEdit->setText(pathFile);
        Utility::setLastPathFile(mSettings, pathFile);
        qInfo() << tr("Responses were successfully loaded from the file");
    }
    refresh();
    emit requestPlot();
}

//! Read spectrums from a file
void ProjectEditor::readSpectrums(QString const& pathFile)
{
    mpSpectrumPathEdit->clear();
    QList<Response> spectrums = ResponseIO::read(pathFile);
    if (!spectrums.isEmpty())
    {
        mProject.spectrums = spectrums;
        mProject.options.iSyncSpectrum = spectrums.size() - 1;
        mpSpectrumPathEdit->setText(pathFile);
        Utility::setLastPathFile(mSettings, pathFile);
        qInfo() << tr("Spectrums were successfully loaded from the file");
    }
    refresh();
    emit requestPlot();
}

//! Obtain the harmonic solution
void ProjectEditor::solve()
{
    HarmonicSolver solver(mProject.responses);
    solver.options = mProject.options;
    solver.refSpectrums = mProject.spectrums;
    mProject.solution = solver.solve();
    if (!mProject.solution.isEmpty())
        qInfo() << tr("Harmonic solver finished successfully");
    refresh();
    emit requestPlot();
}

//! Update the widget state
void ProjectEditor::refresh()
{
    // Clear the previous paths
    if (mProject.responses.isEmpty())
        mpResponsePathEdit->clear();
    if (mProject.spectrums.isEmpty())
        mpSpectrumPathEdit->clear();

    // Set the response combobox
    QSignalBlocker blockerResponse(mpResponseComboBox);
    mpResponseComboBox->clear();
    int numResponses = mProject.responses.size();
    for (int i = 0; i != numResponses; ++i)
        mpResponseComboBox->addItem(mProject.responses[i].props.name, i);
    Utility::setIndexByKey(mpResponseComboBox, mProject.options.iSyncResponse);

    // Set the spectrum combobox
    QSignalBlocker blockerSpectrum(mpSpectrumComboBox);
    mpSpectrumComboBox->clear();
    int numSpectrums = mProject.spectrums.size();
    for (int i = 0; i != numSpectrums; ++i)
        mpSpectrumComboBox->addItem(mProject.spectrums[i].props.name, i);
    Utility::setIndexByKey(mpSpectrumComboBox, mProject.options.iSyncSpectrum);

    // Set the options
    QSignalBlocker blockerSmoothFactor(mpSmoothFactorEdit);
    QSignalBlocker blockerNumIter(mpNumIterEdit);
    QSignalBlocker blockerNumAverages(mpNumAveragesEdit);
    QSignalBlocker blockerNumSkipPeriods(mpNumSkipPeriodsEdit);
    mpSmoothFactorEdit->setValue(mProject.options.smoothFactor);
    mpNumIterEdit->setValue(mProject.options.numIter);
    mpNumAveragesEdit->setValue(mProject.options.numAverages);
    mpNumSkipPeriodsEdit->setValue(mProject.options.numSkipPeriods);

    // Set the info
    setInfo();
}

//! Create all the widgets
void ProjectEditor::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(createResponseGroupBox());
    pLayout->addWidget(createSpectrumGroupBox());
    pLayout->addWidget(createSolverGroupBox());
    pLayout->addWidget(createInfoGroupBox());
    pLayout->addWidget(createExportGroupBox());
    pLayout->addStretch(1);
    setLayout(pLayout);
}

//! Specify the connections between widgets
void ProjectEditor::createConnections()
{
    // Options
    connect(mpSmoothFactorEdit, &Edit1d::valueChanged, this, &ProjectEditor::setOptions);
    connect(mpNumIterEdit, &Edit1i::valueChanged, this, &ProjectEditor::setOptions);
    connect(mpNumAveragesEdit, &Edit1i::valueChanged, this, &ProjectEditor::setOptions);
    connect(mpNumSkipPeriodsEdit, &Edit1i::valueChanged, this, &ProjectEditor::setOptions);
}

//! Create a widget to handle responses
QGroupBox* ProjectEditor::createResponseGroupBox()
{
    // Create the layout
    QHBoxLayout* pLayout = new QHBoxLayout;

    // Create the widgets
    mpResponsePathEdit = new QLineEdit;
    mpResponsePathEdit->setReadOnly(true);
    QPushButton* pOpenButton = new QPushButton(QIcon(":/icons/document-open.svg"), QString());
    connect(pOpenButton, &QPushButton::clicked, this, &ProjectEditor::openResponseDialog);

    // Combine the widgets
    pLayout->addWidget(new QLabel(tr("Path: ")));
    pLayout->addWidget(mpResponsePathEdit);
    pLayout->addWidget(pOpenButton);

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(tr("Time Responses"));
    pGroupBox->setLayout(pLayout);

    return pGroupBox;
}

//! Create a widget to handle spectrums
QGroupBox* ProjectEditor::createSpectrumGroupBox()
{
    // Create the layout
    QHBoxLayout* pLayout = new QHBoxLayout;

    // Create the widgets
    mpSpectrumPathEdit = new QLineEdit;
    mpSpectrumPathEdit->setReadOnly(true);
    QPushButton* pOpenButton = new QPushButton(QIcon(":/icons/document-open.svg"), QString());
    connect(pOpenButton, &QPushButton::clicked, this, &ProjectEditor::openSpectrumDialog);

    // Combine the widgets
    pLayout->addWidget(new QLabel(tr("Path: ")));
    pLayout->addWidget(mpSpectrumPathEdit);
    pLayout->addWidget(pOpenButton);

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(tr("Reference Spectrums"));
    pGroupBox->setLayout(pLayout);

    return pGroupBox;
}

//! Create a widget to handle solver
QGroupBox* ProjectEditor::createSolverGroupBox()
{
    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Create the sync widgets
    QGridLayout* pSyncLayout = new QGridLayout;
    mpResponseComboBox = new QComboBox;
    mpSpectrumComboBox = new QComboBox;
    mpResponseComboBox->setFont(font());
    mpSpectrumComboBox->setFont(font());
    mpResponseComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    mpSpectrumComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    pSyncLayout->addWidget(new QLabel(tr("Sync response: ")), 0, 0);
    pSyncLayout->addWidget(mpResponseComboBox, 0, 1);
    pSyncLayout->addWidget(new QLabel(tr("Reference spectrum: ")), 1, 0);
    pSyncLayout->addWidget(mpSpectrumComboBox, 1, 1);
    pSyncLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed), 1, 2);

    // Create the edit widgets
    QGridLayout* pEditLayout = new QGridLayout;
    mpSmoothFactorEdit = new Edit1d;
    mpNumIterEdit = new Edit1i;
    mpNumAveragesEdit = new Edit1i;
    mpNumSkipPeriodsEdit = new Edit1i;
    mpNumIterEdit->setMinimum(1);
    mpNumAveragesEdit->setMinimum(1);
    mpNumSkipPeriodsEdit->setMinimum(0);
    pEditLayout->addWidget(new QLabel(tr("Smooth factor: ")), 0, 0);
    pEditLayout->addWidget(mpSmoothFactorEdit, 0, 1);
    pEditLayout->addWidget(new QLabel(tr("Num. iterations: ")), 0, 2);
    pEditLayout->addWidget(mpNumIterEdit, 0, 3);
    pEditLayout->addWidget(new QLabel(tr("Num. averages: ")), 1, 0);
    pEditLayout->addWidget(mpNumAveragesEdit, 1, 1);
    pEditLayout->addWidget(new QLabel(tr("Num. skip periods: ")), 1, 2);
    pEditLayout->addWidget(mpNumSkipPeriodsEdit, 1, 3);
    pEditLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed), 1, 4);

    // Create the control widgets
    QHBoxLayout* pControlLayout = new QHBoxLayout;
    QPushButton* pSolveButton = new QPushButton(QIcon(":/icons/solve.svg"), tr("Solve"));
    connect(pSolveButton, &QPushButton::clicked, this, &ProjectEditor::solve);
    pControlLayout->addWidget(pSolveButton);
    pControlLayout->addStretch();

    // Combine all the widgets
    pMainLayout->addLayout(pSyncLayout);
    pMainLayout->addLayout(pEditLayout);
    pMainLayout->addLayout(pControlLayout);

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(tr("Solver"));
    pGroupBox->setLayout(pMainLayout);

    return pGroupBox;
}

//! Create widgets to display project information
QGroupBox* ProjectEditor::createInfoGroupBox()
{
    // Create the info widget
    mpInfoEdit = new QPlainTextEdit;
    mpInfoEdit->setFont(Utility::getMonospaceFont());
    mpInfoEdit->setReadOnly(true);

    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addWidget(mpInfoEdit);

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(tr("Project Information"));
    pGroupBox->setLayout(pMainLayout);

    return pGroupBox;
}

//! Create widgets to export project
QGroupBox* ProjectEditor::createExportGroupBox()
{
    // Create the widgets
    mpExportComboBox = new QComboBox;
    mpExportComboBox->addItem("Matlab", ExportFormat::kMatlab);

    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Create the format layout
    QHBoxLayout* pFormatLayout = new QHBoxLayout;
    pFormatLayout->addWidget(new QLabel(tr("Export format: ")));
    pFormatLayout->addWidget(mpExportComboBox);
    pFormatLayout->addStretch();

    // Create the control layout
    QHBoxLayout* pControlLayout = new QHBoxLayout;
    QPushButton* pExportButton = new QPushButton(QIcon(":/icons/document-export.svg"), tr("Export"));
    connect(pExportButton, &QPushButton::clicked, this, &ProjectEditor::exportDialog);
    pControlLayout->addWidget(pExportButton);
    pControlLayout->addStretch();

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(tr("Project Export"));
    pMainLayout->addLayout(pFormatLayout);
    pMainLayout->addLayout(pControlLayout);
    pGroupBox->setLayout(pMainLayout);

    return pGroupBox;
}

//! Create a file dialog for opening responses
void ProjectEditor::openResponseDialog()
{
    QString const kExpectedSuffix = "vaufx";
    QString pathFile = QFileDialog::getOpenFileName(this, tr("Read Responses"), Utility::getLastDirectory(mSettings).path(),
                                                    tr("Response file format (*%1)").arg(kExpectedSuffix));
    if (pathFile.isEmpty())
        return;

    // Read the responses
    readResponses(pathFile);
}

//! Create a file dialog for opening spectrums
void ProjectEditor::openSpectrumDialog()
{
    QString const kExpectedSuffix = "mat";
    QString pathFile = QFileDialog::getOpenFileName(this, tr("Read Spectrums"), Utility::getLastDirectory(mSettings).path(),
                                                    tr("Spectrum file format (*%1)").arg(kExpectedSuffix));
    if (pathFile.isEmpty())
        return;

    // Read the spectrums
    readSpectrums(pathFile);
}

//! Show project export dialog
void ProjectEditor::exportDialog()
{
    ExportFormat format = (ExportFormat) mpExportComboBox->currentData().toInt();
    if (format == ExportFormat::kMatlab)
    {
        QString const kExpectedSuffix = "mat";

        QString pathFile = QFileDialog::getSaveFileName(this, tr("Save Project"), Utility::getLastDirectory(mSettings).path(),
                                                        tr("Project file format (*%1)").arg(kExpectedSuffix));
        if (pathFile.isEmpty())
            return;

        // Modify the suffix, if necessary
        Utility::modifyFileSuffix(pathFile, kExpectedSuffix);

        // Save the project
        mProject.write(pathFile);
    }
}

//! Update options from the widgets
void ProjectEditor::setOptions()
{
    HarmonicOptions& options = mProject.options;
    options.iSyncResponse = mpResponseComboBox->currentData().toInt();
    options.iSyncSpectrum = mpSpectrumComboBox->currentData().toInt();
    options.smoothFactor = mpSmoothFactorEdit->value();
    options.numIter = mpNumIterEdit->value();
    options.numAverages = mpNumAveragesEdit->value();
    options.numSkipPeriods = mpNumSkipPeriodsEdit->value();
}

//! Display project information
void ProjectEditor::setInfo()
{
    // Clear the previous text
    mpInfoEdit->clear();
    if (mProject.responses.size() == 0)
        return;

    // Obtain the input statistics
    int numResponses = mProject.responses.size();
    int numSpectrums = mProject.spectrums.size();
    double sampleRate = numResponses > 0 ? mProject.responses.first().props.sampleRate : 0.0;

    // Display the input statistics
    mpInfoEdit->appendHtml(tr("<b>* Input data</b>"));
    mpInfoEdit->appendPlainText(tr("-> Number of time responses: %1").arg(numResponses));
    mpInfoEdit->appendPlainText(tr("-> Number of reference spectrums: %1").arg(numSpectrums));
    mpInfoEdit->appendPlainText(tr("-> Sample rate: %1 Hz").arg(sampleRate));

    // Process the solution
    if (!mProject.solution.isEmpty())
    {
        // Obtain the output statistics
        int numSegments = mProject.solution.segments.size();
        double minFreq = std::numeric_limits<double>::max();
        double maxFreq = std::numeric_limits<double>::lowest();
        double stepFreq = 0.0;
        for (int i = 0; i != numSegments; ++i)
        {
            Segment const& segment = mProject.solution.segments[i];
            minFreq = std::min(minFreq, segment.freq);
            maxFreq = std::max(maxFreq, segment.freq);
        }
        if (numSegments > 1)
        {
            VectorXd diffFreqs(numSegments - 1);
            for (int i = 0; i != numSegments - 1; ++i)
                diffFreqs[i] = std::abs(mProject.solution.segments[i + 1].freq - mProject.solution.segments[i].freq);
            stepFreq = Backend::Utility::median(diffFreqs);
        }

        // Display the output statistics
        mpInfoEdit->appendHtml(tr("<b>* Output data</b>"));
        mpInfoEdit->appendPlainText(tr("-> Number of segments: %1").arg(numSegments));
        mpInfoEdit->appendPlainText(tr("-> Frequency range: %1 - %2 Hz").arg(minFreq, 0, 'f', 2).arg(maxFreq, 0, 'f', 2));
        mpInfoEdit->appendPlainText(tr("-> Frequency step: %1 Hz").arg(stepFreq, 0, 'g', 3));
    }
}
