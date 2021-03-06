#pragma once

#include <QJsonObject>
#include <QTimer>
#include <QWebFrame>
#include <QWebInspector>
#include <QWebPage>
#include <QWebView>
#include <QWidget>

class UIObject;
class UIFactory;
class QNetworkReply;

class Engine : public QWidget {
    Q_OBJECT
public:
    Engine(QWidget* parent = 0);

    QUrl basePath;

    QWebView* view;
    QWebFrame* frame;
    QWebInspector* inspector;

    QVariant runScript(QString script);
    QVariant runScriptFile(QString path);

    bool loadHtml(QString content, QUrl base);
    bool loadHtmlFile(QString path, QUrl base);

    void runFromUrl(QUrl path);
    void addFactory(UIFactory* factory);

    UIObject* findInRegistryById(QString id);
    UIObject* findInRegistry(QString key, QJsonObject json);
    UIObject* addToRegistry(QJsonObject json, UIObject* object);
    UIObject* create(QString id, QString type, bool persistent);

    QIcon icon(QString id);
    QIcon registerIcon(QString id, QIcon icon);
    
public slots:
    void showInspector(bool withHtml);
    void hideInspector();

    void mount(QString json);
    void update(QString json);
    void unmount(QString json);
    void widget(QString id);

signals:
    void engineReady();

private Q_SLOTS:
    void startEngine();
    void render();

private:
    QTimer updateTimer;
    QMap<QString, UIObject*> registry;
    QList<UIObject*> garbage;

    // icons    
    QMap<QString, QIcon> icons;
    
    // requests
    QList<QJsonObject> mounts;
    QList<QJsonObject> unmounts;
    QList<QJsonObject> updates;

    QList<UIFactory*> factories;
};