
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTranslator>

#include "project.h"

namespace Frontend
{

class CustomStatusBar;
class ViewManager;
class ProjectEditor;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* pParent = nullptr, bool isRestore = true);
    virtual ~MainWindow();

    // File interaction
    void newProject();
    void saveAsProject(QString const& pathFile);

    // Objects
    Backend::Core::Project& project();

    // Widgets
    ViewManager* viewManager();
    ProjectEditor* projectEditor();

public:
    static QString language;
    static CustomStatusBar* pStatusBar;

private:
    void initializeWindow();

    // Content
    void createContent();
    void createFileActions();
    void createLanguageActions();
    void createHelpActions();
    void createConnections();

    // State
    void setProjectTitle();
    void setModified(bool flag);
    void setTheme();
    void setLanguage(QString const& newLanguage);
    void applyLanguage();
    void restart();

    // Settings
    void saveSettings();
    void restoreSettings();

    // Dialogs
    void saveAsProjectDialog();
    void about();

    // Slots
    void onEdited();

private:
    QSettings mSettings;

    // Ui
    ViewManager* mpViewManager;
    ProjectEditor* mpProjectEditor;

    // Project
    Backend::Core::Project mProject;

    // Translations
    QTranslator mTranslatorApplication;
    QTranslator mTranslatorQt;
};

void logMessage(QtMsgType type, QMessageLogContext const& /*context*/, QString const& message);
}

#endif // MAINWINDOW_H
