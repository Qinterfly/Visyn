#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QListWidget)

namespace Backend::Core
{
struct HarmonicSolution;
struct Segment;
class Response;
struct Project;
}

namespace Frontend
{

class CustomPlot;
class CustomTabWidget;
class ResponseView;
class SegmentView;

class ViewManager : public QWidget
{
    Q_OBJECT

public:
    ViewManager(Backend::Core::Project const& project, QWidget* pParent = nullptr);
    virtual ~ViewManager();

    QSize sizeHint() const override;

    void clear();
    void plot();

private:
    // Content
    void createContent();
    void createDockManager();

private:
    Backend::Core::Project const& mProject;

    CustomTabWidget* mpTabWidget;
    ResponseView* mpResponseView;
    ResponseView* mpRefSpectrumView;
    SegmentView* mpSegmentView;
    ResponseView* mpResSpectrumView;
};

class ResponseView : public QWidget
{
    Q_OBJECT

public:
    enum Type
    {
        kNone,
        kReal,
        kImag,
        kAmplitude,
        kPhase
    };
    struct Options
    {
        Options();
        Options(Type uUpType, Type uDownType);
        ~Options() = default;

        Type upType;
        Type downType;
        QList<int> indicesSelected;
        QList<QColor> colors;
    };
    ResponseView(QList<Backend::Core::Response> const& responses, Options const& options = Options());
    virtual ~ResponseView() = default;

    void clear();
    void refresh();
    void plot();

private:
    // Content
    void createContent();
    void createConnections();

    // Render
    void draw(Type type, CustomPlot* pPlot);

    // Slots
    void setOptions();
    void processSelected();

private:
    QList<Backend::Core::Response> const& mResponses;
    Options mOptions;

    // Widgets
    QComboBox* mpUpComboBox;
    QComboBox* mpDownComboBox;
    CustomPlot* mpUpPlot;
    CustomPlot* mpDownPlot;
    QListWidget* mpSelectList;

    // Selection
    QList<bool> mMaskSelected;
};

class SegmentView : public QWidget
{
    Q_OBJECT

public:
    SegmentView(Backend::Core::HarmonicSolution const& solution, QWidget* pParent = nullptr);
    virtual ~SegmentView() = default;

    void clear();
    void plot();

private:
    Backend::Core::HarmonicSolution const& mSolution;
    CustomPlot* mpPlot;
};

}

#endif // VIEWMANAGER_H
