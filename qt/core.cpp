#include "core.h"
#include "engine.h"

QJsonObject toJson(QString json)
{
    QByteArray bytes;
    bytes.append(json);
    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    return doc.object();
}

static void applyStyle(QString qtWidgetName, UIObject *obj, QJsonObject json) {
    QWidget *w = obj->widget();
    if (!w) {
        return;
    }
    w->setProperty("className", json.value("className").toString());
    
    QJsonObject style = toJson(json.value("style").toString());
    if (json.contains("style")) {
        QString qss = qtWidgetName;
        qss += " ";
        qss += json.value("style").toString();
        qss = qss.replace("\"", "");
        w->setStyleSheet(qss);
    } else {
        w->setStyleSheet("");
    }

    QBoxLayout *l = obj->layout();
    if (l && style.contains("direction")) {
        if (style.value("direction") == "row") {
            l->setDirection(QBoxLayout::LeftToRight);
        }
        if (style.value("direction") == "row-reverse") {
            l->setDirection(QBoxLayout::RightToLeft);
        }
        if (style.value("direction") == "column") {
            l->setDirection(QBoxLayout::TopToBottom);
        }
        if (style.value("direction") == "column-rever") {
            l->setDirection(QBoxLayout::BottomToTop);
        }
    }
}

//----------------------------
// base factory
//----------------------------
UIFactory::UIFactory(QObject *parent) :
    QObject(parent)
{
    engine = qobject_cast<Engine*>(parent);
}

//----------------------------
// MainWindow
//----------------------------
MainWindow::MainWindow() :
    uiObject(new QMainWindow)
{
    QWidget* main = new QWidget();
    main->setLayout(new QVBoxLayout());
    uiObject->setCentralWidget(main);
    uiObject->resize(1200, 800);
}

MainWindow::~MainWindow()
{
    uiObject->deleteLater();
}

bool MainWindow::update(QJsonObject json) {
    applyStyle("QMainWindow", this, json);
    return true;
}

//----------------------------
// View
//----------------------------
View::View() :
    uiObject(new QWidget)
{
    uiObject->setLayout(new QVBoxLayout());
}

View::~View()
{
    uiObject->deleteLater();
}

bool View::update(QJsonObject json) {
    applyStyle("QWidget", this, json);
    return true;
}

//----------------------------
// Text
//----------------------------
Text::Text() :
    uiObject(new QLabel)
{
    uiObject->setLayout(new QVBoxLayout());
    uiObject->setTextFormat(Qt::RichText);
    uiObject->setTextInteractionFlags(Qt::NoTextInteraction);
}

Text::~Text()
{
    uiObject->deleteLater();
}

bool Text::update(QJsonObject json) {
    applyStyle("QLabel", this, json);
    if (json.contains("text")) {
        uiObject->setText(json.value("text").toString());
    } else if (json.contains("renderedText")) {
        uiObject->setText(json.value("renderedText").toString());
    }
    return true;
}

//----------------------------
// TextInput
//----------------------------
TextInput::TextInput() :
    uiObject(new QLineEdit)
{
    uiObject->setLayout(new QVBoxLayout());

    connect(uiObject, SIGNAL(textEdited(QString)), this, SLOT(onChange(QString)));
    connect(uiObject, SIGNAL(returnPressed()), this, SLOT(onSubmit()));
}

TextInput::~TextInput()
{
    uiObject->deleteLater();
}

bool TextInput::update(QJsonObject json) {
    applyStyle("QLineEdit", this, json);
    uiObject->setText(json.value("text").toString());
    return true;
}

void TextInput::onChange(QString value)
{
    QString id = property("id").toString();
    QString script = "$events[\"" + id + "\"].onChange({ target: { src: \"" + id + "\", value: \"" + value + "\" }})";
    engine->runScript(script);
}

void TextInput::onSubmit()
{
    QString id = property("id").toString();
    QString value = ""; //
    QString script = "$events[\"" + id + "\"].onSubmit({ target: { src: \"" + id + "\", value: \"" + value + "\" }})";
    engine->runScript(script);
}

//----------------------------
// Button
//----------------------------
Button::Button() :
    uiObject(new QPushButton)
{
    uiObject->setLayout(new QHBoxLayout());
    connect(uiObject, SIGNAL(clicked(bool)), this, SLOT(onClick(bool)));
}

Button::~Button()
{
    uiObject->deleteLater();
}

bool Button::update(QJsonObject json) {
    applyStyle("QPushButton", this, json);
    if (json.contains("text")) {
        uiObject->setText(json.value("text").toString());
    }
    return true;
}

void Button::onClick(bool checked) {
    QString id = property("id").toString();
    QString value = ""; //
    QString script = "$events[\"" + id + "\"].onClick({ target: { src: \"" + id + "\", value: \"" + value + "\" }})";
    qDebug() << script;
    engine->runScript(script);
}

//----------------------------
// Core factory
//----------------------------
UIObject* UICoreFactory::create(QJsonObject json) {
    QString type = json.value("type").toString();

    // qDebug() << json;
    // qDebug() << "create" << type;

    BEGIN_UI_DEF(MainWindow)
        uiObject->widget()->show();
    END_UI()

    BEGIN_UI_DEF(View)
        uiObject->widget()->show();
    END_UI()

    BEGIN_UI_DEF(Text)
    END_UI()

    BEGIN_UI_DEF(TextInput)
    END_UI()

    BEGIN_UI_DEF(Button)
    END_UI()
    
    return NULL; 
}
