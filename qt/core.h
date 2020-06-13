#pragma once

#include <QObject>
#include <QWidget>
#include <QBoxLayout>

#include <QJsonObject>
#include <QJsonDocument>

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QScrollArea>

#define BEGIN_UI_DEF(T) \
    if (type == #T) { \
        T* uiObject = new T();

#define END_UI()     \
    return uiObject; \
    }

QJsonObject toJson(QString json);

class Engine;
class View;
class QSpacerItem;

class UIObject : public QObject {
    Q_OBJECT
public:
    virtual bool mount(QJsonObject json) = 0; 
    virtual bool update(QJsonObject json) = 0;
    virtual bool unmount(QJsonObject json) = 0;
    virtual bool addChild(UIObject *obj) = 0;
    virtual QWidget* widget() = 0;
    virtual QBoxLayout* layout() = 0;

    Engine* engine;
};

class MainWindow : public UIObject
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; }; 
    bool unmount(QJsonObject json) override { this->deleteLater(); return true; };
    bool addChild(UIObject *obj) override;

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout() { return qobject_cast<QBoxLayout*>(uiObject->centralWidget()->layout()); }

private:
    QMainWindow* uiObject;
    View *view;
};

class View : public UIObject
{
   Q_OBJECT
public:
    View();
    ~View();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; }; 
    bool unmount(QJsonObject json) override { this->deleteLater(); return true; };
    bool addChild(UIObject *obj) override;

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout() { return qobject_cast<QBoxLayout*>(uiObject->layout()); }

private:
    void relayout();
    
private:
    QWidget *uiObject;
};

class ScrollView : public UIObject
{
   Q_OBJECT
public:
    ScrollView();
    ~ScrollView();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; }; 
    bool unmount(QJsonObject json) override { this->deleteLater(); return true; };
    bool addChild(UIObject *obj) override;

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout() { return qobject_cast<QBoxLayout*>(view->layout()); }

private:
    QScrollArea *uiObject;
    QWidget *view;
};

class Text : public UIObject
{
   Q_OBJECT
public:
    Text();
    ~Text();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; }; 
    bool unmount(QJsonObject json) override { this->deleteLater(); return true; };
    bool addChild(UIObject *obj) override { layout()->addWidget(obj->widget()); return true; };

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout() { return qobject_cast<QBoxLayout*>(uiObject->layout()); }

private:
    QLabel *uiObject;
};

class TextInput : public UIObject
{
   Q_OBJECT
public:
    TextInput();
    ~TextInput();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; }; 
    bool unmount(QJsonObject json) override { this->deleteLater(); return true; };
    bool addChild(UIObject *obj) override { layout()->addWidget(obj->widget()); return true; };

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout() { return qobject_cast<QBoxLayout*>(uiObject->layout()); }

private:
    QLineEdit *uiObject;

private Q_SLOTS:
    void onChange(QString val);
    void onSubmit();
};

class Button : public UIObject
{
   Q_OBJECT
public:
    Button();
    ~Button();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; }; 
    bool unmount(QJsonObject json) override { this->deleteLater(); return true; };
    bool addChild(UIObject *obj) override { layout()->addWidget(obj->widget()); return true; };

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout() { return qobject_cast<QBoxLayout*>(uiObject->layout()); }

private:
    QPushButton *uiObject;

private Q_SLOTS:
    void onClick(bool checked);
};

class UIFactory : public QObject {
public:
    UIFactory(QObject *engine = 0);

    UIObject* create(QString jsonString) {
        QByteArray bytes;
        bytes.append(jsonString);
        QJsonDocument doc = QJsonDocument::fromJson(bytes);
        QJsonObject json = doc.object();
        return create(json); 
    }

    virtual UIObject* create(QJsonObject json) { return NULL; }

private:
    Engine *engine;
};

class UICoreFactory : public UIFactory
{
public:
    UIObject* create(QJsonObject json) override;    
};