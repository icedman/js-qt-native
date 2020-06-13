#pragma once

#include <QTimer>
#include <QWebFrame>
#include <QWebInspector>
#include <QWebPage>
#include <QWebView>
#include <QWidget>
#include <QJsonObject>

class UIObject;
class UIFactory;

class Engine : public QWidget {
    Q_OBJECT
public:
    Engine(QWidget* parent = 0);

    QWebView* view;
    QWebFrame* frame;
    QWebInspector* inspector;
    
    QVariant runScript(QString script);
    QVariant runScriptFile(QString path);

    void runDevelopment(QString path);
    void addFactory(UIFactory *factory);

    UIObject* findInRegistry(QString id, QJsonObject json);
    UIObject* addToRegistry(QJsonObject json, UIObject* object);
    
public slots:
    void showInspector(bool withHtml);

    void mount(QString json);
    void update(QString json);
    void unmount(QString json);

private Q_SLOTS:
    void setupEnvironment();    
    void render();

private:
    QTimer updateTimer;
    QMap<QString, UIObject*> registry;

    // requests
    QList<QJsonObject> mounts;
    QList<QJsonObject> unmounts;
    QList<QJsonObject> updates;

    QList<UIFactory*> factories;
};