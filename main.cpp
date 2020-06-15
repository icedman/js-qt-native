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
    QCommandLineOption entryOption(QStringList() << "e" << "entry", "set entry script", "entry", "");
    QCommandLineOption hostOption(QStringList() << "x" << "host", "development host", "host", "");
    parser.addHelpOption();
    parser.addOption(inspectOption);
    parser.addOption(htmlOption);
    parser.addOption(entryOption);
    parser.addOption(hostOption);
    parser.process(app);

    Engine engine;
    engine.addFactory(new UICoreFactory());

    // engine.mount("{ \"id\": \"mainWindow\", \"type\": \"MainWindow\", \"persist\": true }");

    if (parser.value(hostOption) != "") {
        // qDebug() << "host";
        engine.runFromUrl(QUrl(parser.value(hostOption)));
    } else if (parser.value(entryOption) != "") {
        QString entryPath = parser.value(entryOption);
        QUrl base = QUrl::fromLocalFile(QFileInfo(entryPath).absolutePath());
        QUrl url = QUrl::fromLocalFile(QFileInfo(entryPath).absoluteFilePath());
        qDebug() << url;
        engine.runFromUrl(url);
    } else {
        // todo load from qrc <deployed app>
    }

    if (parser.isSet(inspectOption)) {
        engine.showInspector(parser.isSet(htmlOption));
    }

    return app.exec();;
}