#pragma once

#include <QBoxLayout>
#include <QObject>
#include <QFrame>
#include <QWidget>

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>

#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include <QSplitter>
#include <QStatusBar>
#include <QStackedWidget>

#define BEGIN_UI_DEF(T) \
    if (type == #T) {   \
        T* uiObject = new T();

#define END_UI()     \
    return uiObject; \
    }

QJsonObject toJson(QString json);

class Engine;
class View;

class QSpacerItem;
class QNetworkReply;

class UIObject : public QObject {
    Q_OBJECT
public:
    virtual bool mount(QJsonObject json) = 0;
    virtual bool update(QJsonObject json) = 0;
    virtual bool unmount() = 0;
    virtual bool addChild(UIObject* obj) = 0;
    virtual QWidget* widget() = 0;
    virtual QBoxLayout* layout() = 0;
    virtual void addToJavaScriptWindowObject() = 0;

    Engine* engine;
};

class Window : public UIObject {
    Q_OBJECT
public:
    Window();
    ~Window();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; };
    bool unmount() override
    {
        this->deleteLater();
        return true;
    };
    bool addChild(UIObject* obj) override;

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout()
    {
        return qobject_cast<QBoxLayout*>(uiObject->centralWidget()->layout());
    }

    void addToJavaScriptWindowObject() override;

private:
    QMainWindow* uiObject;
    View* view;
};

class TouchableWidget : public QFrame
{
    Q_OBJECT
private:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

signals:
    void pressed();
    void released();
};

class View : public UIObject {
    Q_OBJECT
public:
    View();
    ~View();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; };
    bool unmount() override
    {
        this->deleteLater();
        return true;
    };
    bool addChild(UIObject* obj) override;

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout()
    {
        return qobject_cast<QBoxLayout*>(uiObject->layout());
    }

    void addToJavaScriptWindowObject() override;

private:
    void relayout();

    TouchableWidget* uiObject;

public Q_SLOTS:
    void onPress();
    void onRelease();
};

class ScrollView : public UIObject {
    Q_OBJECT
public:
    ScrollView();
    ~ScrollView();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; };
    bool unmount() override
    {
        this->deleteLater();
        return true;
    };
    bool addChild(UIObject* obj) override;

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout() { return qobject_cast<QBoxLayout*>(view->layout()); }

    void addToJavaScriptWindowObject() override;

private:
    QScrollArea* uiObject;
    QWidget* view;
};

class StatusBar : public UIObject {
    Q_OBJECT
public:
    StatusBar();
    ~StatusBar();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; };
    bool unmount() override
    {
        this->deleteLater();
        return true;
    };
    bool addChild(UIObject* obj) override;

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout()
    {
        return qobject_cast<QBoxLayout*>(uiObject->layout());
    }

    void addToJavaScriptWindowObject() override;

private:
    void relayout();
    
    QStatusBar* uiObject;
    
public Q_SLOTS:
    
    void showMessage(QString msg, int timeout);
    void clearMessage();
};

class SplitterView : public UIObject {
    Q_OBJECT
public:
    SplitterView();
    ~SplitterView();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; };
    bool unmount() override
    {
        this->deleteLater();
        return true;
    };
    bool addChild(UIObject* obj) override;

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout()
    {
        return qobject_cast<QBoxLayout*>(uiObject->layout());
    }

    void addToJavaScriptWindowObject() override;

private:
    QSplitter* uiObject;
};

class StackedView : public UIObject {
    Q_OBJECT
public:
    StackedView();
    ~StackedView();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; };
    bool unmount() override
    {
        this->deleteLater();
        return true;
    };
    bool addChild(UIObject* obj) override;

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout()
    {
        return qobject_cast<QBoxLayout*>(uiObject->layout());
    }

    void addToJavaScriptWindowObject() override;

private:
    QStackedWidget* uiObject;
};

class Text : public UIObject {
    Q_OBJECT
public:
    Text();
    ~Text();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; };
    bool unmount() override
    {
        this->deleteLater();
        return true;
    };
    bool addChild(UIObject* obj) override
    {
        layout()->addWidget(obj->widget());
        return true;
    };

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout()
    {
        return qobject_cast<QBoxLayout*>(uiObject->layout());
    }

    void addToJavaScriptWindowObject() override;

private:
    QLabel* uiObject;
};

class TextInput : public UIObject {
    Q_OBJECT
public:
    TextInput();
    ~TextInput();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; };
    bool unmount() override
    {
        this->deleteLater();
        return true;
    };
    bool addChild(UIObject* obj) override
    {
        layout()->addWidget(obj->widget());
        return true;
    };

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout()
    {
        return qobject_cast<QBoxLayout*>(uiObject->layout());
    }

    void addToJavaScriptWindowObject() override;

public Q_SLOTS:
    
    void focus();
    void select();

private:
    QLineEdit* uiObject;

private Q_SLOTS:
    void onChange(QString val);
    void onSubmit();
};

class Image : public UIObject {
    Q_OBJECT
public:
    Image();
    ~Image();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; };
    bool unmount() override
    {
        this->deleteLater();
        return true;
    };
    bool addChild(UIObject* obj) override
    {
        layout()->addWidget(obj->widget());
        return true;
    };

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout()
    {
        return qobject_cast<QBoxLayout*>(uiObject->layout());
    }

    void addToJavaScriptWindowObject() override;

private Q_SLOTS:
    void replyFinished(QNetworkReply* reply);

private:
    QLabel* uiObject;
    QNetworkAccessManager* netman;
    QString lastSource;
    QImage image;
};

class Button : public UIObject {
    Q_OBJECT
public:
    Button();
    ~Button();

    bool update(QJsonObject json) override;
    bool mount(QJsonObject json) override { return true; };
    bool unmount() override
    {
        this->deleteLater();
        return true;
    };
    bool addChild(UIObject* obj) override
    {
        layout()->addWidget(obj->widget());
        return true;
    };

    QWidget* widget() { return uiObject; }
    QBoxLayout* layout()
    {
        return qobject_cast<QBoxLayout*>(uiObject->layout());
    }

    void addToJavaScriptWindowObject() override;
    
private:
    QPushButton* uiObject;

private Q_SLOTS:
    void onClick(bool checked);
    void onPress();
    void onRelease();
};

class UIFactory : public QObject {
public:
    UIFactory(QObject* engine = 0);

    virtual UIObject* create(QString jsonString)
    {
        QByteArray bytes;
        bytes.append(jsonString);
        QJsonDocument doc = QJsonDocument::fromJson(bytes);
        QJsonObject json = doc.object();
        return create(json);
    }
    
    virtual UIObject* create(QJsonObject json) { return NULL; }

private:
    Engine* engine;
};

class UICoreFactory : public UIFactory {
public:
    UIObject* create(QJsonObject json) override;
};