#ifndef RICHITEM_H
#define RICHITEM_H

#include <QFrame>
#include <QLabel>

namespace control {

class RichVBox;

const QString LEFT_MARK_ON = "background: #BCF317; border: transparent;";
const QString LEFT_MARK_OFF = "background: transparent; border: transparent;";
const QString HORZ_LINE = "background: rgba(255, 255, 255, 0.5); border: transparent;";  // Same as Resorces has
const int     LEFT_MARK_SIZE = 3;
const int     LEFT_MARK_SPACING = 10;
const QString LOW_LIGHT_COLOR = "#BF84FF";
const int     VBOX_SPACING = 5;
const int     ROW_HEIGHT = 16; // use it for non text elements like icons

const int     FONT_NORMAL = 14;
const int     FONT_LARGE  = 16;
const int     FONT_SMALL  = 12;

// Rich Item should provide ability to dynamically construct complex list items that allow to show information better then traditional lists
// with columns
class RichItem : public QFrame
{
    Q_OBJECT
public:
    explicit RichItem(QString id, QWidget *parent);

    // Here is the builders.

    // Create New layouts
    RichItem & hbox();
    RichItem & vbox();
    RichItem & pop(); // Pop layout back
    RichItem & setSpacing(int spacing); // Spacing for layout
    RichItem & setContentsMargins(int left, int top, int right, int bottom); // Layour margins

    RichItem & addWidget( QWidget * widget );
    RichItem & setWidgetStyleSheet(QString styleSheet);
    RichItem & setFixedWidth(int cx);
    RichItem & setFixedHeight(int cy);
    RichItem & setMinWidth(int cx);
    RichItem & setText(const QString & text);
    RichItem & addSpacer();
    RichItem & addFixedHSpacer(int cx);

    QWidget * getCurrentWidget() const {return curWidget;}
    QLayout * getCurrentLayout() const { Q_ASSERT(!layoutStack.isEmpty()); return layoutStack.last(); }

    RichItem & apply(); // last step, first item form activeLayoutStack will be applyed

    ///////////////////////////////
    void setParent(RichVBox * p) {eventsHandler=p;}

    void setFocusState(bool focus);
signals:

protected:
    virtual void focusInEvent(QFocusEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
private:
    QString id;
    QVector< QLayout * > layoutStack;
    QWidget * curWidget = nullptr;

    RichVBox * eventsHandler;
};

// Helpers, Control builder

// Create Mark + v layout that ready to accept the rows
RichItem * createMarkedItem(QString itemId, QWidget *parent, bool marked );

// Create a lable with a text
QLabel *  createLabel( control::RichItem * parent, bool wordwrap, bool lowLight, QString text, int fontSize = FONT_NORMAL );

// Create a lable with an icon
QLabel *  createIcon( control::RichItem * parent, QString pixmapPath, int cx, int cy );

QLabel * createHorzLine(control::RichItem * parent);

}

#endif // RICHITEM_H
