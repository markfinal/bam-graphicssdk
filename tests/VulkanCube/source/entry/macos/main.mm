#include "windowlibrary/graphicswindow.h"

#include <memory>

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>

/* ---------------------------------------------------------------------- */

@interface MetalViewController : NSViewController
{
    CAMetalLayer *_metalLayer;
}
-(void)configureMetal;
-(void)viewDidLoad;
-(void)viewWillAppear;
-(void)viewDidAppear;
-(void)updateViewConstraints;
-(void)viewWillLayout;
-(void)viewDidLayout;
-(void)viewWillDisappear;
-(void)viewDidDisappear;
@end

/* ---------------------------------------------------------------------- */

@implementation MetalViewController : NSViewController
-(void)configureMetal
{
    self->_metalLayer = [CAMetalLayer layer];
}
-(void)viewDidLoad
{
    [super viewDidLoad];
    NSLog((@"%s [Line %d] "), __PRETTY_FUNCTION__, __LINE__);
}
-(void)viewWillAppear
{
    [super viewWillAppear];
    NSLog((@"%s [Line %d] "), __PRETTY_FUNCTION__, __LINE__);
}
-(void)viewDidAppear
{
    [super viewDidAppear];
    NSLog((@"%s [Line %d] "), __PRETTY_FUNCTION__, __LINE__);
    [self configureMetal];
}
-(void)updateViewConstraints
{
    [super updateViewConstraints];
    NSLog((@"%s [Line %d] "), __PRETTY_FUNCTION__, __LINE__);
}
-(void)viewWillLayout
{
    [super viewWillLayout];
    NSLog((@"%s [Line %d] "), __PRETTY_FUNCTION__, __LINE__);
}
-(void)viewDidLayout
{
    [super viewDidLayout];
    NSLog((@"%s [Line %d] "), __PRETTY_FUNCTION__, __LINE__);
}
-(void)viewWillDisappear
{
    [super viewWillDisappear];
    NSLog((@"%s [Line %d] "), __PRETTY_FUNCTION__, __LINE__);
}
-(void)viewDidDisappear
{
    [super viewDidDisappear];
    NSLog((@"%s [Line %d] "), __PRETTY_FUNCTION__, __LINE__);
}
@end

/* ---------------------------------------------------------------------- */

int
main()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSLog(@"%@", [[NSProcessInfo processInfo] arguments]);

    [NSApplication sharedApplication];

    /* -- add a menu bar with a Quit option -- */
    id menubar = [[NSMenu new] autorelease];
    id appMenuItem = [[NSMenuItem new] autorelease];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];
    id appMenu = [[NSMenu new] autorelease];
    id appName = [[NSProcessInfo processInfo] processName];
    id quitTitle = [@"Quit " stringByAppendingString:appName];
    id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];

    /* -- add a Metal view -- */
    std::unique_ptr<WindowLibrary::GraphicsWindow> metalWindow(new WindowLibrary::GraphicsWindow);
    metalWindow->init(512, 512, "Vulkan Cube Example");

    auto metal_view = [[MTKView alloc] initWithFrame:NSMakeRect(0, 0, metalWindow->width(), metalWindow->height())];
    auto view_controller = [[MetalViewController alloc] init];

    view_controller.view = metal_view;

    [[metalWindow->getNativeWindowHandle() contentView] addSubview:metal_view];

    metalWindow->show();

    /* -- go... -- */
    [NSApp run];

    [pool drain];
    return 0;
}
