#include "core.h"
#include "engine.h"

#include <QApplication>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QLayout>
#include <QLayoutItem>
#include <QMouseEvent>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStyle>

QJsonObject toJson(QString json)
{
    QByteArray bytes;
    bytes.append(json);
    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    return doc.object();
}

QString toStyle(QJsonObject json)
{
    static const QStringList remove = {
        "flex",
        "flex-direction",
        "align-items",
        "justify-content",
        "visible",
        "icon-width",
        "icon-height"
    };
    for (auto s : remove) {
        json.remove(s);
    }

    QString qss = " ";
    qss += QJsonDocument(QJsonObject(json)).toJson(QJsonDocument::Compact);
    qss = qss.replace("\"", "");
    qss = qss.replace(",", ";");
    return qss;
}

QString toQss(QJsonObject json) {
    QString sheet = "";
    for(auto k : json.keys()) {
        sheet += "\n";
        sheet += k;
        sheet += toStyle(json.value(k).toObject());
    }
    return sheet;
}

static void applyStyle(QString qtWidgetName, UIObject* obj, QJsonObject json)
{
    QWidget* w = obj->widget();
    if (!w) {
        return;
    }
    w->setProperty("id", json.value("id").toString());
    w->setProperty("order", json.value("order").toInt());
    w->setProperty("className", json.value("className").toString());
    w->setProperty("permanent", json.contains("permanent"));

    // qDebug() << w->property("className").toString();

    QJsonObject style = json.value("style").toObject();
    QJsonObject sheet = json.value("qss").toObject();

    // geometry
    if (style.contains("width") || style.contains("height")) {
        w->resize(style.value("width").toInt(), style.value("height").toInt());
    }
    if (style.contains("minWidth") || style.contains("minHeight")) {
        w->setMinimumSize(style.value("minWidth").toInt(), style.value("minHeight").toInt());
    }
    if (style.contains("maxWidth") || style.contains("maxHeight")) {
        w->setMaximumSize(style.value("maxWidth").toInt(), style.value("maxHeight").toInt());
    }
    if (style.contains("visible")) {
        w->setVisible(style.value("visible").toBool() == true);
    }

    // flexbox
    QBoxLayout* l = obj->layout();
    if (l && style.contains("flex-direction")) {
        if (style.value("flex-direction") == "row") {
            l->setDirection(QBoxLayout::LeftToRight);
        }
        if (style.value("flex-direction") == "row-reverse") {
            l->setDirection(QBoxLayout::RightToLeft);
        }
        if (style.value("flex-direction") == "column") {
            l->setDirection(QBoxLayout::TopToBottom);
        }
        if (style.value("flex-direction") == "column-reverse") {
            l->setDirection(QBoxLayout::BottomToTop);
        }
    }
    w->setProperty("flex", style.value("flex").toInt());
    w->setProperty("align-items", style.value("align-items").toString());
    w->setProperty("justify-content", style.value("justify-content").toString());

    QString qss;

    // style qss
    if (json.contains("style") && !qtWidgetName.isEmpty()) {
        QString styleText = toStyle(style);
        if (styleText != " {}") {
            qss += qtWidgetName;
            qss += styleText;
        }
    }

    // sheet
    if (json.contains("qss")) {
        qss += toQss(sheet);
    }

    if (!qss.isEmpty()) {
        // qDebug() << qss;
        w->setStyleSheet(qss);
    }
}

//----------------------------
// base factory
//----------------------------
UIFactory::UIFactory(QObject* parent)
    : QObject(parent)
{
    engine = qobject_cast<Engine*>(parent);
}

//----------------------------
// Window
//----------------------------
Window::Window()
    : uiObject(new QMainWindow)
{
    view = new View();
    uiObject->setCentralWidget(view->widget());
    uiObject->resize(1200, 800);
}

Window::~Window() { uiObject->deleteLater(); }

bool Window::update(QJsonObject json)
{
    applyStyle("QMainWindow", this, json);
    // applyStyle("QWidget", view, json);
    return true;
}

bool Window::addChild(UIObject* obj)
{
    QStatusBar *status = qobject_cast<QStatusBar*>(obj->widget());
    if (status) {
        uiObject->setStatusBar(status);
        status->show();
        return true;
    }
    
    QMenuBar *bar = qobject_cast<QMenuBar*>(obj->widget());
    if (bar) {
        uiObject->setMenuBar(bar);
        bar->show();
        return true;
    }
    
    view->addChild(obj);
    return true;
}

void Window::addToJavaScriptWindowObject()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    engine->frame->addToJavaScriptWindowObject("$widgets_" + id.replace(':','_'), this);
}

//----------------------------
// View
//----------------------------
TouchableWidget::TouchableWidget() : QFrame() {
    hoverable = false;
    touchable = false;
}

void TouchableWidget::mousePressEvent(QMouseEvent *event) {
    event->ignore();
    if (touchable) {
        emit pressed();
    }
}

void TouchableWidget::mouseReleaseEvent(QMouseEvent *event) {
    event->ignore();
    if (touchable) {
        emit released();
    }
}

void TouchableWidget::mouseMoveEvent(QMouseEvent *event) {
    event->ignore();
}

void TouchableWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->ignore();
}

void TouchableWidget::enterEvent(QEvent *event) {
    if (hoverable) {
        QString className = property("className").toString();
        if (!className.contains("hover")) {
            className += " hover";
            setProperty("className", className); 
            QApplication *app = qobject_cast<QApplication*>(QApplication::instance()); 
            app->style()->unpolish(app);
            app->style()->polish(app);
            update();
        }
    }
    event->ignore();
}

void TouchableWidget::leaveEvent(QEvent *event) {
    if (hoverable) {
        QString className = property("className").toString();
        if (className.contains("hover")) {
            className.replace(" hover", "");
            setProperty("className", className);
            QApplication *app = qobject_cast<QApplication*>(QApplication::instance()); 
            app->style()->unpolish(app);
            app->style()->polish(app);
            update();
        }
    }
    event->ignore();
}


void TouchableWidget::focusInEvent(QFocusEvent *event)
{
    QFrame::focusInEvent(event);
    update();
}

void TouchableWidget::focusOutEvent(QFocusEvent *event)
{
    QFrame::focusOutEvent(event);
    update();
}
    
View::View()
    : uiObject(new TouchableWidget)
{
    uiObject->setLayout(new QVBoxLayout());
    uiObject->layout()->setMargin(0);
    uiObject->layout()->setSpacing(0);
    connect(uiObject, SIGNAL(pressed()), this, SLOT(onPress()));
    connect(uiObject, SIGNAL(released()), this, SLOT(onRelease()));
}

View::~View() { uiObject->deleteLater(); }


void View::onPress()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    QString value = ""; //
    QString script = "$widgets[\"" + id + "\"].onPress({ target: { src: \"" + id + "\", value: \"" + value + "\" }})";
    // qDebug() << script;
    engine->runScript(script);
}

void View::onRelease()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    QString value = ""; //
    QString script = "$widgets[\"" + id + "\"].onRelease({ target: { src: \"" + id + "\", value: \"" + value + "\" }})";
    // qDebug() << script;
    engine->runScript(script);
}

bool View::update(QJsonObject json)
{
    applyStyle("QFrame", this, json);
    
    if (json.contains("hoverable")) {
        uiObject->hoverable = json["hoverable"].toBool();
    }
    if (json.contains("touchable")) {
        uiObject->touchable = json["touchable"].toBool();
    }
    
    if (uiObject->hoverable || uiObject->touchable) {
        uiObject->setFocusPolicy(Qt::StrongFocus);
    } else {
        uiObject->setFocusPolicy(Qt::NoFocus);
    }
    
    relayout();
    return true;
}

bool View::addChild(UIObject* obj)
{
    layout()->addWidget(obj->widget());
    relayout();
    return true;
};

void View::relayout()
{
    uiObject->setUpdatesEnabled(false);

    QBoxLayout* l = layout();
    for (int i = 0; i < l->count(); ++i) {
        QLayoutItem* layoutItem = l->itemAt(i);
        if (layoutItem->spacerItem()) {
            l->removeItem(layoutItem);
            delete layoutItem;
            --i;
            continue;
        }
        QWidget* w = layoutItem->widget();
        if (w) {
            int stretch = w->property("flex").toInt();
            l->setStretch(i, stretch);
        }
    }

    QString align = widget()->property("align-items").toString();
    QString justify = widget()->property("justify-content").toString();

    // re-order
    for (int j = 0; j < l->count(); ++j) {
        for (int i = 0; i < l->count(); ++i) {
            QLayoutItem* layoutItem = l->itemAt(i);
            QWidget* w = layoutItem->widget();
            if (w) {
                int order = w->property("order").toInt();
                if (order != -1) {
                    layout()->insertWidget(order, w);
                }
            }
        }
    }

    if (justify == "space-around" || justify == "space-between") {
        int c = l->count() - 1;
        for (int i = 0; i < c; i++) {
            l->insertStretch((i * 2) + 1, 1);
        }
    }

    if (align == "flex-start" || align == "center" || justify == "center" || justify == "space-around") {
        l->insertStretch(-1, 1);
    }
    if (align == "flex-end" || align == "center" || justify == "center" || justify == "space-around") {
        l->insertStretch(0, 1);
    }

    uiObject->setUpdatesEnabled(true);
    uiObject->update();
}

void View::addToJavaScriptWindowObject()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    engine->frame->addToJavaScriptWindowObject("$widgets_" + id.replace(':','_'), this);
}

//----------------------------
// ScrollView
//----------------------------
ScrollView::ScrollView()
    : uiObject(new QScrollArea)
{
    view = new QWidget();
    view->setLayout(new QVBoxLayout());
    view->layout()->setMargin(0);
    view->layout()->setSpacing(0);
    uiObject->setWidget(view);
    uiObject->setWidgetResizable(true);
}

ScrollView::~ScrollView() { uiObject->deleteLater(); }

bool ScrollView::update(QJsonObject json)
{
    applyStyle("QScrollArea", this, json);
    return true;
}

bool ScrollView::addChild(UIObject* obj)
{
    layout()->addWidget(obj->widget());
    return true;
};

void ScrollView::addToJavaScriptWindowObject()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    engine->frame->addToJavaScriptWindowObject("$widgets_" + id.replace(':','_'), this);
}

//----------------------------
// MenuBar
//----------------------------
MenuBar::MenuBar()
    : uiObject(new QMenuBar)
{
}

MenuBar::~MenuBar() { uiObject->deleteLater(); }

bool MenuBar::update(QJsonObject json)
{
    applyStyle("QMenuBar", this, json);
    return true;
}

bool MenuBar::addChild(UIObject* obj)
{
    Menu *menu = qobject_cast<Menu*>(obj);
    if (menu) {
        uiObject->addMenu(qobject_cast<QMenu*>(menu->widget()));
        return true;
    }
    
    MenuItem *item = qobject_cast<MenuItem*>(obj);
    if (item) {
        uiObject->addAction(item->action());
        return true;
    }
    
    return true;
};

void MenuBar::addToJavaScriptWindowObject()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    engine->frame->addToJavaScriptWindowObject("$widgets_" + id.replace(':','_'), this);
}


void MenuBar::setWidget(QWidget *w)
{
    uiObject->deleteLater();
    uiObject = (QMenuBar*)w;
}

//----------------------------
// Menu
//----------------------------
Menu::Menu()
    : uiObject(new QMenu)
{
}

Menu::~Menu() { uiObject->deleteLater(); }

bool Menu::update(QJsonObject json)
{
    applyStyle("QMenu", this, json);
    if (json.contains("text")) {
        QString newText = json.value("text").toString();
        if (newText != uiObject->title()) {
            uiObject->setTitle(newText);
        }
    }
    return true;
}

bool Menu::addChild(UIObject* obj)
{
    Menu *menu = qobject_cast<Menu*>(obj);
    if (menu) {
        uiObject->addMenu(qobject_cast<QMenu*>(menu->widget()));
        return true;
    }
    
    MenuItem *item = qobject_cast<MenuItem*>(obj);
    if (item) {
        uiObject->addAction(item->action());
        return true;
    }
    return true;
};

void Menu::addToJavaScriptWindowObject()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    engine->frame->addToJavaScriptWindowObject("$widgets_" + id.replace(':','_'), this);
}

//----------------------------
// MenuItem
//----------------------------
MenuItem::MenuItem()
    : uiObject(new QAction)
    , _widget(new QWidget)
{
    connect(uiObject, SIGNAL(triggered(bool)), this, SLOT(onTrigger(bool)));
}

MenuItem::~MenuItem() { uiObject->deleteLater(); _widget->deleteLater(); }

bool MenuItem::update(QJsonObject json)
{
    applyStyle("MenuItem", this, json);
    if (json.contains("text")) {
        QString newText = json.value("text").toString();
        if (newText != uiObject->text()) {
            uiObject->setText(newText);
        }
    }
    
    if (json.contains("icon")) {
        uiObject->setIcon(engine->icon(json.value("icon").toString()));
    }
    return true;
}

bool MenuItem::addChild(UIObject* obj)
{
    // uiObject->addWidget(obj->widget());
    return true;
};

void MenuItem::addToJavaScriptWindowObject()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    engine->frame->addToJavaScriptWindowObject("$widgets_" + id.replace(':','_'), this);
}

void MenuItem::onTrigger(bool checked)
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    QString script = "$widgets[\"" + id + "\"].onClick({ target: { src: \"" + id + "\", value: " + (checked ? "true" : "false") + " }})";
    // qDebug() << script;
    engine->runScript(script);
}

//----------------------------
// StatusBar
//----------------------------
StatusBar::StatusBar()
    : uiObject(new QStatusBar)
{
}

StatusBar::~StatusBar() { uiObject->deleteLater(); }

bool StatusBar::update(QJsonObject json)
{
    applyStyle("QStatusBar", this, json);
    relayout();
    return true;
}

bool StatusBar::addChild(UIObject* obj)
{
    uiObject->addPermanentWidget(obj->widget());
    relayout();
    return true;
};

void StatusBar::relayout()
{
    QList<QWidget*> widgets;
    for(auto c : uiObject->children()) {
        QWidget *w = qobject_cast<QWidget*>(c);
        if (w) {
            widgets.append(w);
        }
    }
    
    for(auto w : widgets) {
       if (w->property("permanent").toBool()) {
           uiObject->addPermanentWidget(w);
       } else {
           uiObject->addWidget(w);
       }
    }
}

void StatusBar::addToJavaScriptWindowObject()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    engine->frame->addToJavaScriptWindowObject("$widgets_" + id.replace(':','_'), this);
}

void StatusBar::showMessage(QString msg, int timeout)
{
    qDebug() << "status" << msg;
    uiObject->showMessage(msg, timeout);
}

void StatusBar::clearMessage()
{
    uiObject->clearMessage();
}

void StatusBar::setWidget(QWidget *w)
{
    uiObject->deleteLater();
    uiObject = (QStatusBar*)w;
}

//----------------------------
// SplitterView
//----------------------------
SplitterView::SplitterView()
    : uiObject(new QSplitter)
{
}

SplitterView::~SplitterView() { uiObject->deleteLater(); }

bool SplitterView::update(QJsonObject json)
{
    applyStyle("QSplitter", this, json);
    return true;
}

bool SplitterView::addChild(UIObject* obj)
{
    uiObject->addWidget(obj->widget());
    return true;
};

void SplitterView::addToJavaScriptWindowObject()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    engine->frame->addToJavaScriptWindowObject("$widgets_" + id.replace(':','_'), this);
}

//----------------------------
// StackedView
//----------------------------
StackedView::StackedView()
    : uiObject(new QStackedWidget)
{
}

StackedView::~StackedView() { uiObject->deleteLater(); }

bool StackedView::update(QJsonObject json)
{
    applyStyle("QStackedWidget", this, json);

    if (json.contains("current")) {
        QString current = json.value("current").toString();
        UIObject *obj = engine->findInRegistryById(current);
        if (obj) {
            uiObject->setCurrentWidget(obj->widget());
        }
    }
    return true;
}

bool StackedView::addChild(UIObject* obj)
{
    uiObject->addWidget(obj->widget());
    return true;
};

void StackedView::addToJavaScriptWindowObject()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    engine->frame->addToJavaScriptWindowObject("$widgets_" + id.replace(':','_'), this);
}

//----------------------------
// Text
//----------------------------
Text::Text()
    : uiObject(new QLabel)
{
    uiObject->setLayout(new QVBoxLayout());
    uiObject->setTextFormat(Qt::RichText);
    uiObject->setTextInteractionFlags(Qt::NoTextInteraction);
}

Text::~Text() { uiObject->deleteLater(); }

bool Text::update(QJsonObject json)
{
    if (json.contains("text")) {
        QString newText = json.value("text").toString();
        if (newText != uiObject->text()) {
            uiObject->setText(newText);
        }
    } else if (json.contains("renderedText")) {
        uiObject->setText(json.value("renderedText").toString());
    }
    applyStyle("QLabel", this, json);
    return true;
}

void Text::addToJavaScriptWindowObject()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    engine->frame->addToJavaScriptWindowObject("$widgets_" + id.replace(':','_'), this);
}

//----------------------------
// TextInput
//----------------------------
TextInput::TextInput()
    : uiObject(new QLineEdit)
{
    uiObject->setLayout(new QVBoxLayout());
    connect(uiObject, SIGNAL(textEdited(QString)), this, SLOT(onChange(QString)));
    connect(uiObject, SIGNAL(returnPressed()), this, SLOT(onSubmit()));
}

TextInput::~TextInput() { uiObject->deleteLater(); }

bool TextInput::update(QJsonObject json)
{
    applyStyle("QLineEdit", this, json);
    if (json.contains("text")) {
        QString newText = json.value("text").toString();
        if (newText != uiObject->text()) {
            uiObject->setText(newText);
        }
    }
    if (json.contains("placeholder")) {
        QString placeholder = json.value("placeholder").toString();
        if (placeholder != uiObject->placeholderText()) {
            uiObject->setPlaceholderText(placeholder);
        }
    }
    return true;
}

void TextInput::onChange(QString value)
{    
    if (!uiObject->isVisible()) {
        return;
    }
    
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    QString script = "$widgets[\"" + id + "\"].onChangeText({ target: { src: \"" + id + "\", value: \"" + value.replace('\\', "\\\\") + "\" }})";
    engine->runScript(script);
}

void TextInput::onSubmit()
{
    if (!uiObject->isVisible()) {
        return;
    }
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    QString value = ""; //
    QString script = "$widgets[\"" + id + "\"].onSubmitEditing({ target: { src: \"" + id + "\", value: \"" + value + "\" }})";
    engine->runScript(script);
}

void TextInput::addToJavaScriptWindowObject()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    engine->frame->addToJavaScriptWindowObject("$widgets_" + id.replace(':','_'), this);
}

void TextInput::focus()
{
    uiObject->setFocus(Qt::ActiveWindowFocusReason);
}

void TextInput::select()
{
    QTimer::singleShot(50, uiObject, &QLineEdit::selectAll);
}

void TextInput::setText(QString text)
{
    uiObject->setText(text);
}

//----------------------------
// Image
//----------------------------
Image::Image()
    : uiObject(new QLabel)
{
    uiObject->setLayout(new QVBoxLayout());
    uiObject->setTextFormat(Qt::RichText);
    uiObject->setTextInteractionFlags(Qt::NoTextInteraction);
    netman = new QNetworkAccessManager(this);
    connect(netman, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

Image::~Image() { uiObject->deleteLater(); }

bool Image::update(QJsonObject json)
{
    applyStyle("QLabel", this, json);
    if (json.contains("source")) {
        // if (engine->basePath.scheme() == "http") {
        QString imageSource = engine->basePath.toString() + json.value("source").toString();
        if (lastSource != imageSource) {
            netman->get(QNetworkRequest(QUrl(imageSource)));
            lastSource = imageSource;
        }
    }
    // qDebug() << "image";
    // qDebug() << uiObject->width();
    // qDebug() << uiObject->height();
    return true;
}

void Image::replyFinished(QNetworkReply* reply)
{
    if (reply->error()) {
        qDebug() << reply->errorString();
    } else {
        int w = uiObject->width();
        int h = uiObject->height();
        QImageReader imageReader(reply);
        imageReader.setAutoDetectImageFormat(true);
        image = imageReader.read();
        uiObject->setPixmap(QPixmap::fromImage(image).scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void Image::addToJavaScriptWindowObject()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    engine->frame->addToJavaScriptWindowObject("$widgets_" + id.replace(':','_'), this);
}

//----------------------------
// Button
//----------------------------
Button::Button()
    : uiObject(new QPushButton)
{
    uiObject->setLayout(new QHBoxLayout());
    connect(uiObject, SIGNAL(clicked(bool)), this, SLOT(onClick(bool)));
    connect(uiObject, SIGNAL(pressed()), this, SLOT(onPress()));
    connect(uiObject, SIGNAL(released()), this, SLOT(onRelease()));
}

Button::~Button() { uiObject->deleteLater(); }

bool Button::update(QJsonObject json)
{
    applyStyle("QPushButton", this, json);
    if (json.contains("text")) {
        QString newText = json.value("text").toString();
        if (newText != uiObject->text()) {
            uiObject->setText(newText);
        }
    }
    if (json.contains("checked")) {
        uiObject->setChecked(json.value("checked").toBool());
    }
    if (json.contains("checkable")) {
        uiObject->setCheckable(json.value("checkable").toBool());
    }
    if (json.contains("icon")) {
        uiObject->setIcon(engine->icon(json.value("icon").toString()));
    }
    if (json.contains("style")) {
        QJsonObject style = json.value("style").toObject();
        if (style.contains("icon-width") && style.contains("icon-height")) {
            QSize sz(style.value("icon-width").toInt(), style.value("icon-height").toInt());
            uiObject->setIconSize(sz);
        } 
    }
    return true;
}

void Button::onClick(bool checked)
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    QString script = "$widgets[\"" + id + "\"].onClick({ target: { src: \"" + id + "\", value: " + (checked ? "true" : "false") + " }})";
    // qDebug() << script;
    engine->runScript(script);
}

void Button::onPress()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    QString value = ""; //
    QString script = "$widgets[\"" + id + "\"].onPress({ target: { src: \"" + id + "\", value: \"" + value + "\" }})";
    // qDebug() << script;
    engine->runScript(script);
}

void Button::onRelease()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    QString value = ""; //
    QString script = "$widgets[\"" + id + "\"].onRelease({ target: { src: \"" + id + "\", value: \"" + value + "\" }})";
    // qDebug() << script;
    engine->runScript(script);
}

void Button::addToJavaScriptWindowObject()
{
    QString id = property("id").toString();
    if (id.isEmpty()) {
        return;
    }
    engine->frame->addToJavaScriptWindowObject("$widgets_" + id.replace(':','_'), this);
}

//----------------------------
// Core factory
//----------------------------
UIObject* UICoreFactory::create(QJsonObject json)
{
    QString type = json.value("type").toString();

    // qDebug() << json;
    // qDebug() << "create" << type;

    BEGIN_UI_DEF(Window)
    uiObject->widget()->show();
    END_UI()

    BEGIN_UI_DEF(View)
    END_UI()

    BEGIN_UI_DEF(Image)
    END_UI()

    BEGIN_UI_DEF(ScrollView)
    END_UI()

    BEGIN_UI_DEF(SplitterView)
    END_UI()

    BEGIN_UI_DEF(StackedView)
    END_UI()

    BEGIN_UI_DEF(Text)
    END_UI()

    BEGIN_UI_DEF(TextInput)
    END_UI()

    BEGIN_UI_DEF(Button)
    END_UI()
    
    BEGIN_UI_DEF(StatusBar)
    END_UI()
    
    BEGIN_UI_DEF(MenuBar)
    END_UI()
        
    BEGIN_UI_DEF(Menu)
    END_UI()
        
    BEGIN_UI_DEF(MenuItem)
    END_UI()

    return NULL;
}
