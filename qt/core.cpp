#include "core.h"
#include "engine.h"

#include <QLayout>
#include <QLayoutItem>

QJsonObject toJson(QString json)
{
    QByteArray bytes;
    bytes.append(json);
    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    return doc.object();
}

QJsonObject toQss(QJsonObject json) {
    // qDebug() << json;
    return json;
}

static void applyStyle(QString qtWidgetName, UIObject *obj, QJsonObject json) {
    QWidget *w = obj->widget();
    if (!w) {
        return;
    }
    w->setProperty("id", json.value("id").toString());
    w->setProperty("className", json.value("className").toString());
    
    // style
    QJsonObject style = toJson(json.value("style").toString());
    if (json.contains("style")) {
        QString qss = qtWidgetName;
        qss += " ";
        qss += QJsonDocument(QJsonObject(style)).toJson(QJsonDocument::Compact);
        qss = qss.replace("\"", "");
        w->setStyleSheet(qss);
    } else {
        w->setStyleSheet("");
    }

    // flexbox
    QBoxLayout *l = obj->layout();
    if (l && style.contains("flexDirection")) {
        if (style.value("flexDirection") == "row") {
            l->setDirection(QBoxLayout::LeftToRight);
        }
        if (style.value("flexDirection") == "row-reverse") {
            l->setDirection(QBoxLayout::RightToLeft);
        }
        if (style.value("flexDirection") == "column") {
            l->setDirection(QBoxLayout::TopToBottom);
        }
        if (style.value("flexDirection") == "column-reverse") {
            l->setDirection(QBoxLayout::BottomToTop);
        }
    }
    w->setProperty("flex", style.value("flex").toInt());
    w->setProperty("alignItems", style.value("alignItems").toString());
    w->setProperty("justifyContent", style.value("justifyContent").toString());
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
    view = new View();
    uiObject->setCentralWidget(view->widget());
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

bool MainWindow::addChild(UIObject *obj) {
    view->addChild(obj);
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
    relayout();
    return true;
}

bool View::addChild(UIObject *obj) {
    layout()->addWidget(obj->widget());
    relayout();
    return true;
};

void View::relayout() {
    QBoxLayout *l = layout();
    for (int i = 0; i < l->count(); ++i) {
        QLayoutItem *layoutItem = l->itemAt(i);
        if (layoutItem->spacerItem()) {
            l->removeItem(layoutItem);
            delete layoutItem;
            --i;
            continue;
        }
        QWidget *w = layoutItem->widget();
        if (w) {
            int stretch = w->property("flex").toInt();
            l->setStretch(i, stretch);
        }
    }

    QString align = widget()->property("alignItems").toString();
    QString justify = widget()->property("justifyContent").toString();

    if (justify == "space-around" || justify == "space-between") {
        int c = l->count() - 1;
        for (int i = 0; i < c; i++) {
            l->insertStretch((i*2)+1, 1);
        }
    }

    if (align == "flex-start" || align == "center" || justify == "center"  || justify == "space-around") {
        l->insertStretch(-1, 1);
    }
    if (align == "flex-end" || align == "center" || justify == "center"  || justify == "space-around") {
        l->insertStretch(0, 1);
    }
}

//----------------------------
// ScrollView
//----------------------------
ScrollView::ScrollView() :
    uiObject(new QScrollArea)
{
    view = new QWidget();
    view->setLayout(new QVBoxLayout());
    uiObject->setWidget(view);
    uiObject->setWidgetResizable(true);
}

ScrollView::~ScrollView()
{
    uiObject->deleteLater();
}

bool ScrollView::update(QJsonObject json) {
    applyStyle("QScrollArea", this, json);
    return true;
}

bool ScrollView::addChild(UIObject *obj) {
    layout()->addWidget(obj->widget()); return true;
};


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
    applyStyle("QPushButton", this, json);
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
    END_UI()

    BEGIN_UI_DEF(ScrollView)
    END_UI()

    BEGIN_UI_DEF(Text)
    END_UI()

    BEGIN_UI_DEF(TextInput)
    END_UI()

    BEGIN_UI_DEF(Button)
    END_UI()
    
    return NULL; 
}
