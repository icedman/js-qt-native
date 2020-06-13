#include <QSplitter>
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QDebug>
#include <QFile>

#include "engine.h"
#include "core.h"

Engine::Engine(QWidget* parent) :
    QWidget(parent)
    , updateTimer(this)
{
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);

    QVBoxLayout* box = new QVBoxLayout(this);
    setLayout(box);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    view = new QWebView(this);
    frame = view->page()->mainFrame();

    connect(frame, SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(setupEnvironment()));

    inspector = new QWebInspector();
    inspector->setPage(view->page());
    inspector->setVisible(true);

    box->addWidget(splitter);
    box->setMargin(0);
    box->setSpacing(0);
    splitter->addWidget(view);
    splitter->addWidget(inspector);

    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(render()));
    updateTimer.start(50);
}

void Engine::runDevelopment(QString path)
{
    view->setUrl(QUrl(path));
}

void Engine::addFactory(UIFactory *factory)
{
    factories.push_back(factory);
}

void Engine::setupEnvironment()
{
    frame->addToJavaScriptWindowObject("$qt", this);

    // hotreload
    for(auto k : registry.keys()) {
        UIObject *obj = registry.value(k);
        if (!obj->property("persistent").toBool()) {
            unmounts << toJson("{\"id\": \"" + obj->property("id").toString() + "\"}");
        }
    }
}

UIObject* Engine::findInRegistry(QString id, QJsonObject json)
{
    if (!json.contains(id)) {
        return NULL;
    }
    QString _id = json.value(id).toString();
    if (!registry.contains(_id)) {
        return NULL;
    }
    return registry.value(_id);
}

UIObject* Engine::addToRegistry(QJsonObject json, UIObject* object)
{
    if (!json.contains("id")) {
        return NULL;
    }
    QString id = json.value("id").toString();
    object->setParent(this);
    object->setProperty("id", id);
    object->setProperty("persistent", json.contains("persistent"));
    registry.insert(id, object);
    return object;
}

QVariant Engine::runScript(QString script)
{
    return frame->evaluateJavaScript(script);
}

QVariant Engine::runScriptFile(QString path)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        return runScript(file.readAll());
    }
    return QVariant();
}

//--------------------
// private slots
//--------------------
void Engine::render()
{
    QList<QJsonObject> retry;

    for(auto doc : mounts) {
        UIObject *obj = findInRegistry("id", doc);
        UIObject *parent = findInRegistry("parent", doc);
        if (!parent && doc.contains("parent")) {
            retry << doc;
            continue;
        }

        if (!obj) {
            // create if not exists
            for(auto f : factories) {
                obj = f->create(doc);
                if (obj) {
                    obj->engine = this; //
                    if (parent) {
                        parent->addChild(obj);
                    }
                    obj->mount(doc);
                    obj->update(doc);
                    break;
                }
            }
            if (obj) {
                if (addToRegistry(doc, obj)) {
                    qDebug() << "added to registry";
                }
            } else {
                qDebug() << "unable to create";
            }
        } else {
            obj->update(doc);
            qDebug() << "already exists";
        }
    }
    mounts.clear();
    mounts << retry;

    retry.clear();
    
    for(auto doc : updates) {
        UIObject *obj = findInRegistry("id", doc);
        if (obj) {
            obj->update(doc);
        } else {
            retry << doc;
        }
    }
    updates.clear();
    updates << retry;

    for(auto doc : unmounts) {
        UIObject *obj = findInRegistry("id", doc);
        if (obj) {
            obj->unmount(doc);
            obj->deleteLater();
            qDebug() << "-----------------";
            qDebug() << "unmount";
            qDebug() << doc;
            registry.remove(doc.value("id").toString());
        }
    }
    unmounts.clear();
}

//--------------------
// public slots
//--------------------
void Engine::showInspector(bool withHtml)
{
    if (withHtml) {
        view->show();
    } else {
        view->hide();
    }

    resize(1200, 800);
    show();
}

void Engine::mount(QString json)
{
    mounts.push_back(toJson(json));
}

void Engine::update(QString json)
{
    updates.push_back(toJson(json));
}

void Engine::unmount(QString json)
{
    unmounts.push_back(toJson(json));
}