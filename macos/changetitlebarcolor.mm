#include <QWidget>
#include <Cocoa/Cocoa.h>

namespace Cocoa
{
void changeTitleBarColor(WId winId, double red, double green, double blue)
{
    if (winId == 0) return;
    NSView* view = (NSView*)winId;
    NSWindow* window = [view window];
    window.titlebarAppearsTransparent = YES;
    window.backgroundColor = [NSColor colorWithRed:red green:green blue:blue alpha:1.];
    window.appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantDark];
}

}
