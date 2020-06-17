#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QSplitter>
#include <QVBoxLayout>

#include "core.h"
#include "engine.h"

#define UPDATE_FREQ 50

Engine::Engine(QWidget* parent)
    : QWidget(parent)
    , updateTimer(this)
{
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);

    QVBoxLayout* box = new QVBoxLayout(this);
    setLayout(box);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);

    view = new QWebView(this);
    frame = view->page()->mainFrame();

    connect(frame, SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(startEngine()));

    inspector = new QWebInspector();
    inspector->setPage(view->page());
    inspector->setVisible(true);

    box->addWidget(splitter);
    box->setMargin(0);
    box->setSpacing(0);
    splitter->addWidget(view);
    splitter->addWidget(inspector);

    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(render()));
    updateTimer.start(UPDATE_FREQ);
}

void Engine::runFromUrl(QUrl path)
{
    view->setUrl(path);
    basePath = path.adjusted(QUrl::RemoveFilename);

    qDebug() << basePath;
}

void Engine::addFactory(UIFactory* factory) { factories.push_back(factory); }

bool Engine::loadHtml(QString content, QUrl base)
{
    basePath = base.adjusted(QUrl::RemoveFilename);

    // hack
    content.replace("src=\"/", "src=\"");

    // content.replace("<script", "<!--script");
    // content.replace("/script>", "/script-->");
    view->setHtml(content, base);
    return true;
}

bool Engine::loadHtmlFile(QString path, QUrl base)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        return loadHtml(file.readAll(), base);
    }
    return true;
}

void Engine::startEngine()
{
    frame->addToJavaScriptWindowObject("$qt", this);

    // this happens at reload
    for (auto k : registry.keys()) {
        UIObject* obj = registry.value(k);
        unmounts << toJson("{\"id\": \"" + obj->property("id").toString() + "\"}");
    }

    emit engineReady();
}

UIObject* Engine::create(QString id, QString type, bool persistent)
{    
    QString jsonString;
    jsonString += "{";
    jsonString += "\"id\": \"" + id + "\",";
    jsonString += "\"type\": \"" + type + "\",";
    jsonString += "\"persistent\": ";
    jsonString += (persistent ? "true" : "false");
    jsonString += "}";
    // qDebug() << "----------------";
    // qDebug() << jsonString;
    mount(jsonString);
    render();
    return findInRegistryById(id);
}

UIObject* Engine::findInRegistryById(QString id)
{
    return registry.value(id);
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
    if (!mounts.size() && !updates.size() && !unmounts.size()) {
        return;
    }
    
    updateTimer.stop();

    QList<QJsonObject> retry;

    for (auto doc : mounts) {
        UIObject* obj = findInRegistry("id", doc);
        UIObject* parent = findInRegistry("parent", doc);
        if (!obj) {
            // create if not exists
            for (auto f : factories) {
                obj = f->create(doc);
                if (obj) {
                    obj->engine = this;
                    obj->mount(doc);
                    obj->update(doc);
                    if (parent) {
                        parent->addChild(obj);
                    }
                    break;
                }
            }
            if (obj) {
                if (addToRegistry(doc, obj)) {
                    // qDebug() << "added to registry";
                }
            } else {
                qDebug() << "unable to create";
                qDebug() << doc;
            }
        } else {
            obj->update(doc);
            qDebug() << "already exists";
        }
    }
    mounts.clear();
    mounts << retry;
    retry.clear();

    for (auto doc : updates) {
        UIObject* obj = findInRegistry("id", doc);
        if (obj) {
            obj->update(doc);
            if (!obj->widget()->parent()) {
                UIObject* parent = findInRegistry("parent", doc);
                if (parent) {
                    // qDebug() << "parented on update";
                    // qDebug() << doc.value("parent").toString();
                    parent->addChild(obj);
                }
            }
        } else {
            retry << doc;
        }
    }
    updates.clear();
    updates << retry;

    for (auto doc : unmounts) {
        UIObject* obj = findInRegistry("id", doc);
        if (obj->property("persistent").toBool()) {
            continue;
        }
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

    updateTimer.start(UPDATE_FREQ);
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

void Engine::mount(QString json) { mounts.push_back(toJson(json)); }

void Engine::update(QString json) { updates.push_back(toJson(json)); }

void Engine::unmount(QString json) { unmounts.push_back(toJson(json)); }