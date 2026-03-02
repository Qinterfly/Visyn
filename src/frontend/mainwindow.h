
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTranslator>

#include "project.h"

namespace ads
{
class CDockWidget;
class CDockManager;
}

namespace Frontend
{

class CustomStatusBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* pParent = nullptr, bool isRestore = true);
    virtual ~MainWindow();

    // Objects
    Backend::Core::Project const& project() const;
    void setProject(Backend::Core::Project const& project);

public:
    static QString language;
    static CustomStatusBar* pStatusBar;

private:
    void initializeWindow();

    // Content
    void createContent();
    void createDockManager();
    void createWindowActions();
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
    void about();

private:
    QSettings mSettings;

    // UI
    ads::CDockManager* mpDockManager;
    QMenu* mpWindowMenu;

    // Project
    Backend::Core::Project mProject;

    // Translations
    QTranslator mTranslatorApplication;
    QTranslator mTranslatorQt;
};

void logMessage(QtMsgType type, QMessageLogContext const& /*context*/, QString const& message);
}

#endif // MAINWINDOW_H
