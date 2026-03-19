#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>

#include <testlab/api.h>

#include "customlineedit.h"
#include "customtable.h"
#include "mathutility.h"
#include "project.h"
#include "projecteditor.h"
#include "setupeditor.h"
#include "uiutility.h"

using namespace Backend::Core;
using namespace Frontend;

// From Testlab
Response convert(Testlab::IResponse* pResponse);
QList<Response> convert(std::vector<Testlab::IResponse*> const responses);
VectorXd convert(std::vector<double> const& data);
VectorXcd convert(std::vector<double> const& real, std::vector<double> const& imag);

// To Testlab
Testlab::IResponse* convert(Response const& response);
std::vector<Testlab::IResponse*> convert(QList<Response> const responses);
std::vector<double> convert(VectorXd const& data);

enum ExportFormat
{
    kMatlab,
    kTestlab
};

ProjectEditor::ProjectEditor(QSettings& settings, Project& project, QWidget* pParent)
    : QWidget(pParent)
    , mSettings(settings)
    , mProject(project)
    , mpTestlabProject(nullptr)
{
    setFont(Utility::getFont());
    createContent();
    createConnections();
    refresh();
}

ProjectEditor::~ProjectEditor()
{
    if (mpTestlabProject)
        delete mpTestlabProject;
}

QSize ProjectEditor::sizeHint() const
{
    return QSize(300, 1000);
}

SetupEditor* ProjectEditor::setupEditor()
{
    return mpSetupEditor;
}

//! Open a Testlab project
void ProjectEditor::openTestlab(QString const& pathFile)
{
    if (mpTestlabProject)
        delete mpTestlabProject;
    mpTestlabProject = nullptr;
    Testlab::IProject* pProject = Testlab::openProject(pathFile.toStdWString());
    if (pProject->isValid())
    {
        mpTestlabProject = pProject;
        mpTestlabPathEdit->setText(pathFile);
        setTestlabExportPath();
        Utility::setLastPathFile(mSettings, pathFile);
        qInfo() << tr("Testlab project is successfully opened");
    }
    else
    {
        delete pProject;
        qWarning() << tr("Could not connect to a Testlab project. Make sure that the license server is running");
    }
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
        mProject.options.iSyncSpectrum = mProject.spectrums.size() - 1;
        mpSpectrumPathEdit->setText(pathFile);
        mpTestlabExportPathEdit->clear();
        Utility::setLastPathFile(mSettings, pathFile);
        qInfo() << tr("Spectrums were successfully loaded from the file");
    }
    refresh();
    emit requestPlot();
}

//! Load selected spectrums from a Testlab project
void ProjectEditor::loadSpectrums()
{
    if (mpTestlabProject && mpTestlabProject->isValid())
    {
        auto spectrums = mpTestlabProject->getSelectedResponses();
        mProject.spectrums = convert(spectrums);
        if (mProject.spectrums.isEmpty())
        {
            qWarning() << tr("There are no selected responses in Testlab project");
        }
        else
        {
            mProject.options.iSyncSpectrum = mProject.spectrums.size() - 1;
            mpSpectrumPathEdit->setText(tr("Loaded from Testlab project"));
            setTestlabExportPath(mProject.spectrums);
            qInfo() << tr("Spectrums were successfully loaded from the Testlab project");
        }
        for (auto* p : spectrums)
            delete p;
        spectrums.clear();
    }
    else
    {
        qWarning() << tr("Testlab project is not opened. Could not load spectrums");
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

//! Add responses to the Testlab project
void ProjectEditor::addResponsesTestlab(QList<Response> const& responses, QString const& path)
{
    // Sanity check
    if (!mpTestlabProject)
    {
        qWarning() << tr("Testlab project is not opened");
        return;
    }
    if (!mpTestlabProject->isValid())
    {
        qWarning() << tr("Testlab project is not valid");
        return;
    }

    // Use the current setup to modify properties
    QList<Response> uResponses = responses;
    mpSetupEditor->replaceProperties(uResponses);

    // Add the responses
    auto cResponses = convert(uResponses);
    bool isSuccess = mpTestlabProject->addResponses(cResponses, path.toStdWString());
    if (isSuccess)
        qInfo() << tr("Responses were sucessfully saved at %1").arg(path);
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
    QSignalBlocker blockerMaxFreq(mpMaxFreqEdit);
    QSignalBlocker blockerLevelAmplitude(mpLevelAmplitudeEdit);
    mpSmoothFactorEdit->setValue(mProject.options.smoothFactor);
    mpNumIterEdit->setValue(mProject.options.numIter);
    mpNumAveragesEdit->setValue(mProject.options.numAverages);
    mpNumSkipPeriodsEdit->setValue(mProject.options.numSkipPeriods);
    mpMaxFreqEdit->setValue(mProject.options.maxFreq);
    mpLevelAmplitudeEdit->setValue(mProject.options.levelAmplitude);

    // Set the info
    setInfo();

    // Set the export
    ExportFormat exportFormat = (ExportFormat) mpExportTypeComboBox->currentData().toInt();
    mpTestlabExportWidget->setVisible(exportFormat == ExportFormat::kTestlab);
}

//! Create all the widgets
void ProjectEditor::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(createInputGroupBox());
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
    connect(mpResponseComboBox, &QComboBox::currentIndexChanged, this, &ProjectEditor::setOptions);
    connect(mpSpectrumComboBox, &QComboBox::currentIndexChanged, this, &ProjectEditor::setOptions);
    connect(mpSmoothFactorEdit, &Edit1d::valueChanged, this, &ProjectEditor::setOptions);
    connect(mpNumIterEdit, &Edit1i::valueChanged, this, &ProjectEditor::setOptions);
    connect(mpNumAveragesEdit, &Edit1i::valueChanged, this, &ProjectEditor::setOptions);
    connect(mpNumSkipPeriodsEdit, &Edit1i::valueChanged, this, &ProjectEditor::setOptions);
    connect(mpMaxFreqEdit, &Edit1d::valueChanged, this, &ProjectEditor::setOptions);
    connect(mpLevelAmplitudeEdit, &Edit1d::valueChanged, this, &ProjectEditor::setOptions);

    // Export
    connect(mpExportTypeComboBox, &QComboBox::currentIndexChanged, this, &ProjectEditor::refresh);
}

//! Create a widget to handle input data
QGroupBox* ProjectEditor::createInputGroupBox()
{
    // Create the layout
    QGridLayout* pLayout = new QGridLayout;

    // Create the widgets
    mpTestlabPathEdit = new Edit1s;
    mpResponsePathEdit = new Edit1s;
    mpSpectrumPathEdit = new Edit1s;
    QPushButton* pOpenTestlabButton = new QPushButton(QIcon(":/icons/document-open.svg"), QString());
    QPushButton* pReadResponseButton = new QPushButton(QIcon(":/icons/document-open.svg"), QString());
    QPushButton* pReadSpectrumButton = new QPushButton(QIcon(":/icons/document-open.svg"), QString());
    QPushButton* pLoadSpectrumButton = new QPushButton(QIcon(":/icons/select.png"), QString());

    // Initialize the widgets
    mpTestlabPathEdit->setReadOnly(true);
    mpResponsePathEdit->setReadOnly(true);
    mpSpectrumPathEdit->setReadOnly(true);

    // Set the connections
    connect(pOpenTestlabButton, &QPushButton::clicked, this, &ProjectEditor::testlabFileDialog);
    connect(pReadResponseButton, &QPushButton::clicked, this, &ProjectEditor::responseFileDialog);
    connect(pReadSpectrumButton, &QPushButton::clicked, this, &ProjectEditor::spectrumFileDialog);
    connect(pLoadSpectrumButton, &QPushButton::clicked, this, &ProjectEditor::loadSpectrums);

    // Combine the widgets
    pLayout->addWidget(new QLabel(tr("Testlab project: ")), 0, 0);
    pLayout->addWidget(mpTestlabPathEdit, 0, 1, 1, 2);
    pLayout->addWidget(pOpenTestlabButton, 0, 3);
    pLayout->addWidget(new QLabel(tr("Time responses: ")), 1, 0);
    pLayout->addWidget(mpResponsePathEdit, 1, 1, 1, 2);
    pLayout->addWidget(pReadResponseButton, 1, 3);
    pLayout->addWidget(new QLabel(tr("Reference spectrums: ")), 2, 0);
    pLayout->addWidget(mpSpectrumPathEdit, 2, 1);
    pLayout->addWidget(pLoadSpectrumButton, 2, 2);
    pLayout->addWidget(pReadSpectrumButton, 2, 3);

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(tr("Input Data"));
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
    mpMaxFreqEdit = new Edit1d;
    mpLevelAmplitudeEdit = new Edit1d;

    // Initialize the widgets
    mpNumIterEdit->setMinimum(1);
    mpNumAveragesEdit->setMinimum(1);
    mpNumSkipPeriodsEdit->setMinimum(0);
    mpMaxFreqEdit->setValue(0.0);
    mpLevelAmplitudeEdit->setMinimum(0.0);
    mpLevelAmplitudeEdit->setMaximum(1.0);

    // Combine the widgets
    pEditLayout->addWidget(new QLabel(tr("Smooth factor: ")), 0, 0);
    pEditLayout->addWidget(mpSmoothFactorEdit, 0, 1);
    pEditLayout->addWidget(new QLabel(tr("Num. iterations: ")), 0, 2);
    pEditLayout->addWidget(mpNumIterEdit, 0, 3);
    pEditLayout->addWidget(new QLabel(tr("Num. averages: ")), 1, 0);
    pEditLayout->addWidget(mpNumAveragesEdit, 1, 1);
    pEditLayout->addWidget(new QLabel(tr("Num. skip periods: ")), 1, 2);
    pEditLayout->addWidget(mpNumSkipPeriodsEdit, 1, 3);
    pEditLayout->addWidget(new QLabel(tr("Max frequecy: ")), 2, 0);
    pEditLayout->addWidget(mpMaxFreqEdit, 2, 1);
    pEditLayout->addWidget(new QLabel(tr("Level amplitude: ")), 2, 2);
    pEditLayout->addWidget(mpLevelAmplitudeEdit, 2, 3);
    pEditLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 4);

    // Create the control widgets
    QHBoxLayout* pControlLayout = new QHBoxLayout;
    QPushButton* pSolveButton = new QPushButton(QIcon(":/icons/solve.svg"), tr("Solve"));
    QPushButton* pIntervalButton = new QPushButton(QIcon(":/icons/interval.svg"), tr("Intervals"));
    connect(pSolveButton, &QPushButton::clicked, this, &ProjectEditor::solve);
    connect(pIntervalButton, &QPushButton::clicked, this, &ProjectEditor::showIntervalEditor);
    pControlLayout->addWidget(pSolveButton);
    pControlLayout->addWidget(pIntervalButton);
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
    mpExportTypeComboBox = new QComboBox;
    mpExportTypeComboBox->addItem("Matlab", ExportFormat::kMatlab);
    mpExportTypeComboBox->addItem("Testlab", ExportFormat::kTestlab);
    Utility::setIndexByKey(mpExportTypeComboBox, ExportFormat::kTestlab);
    mpSetupEditor = new SetupEditor;

    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Create the format layout
    QHBoxLayout* pFormatLayout = new QHBoxLayout;
    pFormatLayout->addWidget(new QLabel(tr("Export format: ")));
    pFormatLayout->addWidget(mpExportTypeComboBox);
    pFormatLayout->addStretch();

    // Create the Testlab layout
    QHBoxLayout* pTestlabLayout = new QHBoxLayout;
    mpTestlabExportPathEdit = new Edit1s;
    pTestlabLayout->addWidget(new QLabel(tr("Project path: ")));
    pTestlabLayout->addWidget(mpTestlabExportPathEdit);
    pTestlabLayout->setContentsMargins(0, 0, 0, 0);
    mpTestlabExportWidget = new QWidget;
    mpTestlabExportWidget->setLayout(pTestlabLayout);

    // Create the control layout
    QHBoxLayout* pControlLayout = new QHBoxLayout;
    QPushButton* pExportButton = new QPushButton(QIcon(":/icons/document-export.svg"), tr("Export"));
    QPushButton* pSetupButton = new QPushButton(QIcon(":/icons/setup.svg"), tr("Setup"));
    connect(pExportButton, &QPushButton::clicked, this, &ProjectEditor::exportDialog);
    connect(pSetupButton, &QPushButton::clicked, mpSetupEditor, &SetupEditor::show);
    pControlLayout->addWidget(pExportButton);
    pControlLayout->addWidget(pSetupButton);
    pControlLayout->addStretch();

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(tr("Project Export"));
    pMainLayout->addLayout(pFormatLayout);
    pMainLayout->addWidget(mpTestlabExportWidget);
    pMainLayout->addLayout(pControlLayout);
    pGroupBox->setLayout(pMainLayout);

    return pGroupBox;
}

//! Create a file dialog for opening a Testlab project
void ProjectEditor::testlabFileDialog()
{
    QString pathFile = QFileDialog::getOpenFileName(this, tr("Open Testlab Project"), Utility::getLastDirectory(mSettings).path(),
                                                    tr("Testlab file format (*.lms)"));
    if (pathFile.isEmpty())
        return;
    openTestlab(pathFile);
}

//! Create a file dialog for opening responses
void ProjectEditor::responseFileDialog()
{
    QString pathFile = QFileDialog::getOpenFileName(this, tr("Read Responses"), Utility::getLastDirectory(mSettings).path(),
                                                    tr("Response file format (*.vaufx *.mat)"));
    if (pathFile.isEmpty())
        return;

    // Read the responses
    readResponses(pathFile);
}

//! Create a file dialog for opening spectrums
void ProjectEditor::spectrumFileDialog()
{
    QString pathFile = QFileDialog::getOpenFileName(this, tr("Read Spectrums"), Utility::getLastDirectory(mSettings).path(),
                                                    tr("Spectrum file format (*.mat)"));
    if (pathFile.isEmpty())
        return;

    // Read the spectrums
    readSpectrums(pathFile);
}

//! Show project export dialog
void ProjectEditor::exportDialog()
{
    ExportFormat format = (ExportFormat) mpExportTypeComboBox->currentData().toInt();
    switch (format)
    {
    case kMatlab:
    {
        QString const kExpectedSuffix = "mat";

        QString pathFile = QFileDialog::getSaveFileName(this, tr("Save Project"), Utility::getLastDirectory(mSettings).path(),
                                                        tr("Project file format (*%1)").arg(kExpectedSuffix));
        if (pathFile.isEmpty())
            return;

        // Modify the suffix, if necessary
        Utility::modifyFileSuffix(pathFile, kExpectedSuffix);

        // Save the project
        if (mProject.write(pathFile))
            qInfo() << tr("The project was saved as the following file %1").arg(pathFile);
        break;
    }
    case kTestlab:
    {
        QString path = mpTestlabExportPathEdit->text();
        addResponsesTestlab(mProject.solution.spectrums, path);
        break;
    }
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
    options.maxFreq = mpMaxFreqEdit->value();
    options.levelAmplitude = mpLevelAmplitudeEdit->value();
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

//! Show the editor of intervals
void ProjectEditor::showIntervalEditor()
{
    IntervalEditor* pEditor = new IntervalEditor(mProject.options.intervals);
    Utility::showAsDialog(pEditor, tr("Interval Editor"), this);
    connect(pEditor, &IntervalEditor::edited, this,
            [this]()
            {
                refresh();
                emit requestPlot();
            });
}

//! Predefine the Testlab export path
void ProjectEditor::setTestlabExportPath(QList<Response> const& responses)
{
    // Constants
    QChar const kDelimiter = '/';

    // Sanity check
    if (!mpTestlabProject)
        return;
    if (!mpTestlabProject->isValid())
        return;

    // Build up the path using the spectrums, if it is not set
    QString path;
    if (!responses.isEmpty())
    {
        int numResponses = responses.size();
        for (int i = 0; i != numResponses; ++i)
        {
            QString const& pathResponse = responses[i].props.path;
            if (pathResponse.isEmpty())
                continue;
            path = pathResponse;
            // Remove the response name
            int iLast = path.length();
            if (path.endsWith(kDelimiter))
                iLast = path.lastIndexOf(kDelimiter) - 1;
            int iSplit = path.lastIndexOf(kDelimiter, iLast);
            path = path.first(iSplit);
            break;
        }
    }

    // Use the active section, if the path is still empty
    if (path.isEmpty())
        path = QString::fromStdWString(mpTestlabProject->getActiveSection());

    // Add the delimiter at the end
    if (!path.endsWith(kDelimiter))
        path += kDelimiter;

    // Set the edit value
    mpTestlabExportPathEdit->setText(path);
}

IntervalEditor::IntervalEditor(QList<PairDouble>& intervals, QWidget* pParent)
    : QWidget(pParent)
    , mIntervals(intervals)
{
    createContent();
    createConnections();
    refresh();
};

QSize IntervalEditor::sizeHint() const
{
    return QSize(300, 400);
}

//! Update the state of the widgets
void IntervalEditor::refresh()
{
    // Block the signals
    QSignalBlocker blockerCount(mpCountEdit);
    QSignalBlocker blockerDataTable(mpDataTable);

    // Set the number of intervals
    int count = mIntervals.size();
    mpCountEdit->setValue(count);

    // Set the interval data
    mpDataTable->clear();
    mpDataTable->setRowCount(count);
    mpDataTable->setColumnCount(2);
    mpDataTable->setHorizontalHeaderLabels({tr("Start, s"), tr("End, s")});
    for (int i = 0; i != count; ++i)
    {
        PairDouble const& interval = mIntervals[i];
        mpDataTable->setItem(i, 0, Utility::createTableItem(interval.first));
        mpDataTable->setItem(i, 1, Utility::createTableItem(interval.second));
    }
}

//! Create all the widgets
void IntervalEditor::createContent()
{
    // Create widgets
    mpCountEdit = new Edit1i;
    mpDataTable = new CustomTable;

    // Initialize widgets
    mpCountEdit->setMinimum(0);
    mpDataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Combine the widgets
    QHBoxLayout* pControlLayout = new QHBoxLayout;
    pControlLayout->addWidget(new QLabel(tr("Number of intervals: ")));
    pControlLayout->addWidget(mpCountEdit);
    pControlLayout->addStretch();

    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pControlLayout);
    pMainLayout->addWidget(mpDataTable);
    setLayout(pMainLayout);
}

//! Specify connections between widgets
void IntervalEditor::createConnections()
{
    connect(mpCountEdit, &Edit1i::valueChanged, this, &IntervalEditor::setCount);
    connect(mpDataTable, &CustomTable::itemChanged, this, &IntervalEditor::setData);
}

//! Set number of intervals
void IntervalEditor::setCount()
{
    mIntervals.resize(mpCountEdit->value(), {0, -1});
    refresh();
    emit edited();
}

//! Set interval data
void IntervalEditor::setData()
{
    int numIntervals = mpDataTable->rowCount();
    mIntervals.resize(numIntervals);
    for (int i = 0; i != numIntervals; ++i)
    {
        double start = mpDataTable->item(i, 0)->text().toDouble();
        double end = mpDataTable->item(i, 1)->text().toDouble();
        if (end > 0.0 && start > end)
            std::swap(start, end);
        mIntervals[i] = {start, end};
    }
    refresh();
    emit edited();
}

//! Helper function to convert a Testlab response
Response convert(Testlab::IResponse* pResponse)
{
    Response result;

    // Set data
    result.setKeys(convert(pResponse->keys));
    if (pResponse->imagValues.size() > 0)
        result.setValues(convert(pResponse->realValues, pResponse->imagValues));
    else
        result.setValues(convert(pResponse->realValues));

    // Set properties
    QString direction = QString::fromStdWString(pResponse->direction);
    QString dimension = QString::fromStdWString(pResponse->dimension);
    ResponseProperties& props = result.props;
    props.id = pResponse->channel;
    props.direction = getDirection(direction);
    props.domain = result.isComplex() ? Domain::kFreq : Domain::kTime;
    props.dimension = dimension == "Accel" ? Dimension::kAccel : Dimension::kNone;
    props.sign = pResponse->sign;
    props.sampleRate = 0;
    props.path = QString::fromStdWString(pResponse->path);
    props.name = QString::fromStdWString(pResponse->name);
    props.node = QString::fromStdWString(pResponse->node);
    props.numAverages = pResponse->numAverages;
    props.transducer = QString::fromStdWString(pResponse->transducer);
    props.comment = QString::fromStdWString(pResponse->comment);

    return result;
}

//! Helper function to convert Testlab responses
QList<Response> convert(std::vector<Testlab::IResponse*> const responses)
{
    int numResponses = responses.size();
    QList<Response> result(numResponses);
    for (int i = 0; i != numResponses; ++i)
        result[i] = convert(responses[i]);
    return result;
}

//! Helper function to convert real array to Eigen vector
VectorXd convert(std::vector<double> const& data)
{
    int numData = data.size();
    VectorXd result(numData);
    for (int i = 0; i != numData; ++i)
        result[i] = data[i];
    return result;
}

//! Helper function to convert complex array to Eigen vector
VectorXcd convert(std::vector<double> const& real, std::vector<double> const& imag)
{
    if (real.size() != imag.size())
        return {};
    int numData = real.size();
    VectorXcd result(numData);
    for (int i = 0; i != numData; ++i)
        result[i] = {real[i], imag[i]};
    return result;
}

//! Convert response from the interop to custom format
Testlab::IResponse* convert(Response const& response)
{
    Testlab::IResponse* pResult = new Testlab::IResponse;

    // Set data
    pResult->keys = convert(response.keys());
    if (response.isComplex())
    {
        pResult->realValues = convert(response.real());
        pResult->imagValues = convert(response.imag());
    }
    else
    {
        pResult->realValues = convert(response.realValues());
    }

    // Set properties
    ResponseProperties const& props = response.props;
    pResult->channel = props.id;
    pResult->direction = getLabel(props.direction).toStdWString();
    if (props.dimension == Dimension::kAccel)
        pResult->dimension = QString("Accel").toStdWString();
    pResult->sign = props.sign;
    pResult->path = props.path.toStdWString();
    pResult->name = props.name.toStdWString();
    pResult->node = props.node.toStdWString();
    pResult->component = props.component.toStdWString();
    pResult->numAverages = props.numAverages;
    pResult->transducer = props.transducer.toStdWString();
    pResult->comment = props.comment.toStdWString();

    return pResult;
}

//! Convert responses for interoperability
std::vector<Testlab::IResponse*> convert(QList<Response> const responses)
{
    int numResponses = responses.size();
    std::vector<Testlab::IResponse*> result(numResponses);
    for (int i = 0; i != numResponses; ++i)
        result[i] = convert(responses[i]);
    return result;
}

//! Convert Eigen double vector to the standard one
std::vector<double> convert(VectorXd const& data)
{
    return std::vector<double>(data.data(), data.data() + data.size());
}
