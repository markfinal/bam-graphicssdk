#include "windowlibrary/graphicswindow.h"

#include <memory>

#import <Cocoa/Cocoa.h>

int
main()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];

    std::unique_ptr<WindowLibrary::GraphicsWindow> window(new WindowLibrary::GraphicsWindow);
    window->init(512, 512, "Metal Example");

    [NSApp run];

    [pool drain];
    return 0;
}
