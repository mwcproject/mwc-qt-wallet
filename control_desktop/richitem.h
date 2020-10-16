#ifndef RICHITEM_H
#define RICHITEM_H

#include <QFrame>
#include <QLabel>

namespace control {

class RichVBox;

const QString LEFT_MARK_ON = "background: #BCF317; border: transparent;";
const QString LEFT_MARK_OFF = "background: transparent; border: transparent;";
const int     LEFT_MARK_SIZE = 3;
const int     LEFT_MARK_SPACING = 10;
const QString LOW_LIGHT_COLOR = "#BF84FF";
const int     VBOX_SPACING = 10;

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
    RichItem & setText(const QString & text);
    RichItem & addSpacer();

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

QLabel *  crateLable( control::RichItem * parent, bool wordwrap, bool lowLight, QString text );

}

#endif // RICHITEM_H
