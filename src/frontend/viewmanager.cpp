#include <QVBoxLayout>

#include <magic_enum/magic_enum.hpp>

#include "customplot.h"
#include "customtabwidget.h"
#include "project.h"
#include "uiconstants.h"
#include "uiutility.h"
#include "viewmanager.h"

using namespace Backend::Core;
using namespace Frontend;

// Helper functions
QVector<double> convert(VectorXd const& vec);
QCPCurve* createCurve(CustomPlot* pPlot, VectorXd const& xData, VectorXd const& yData, QColor const& color, QString const& name = QString(),
                      double lineWidth = 1.0);
QCPGraph* createGraph(CustomPlot* pPlot, VectorXd const& xData, VectorXd const& yData, QColor const& color, QString const& name = QString(),
                      double lineWidth = 1.0);

ViewManager::ViewManager(Project const& project, QWidget* pParent)
    : QWidget(pParent)
    , mProject(project)
{
    createContent();
}

ViewManager::~ViewManager()
{
    clear();
}

QSize ViewManager::sizeHint() const
{
    return QSize(800, 1000);
}

//! Destroy all views
void ViewManager::clear()
{
    mpTabWidget->removeAllPages();
}

//! Replot all the views
void ViewManager::plot()
{
    mpResponseView->plot();
    mpRefSpectrumView->plot();
    mpSegmentView->plot();
    mpResSpectrumView->plot();
}

//! Create all the widgets and corresponding actions
void ViewManager::createContent()
{
    // Create the tab widget
    mpTabWidget = new CustomTabWidget;
    mpTabWidget->setTabsClosable(false);
    mpTabWidget->setTabsRenamable(false);

    // Create the view options
    auto timeOptions = ResponseView::Options(ResponseView::kReal, ResponseView::kNone);
    auto freqOptions = ResponseView::Options(ResponseView::kImag, ResponseView::kReal);

    // Create the views
    mpResponseView = new ResponseView(mProject.responses, timeOptions);
    mpRefSpectrumView = new ResponseView(mProject.spectrums, freqOptions);
    mpSegmentView = new SegmentView(mProject.solution);
    mpResSpectrumView = new ResponseView(mProject.solution.spectrums, freqOptions);

    // Add the views
    mpTabWidget->addTab(mpResponseView, QIcon(), tr("Time Responses"));
    mpTabWidget->addTab(mpRefSpectrumView, QIcon(), tr("Reference Spectrums"));
    mpTabWidget->addTab(mpSegmentView, QIcon(), tr("Segments"));
    mpTabWidget->addTab(mpResSpectrumView, QIcon(), tr("Resultant Spectrums"));

    // Insert the widgets into the main layout
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->addWidget(mpTabWidget);
    setLayout(pLayout);
}

ResponseView::Options::Options()
{
    upType = kReal;
    downType = kNone;
    colors = Constants::Colors::skStandardColors;
    indicesSelected.resize(1);
    std::iota(indicesSelected.begin(), indicesSelected.end(), 0);
}

ResponseView::Options::Options(Type uUpType, Type uDownType)
    : Options()
{
    upType = uUpType;
    downType = uDownType;
}

ResponseView::ResponseView(QList<Response> const& responses, Options const& options)
    : mResponses(responses)
    , mOptions(options)
{
    setFont(Utility::getFont());
    createContent();
    createConnections();
    refresh();
}

void ResponseView::clear()
{
    mpUpPlot->clear();
    mpDownPlot->clear();
    mMaskSelected.clear();
}

void ResponseView::refresh()
{
    // Constants
    QMap<Type, QString> typeNames;
    typeNames[kNone] = QString();
    typeNames[kReal] = tr("Real");
    typeNames[kImag] = tr("Imag");
    typeNames[kAmplitude] = tr("Amplitude");
    typeNames[kPhase] = tr("Phase");

    // Build up the selection mask
    int numSelected = mOptions.indicesSelected.size();
    int numResponses = mResponses.size();
    mMaskSelected.resize(numResponses, false);
    for (int i = 0; i != numSelected; ++i)
    {
        int iSelect = mOptions.indicesSelected[i];
        if (iSelect >= 0 && iSelect < numResponses)
            mMaskSelected[iSelect] = true;
    }

    // Block the comboboxes
    QSignalBlocker blockerUpComboBox(mpUpComboBox);
    QSignalBlocker blockerDownComboBox(mpDownComboBox);

    // Insert the types
    mpUpComboBox->clear();
    mpDownComboBox->clear();
    auto types = magic_enum::enum_values<Type>();
    for (auto type : types)
    {
        QString name = typeNames.contains(type) ? typeNames[type] : magic_enum::enum_name(type).data();
        mpUpComboBox->addItem(name, type);
        mpDownComboBox->addItem(name, type);
    }

    // Select the types
    Utility::setIndexByKey(mpUpComboBox, mOptions.upType);
    Utility::setIndexByKey(mpDownComboBox, mOptions.downType);

    // Insert the responses
    QSignalBlocker blockerSelectList(mpSelectList);
    mpSelectList->clear();
    for (int iResponse = 0; iResponse != numResponses; ++iResponse)
    {
        QListWidgetItem* pItem = new QListWidgetItem(mResponses[iResponse].props.name);

        // Get icon size
        QFontMetrics fontMetrics(pItem->font());
        QRect textRect = fontMetrics.boundingRect(pItem->text());
        QSize iconSize = QSize(textRect.height(), textRect.height());

        // Set the icon
        int iColor = Utility::getRepeatedIndex(iResponse, mOptions.colors.size());
        QCPScatterStyle style(QCPScatterStyle::ssNone, mOptions.colors[iColor], 1.0);
        QIcon icon = Utility::getIcon(style, iconSize, true, false);
        pItem->setIcon(icon);

        // Add item
        mpSelectList->addItem(pItem);

        // Select the item
        if (mMaskSelected[iResponse])
            mpSelectList->setCurrentItem(pItem, QItemSelectionModel::Select);
    }
}

void ResponseView::plot()
{
    clear();
    refresh();
    draw(mOptions.upType, mpUpPlot);
    draw(mOptions.downType, mpDownPlot);
}

void ResponseView::draw(Type type, CustomPlot* pPlot)
{
    // Set the visibility
    pPlot->setVisible(type != kNone);

    // Do not render invisible plots
    if (type == kNone)
        return;

    // Process all the responses
    QList<QColor> const colors = Constants::Colors::skStandardColors;
    int numResponses = mResponses.size();
    int numColors = colors.size();
    for (int iResponse = 0; iResponse != numResponses; ++iResponse)
    {
        // Check if the response is selected for plotting
        if (!mMaskSelected[iResponse])
            continue;
        Response const& response = mResponses[iResponse];
        if (response.isEmpty())
            continue;

        // Get the values
        VectorXd values;
        switch (type)
        {
        case kReal:
            values = response.real();
            break;
        case kImag:
            values = response.imag();
            break;
        case kAmplitude:
            values = response.amplitudes();
            break;
        case kPhase:
            values = response.phases();
            break;
        default:
            break;
        }
        int numValues = values.size();

        // Get the keys
        VectorXd keys = response.keys();
        if (numValues > 0 && keys.size() != numValues)
        {
            double timeStep = 1.0;
            if (response.props.sampleRate > std::numeric_limits<double>::epsilon())
                timeStep = 1.0 / response.props.sampleRate;
            keys = VectorXd::LinSpaced(numValues, 0.0, (numValues - 1) * timeStep);
        }

        // Create the graph
        int iColor = Utility::getRepeatedIndex(iResponse, numColors);
        createGraph(pPlot, keys, values, colors[iColor], response.props.name);
    }

    // Replot
    pPlot->rescaleAxes();
    pPlot->replot();
}

void ResponseView::setOptions()
{
    Type upType = (Type) mpUpComboBox->currentIndex();
    Type downType = (Type) mpDownComboBox->currentIndex();
    if (upType != mOptions.upType && upType != kNone && downType != kNone)
    {
        switch (upType)
        {
        case kReal:
            downType = kImag;
            break;
        case kImag:
            downType = kReal;
            break;
        case kAmplitude:
            downType = kPhase;
            break;
        case kPhase:
            downType = kAmplitude;
            break;
        default:
            break;
        }
    }
    mOptions.upType = upType;
    mOptions.downType = downType;

    plot();
}

//! Process changing selected responses
void ResponseView::processSelected()
{
    // Save the selected indices
    auto selectedItems = mpSelectList->selectedItems();
    int numSelected = selectedItems.size();
    mOptions.indicesSelected.resize(numSelected);
    for (int i = 0; i != numSelected; ++i)
        mOptions.indicesSelected[i] = mpSelectList->row(selectedItems[i]);

    // Refresh the plots
    plot();
}

//! Create all the widgets
void ResponseView::createContent()
{
    // Create the widgets
    mpUpComboBox = new QComboBox;
    mpDownComboBox = new QComboBox;
    mpUpPlot = new CustomPlot;
    mpDownPlot = new CustomPlot;
    mpSelectList = new QListWidget;

    // Initialize the widgets
    mpUpComboBox->setFont(font());
    mpDownComboBox->setFont(font());
    mpSelectList->setFont(font());
    mpSelectList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mpSelectList->setContentsMargins(0, 0, 0, 0);
    mpSelectList->setResizeMode(QListWidget::Adjust);
    mpSelectList->setSizeAdjustPolicy(QListWidget::AdjustToContentsOnFirstShow);

    // Create the control layout
    QHBoxLayout* pControlLayout = new QHBoxLayout;
    pControlLayout->addWidget(new QLabel(tr("Top: ")));
    pControlLayout->addWidget(mpUpComboBox);
    pControlLayout->addWidget(new QLabel(tr("Bottom: ")));
    pControlLayout->addWidget(mpDownComboBox);
    pControlLayout->addStretch();

    // Create the plot layout
    QWidget* pPlotWidget = new QWidget;
    QVBoxLayout* pPlotLayout = new QVBoxLayout;
    pPlotLayout->addLayout(pControlLayout);
    pPlotLayout->addWidget(mpUpPlot);
    pPlotLayout->addWidget(mpDownPlot);
    pPlotWidget->setLayout(pPlotLayout);

    // Create the splitter
    QSplitter* pSplitter = new QSplitter(Qt::Horizontal);
    pSplitter->addWidget(pPlotWidget);
    pSplitter->addWidget(mpSelectList);
    pSplitter->setStretchFactor(0, 2);
    pSplitter->setStretchFactor(1, 1);

    // Set the main layout
    QHBoxLayout* pMainLayout = new QHBoxLayout;
    pMainLayout->addWidget(pSplitter);
    setLayout(pMainLayout);
}

//! Set the connections between the widgets
void ResponseView::createConnections()
{
    connect(mpUpComboBox, &QComboBox::currentIndexChanged, this, &ResponseView::setOptions);
    connect(mpDownComboBox, &QComboBox::currentIndexChanged, this, &ResponseView::setOptions);
    connect(mpSelectList, &QListWidget::itemSelectionChanged, this, &ResponseView::processSelected);
}

SegmentView::SegmentView(HarmonicSolution const& solution, QWidget* pParent)
    : QWidget(pParent)
    , mSolution(solution)
{
    mpPlot = new CustomPlot;
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(mpPlot);
    setLayout(pLayout);
}

//! Remove previously plotted data
void SegmentView::clear()
{
    mpPlot->clear();
}

//! Plot segments
void SegmentView::plot()
{
    // Clear the previous data
    clear();

    // Slice data
    VectorXd const& xFreqs = mSolution.freqs.keys();
    VectorXd const& yFreqs = mSolution.freqs.realValues();
    VectorXd const& xFilterFreqs = mSolution.filterFreqs.keys();
    VectorXd const& yFilterFreqs = mSolution.filterFreqs.realValues();

    // Plot the frequencies
    createGraph(mpPlot, xFreqs, yFreqs, QColor("blue"), tr("Freqs"), 1.0);
    createGraph(mpPlot, xFilterFreqs, yFilterFreqs, QColor("red"), tr("Filter freqs"), 2.0);

    // Get the boundary frequencies
    double minFreq = 0.0;
    double maxFreq = 0.0;
    if (yFreqs.size() > 0)
    {
        minFreq = yFreqs.minCoeff();
        maxFreq = yFreqs.maxCoeff();
    }

    // Plot the segments
    int numSegments = mSolution.segments.size();
    for (int i = 0; i != numSegments; ++i)
    {
        Segment const& segment = mSolution.segments[i];
        QCPCurve* pCurve = new QCPCurve(mpPlot->xAxis, mpPlot->yAxis);
        pCurve->setData({segment.keys.second, segment.keys.second}, {minFreq, maxFreq});
        pCurve->setPen(QPen(QColor("black"), 0.5, Qt::DashLine));
    }

    // Set the labels
    mpPlot->xAxis->setLabel(tr("Time, s"));
    mpPlot->yAxis->setLabel(tr("Frequency, Hz"));

    // Replot
    mpPlot->rescaleAxes();
    mpPlot->replot();
}

//! Helper function to convert Eigen vector consisted of double values
QVector<double> convert(VectorXd const& vec)
{
    return QVector<double>(vec.data(), vec.data() + vec.rows() * vec.cols());
}

//! Helper function to create a curve associated with a custom plot
QCPCurve* createCurve(CustomPlot* pPlot, VectorXd const& xData, VectorXd const& yData, QColor const& color, QString const& name, double lineWidth)
{
    if (xData.size() == 0 || xData.size() != yData.size())
        return nullptr;
    QCPCurve* pCurve = new QCPCurve(pPlot->xAxis, pPlot->yAxis);
    pCurve->setData(convert(xData), convert(yData));
    pCurve->setSelectable(QCP::SelectionType::stSingleData);
    pCurve->setName(name);
    pCurve->setPen(QPen(color, lineWidth));
    return pCurve;
}

//! Helper function to create a graph associated with a custom plot
QCPGraph* createGraph(CustomPlot* pPlot, VectorXd const& xData, VectorXd const& yData, QColor const& color, QString const& name, double lineWidth)
{
    if (xData.size() == 0 || xData.size() != yData.size())
        return nullptr;
    QCPGraph* pGraph = new QCPGraph(pPlot->xAxis, pPlot->yAxis);
    pGraph->setData(convert(xData), convert(yData));
    pGraph->setSelectable(QCP::SelectionType::stSingleData);
    pGraph->setName(name);
    pGraph->setPen(QPen(color, lineWidth));
    pGraph->setAdaptiveSampling(true);
    return pGraph;
}
