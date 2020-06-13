#include <QApplication>
#include <QCommandLineParser>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QFileInfo>

#include "qt/engine.h"
#include "qt/core.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    
    QCommandLineParser parser;
    QCommandLineOption inspectOption(QStringList() << "i" << "inspect", "show web inspector");
    QCommandLineOption htmlOption(QStringList() << "m" << "html", "inspect with html view");
    QCommandLineOption entryOption(QStringList() << "e" << "entry", "set entry script", "entry", "./index.js");
    QCommandLineOption devModeOption(QStringList() << "d" << "develop", "run in development mode");
    QCommandLineOption devHostOption(QStringList() << "x" << "host", "development host", "host", "http://localhost:1234");
    parser.addHelpOption();
    parser.addOption(inspectOption);
    parser.addOption(htmlOption);
    parser.addOption(entryOption);
    parser.addOption(devModeOption);
    parser.addOption(devHostOption);
    parser.process(app);

    Engine engine;
    engine.addFactory(new UICoreFactory());

    // engine.mount("{ \"id\": \"mainWindow\", \"type\": \"MainWindow\", \"persist\": true }");

    if (parser.isSet(devModeOption)) {
        engine.runDevelopment(parser.value(devHostOption));
        // qDebug() << parser.value(devHostOption);
    } else {
        QString entryPath = parser.value(entryOption);
        QUrl base = QUrl::fromLocalFile(QFileInfo(entryPath).absolutePath() + "/");
        // qDebug() << base;
        engine.loadHtmlFile(entryPath, base);
    }

    if (parser.isSet(inspectOption)) {
        engine.showInspector(parser.isSet(htmlOption));
    }

    return app.exec();;
}