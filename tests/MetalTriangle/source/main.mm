#include "windowlibrary/graphicswindow.h"

#import <Cocoa/Cocoa.h>

int
main()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    [pool drain];
    return 0;
}
