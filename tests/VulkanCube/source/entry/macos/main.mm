#include "renderer/renderer.h"
#include "appwindow.h"

#include "windowlibrary/graphicswindow.h"

#include <memory>

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>

/* ---------------------------------------------------------------------- */

std::unique_ptr<AppWindow> metalWindow;
std::unique_ptr<Renderer> renderer;

/* ---------------------------------------------------------------------- */

@interface MetalViewController : NSViewController
{
    CVDisplayLinkRef _displayLink;
}
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

    metalWindow->macosSetViewHandle([self view]);

    renderer.reset(new Renderer(metalWindow.get()));
    renderer->init();

    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, NULL);
    CVDisplayLinkStart(_displayLink);
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
    CVDisplayLinkStop(_displayLink);
}
-(void)viewDidDisappear
{
    [super viewDidDisappear];
    NSLog((@"%s [Line %d] "), __PRETTY_FUNCTION__, __LINE__);
}

static CVReturn
DisplayLinkCallback(
    CVDisplayLinkRef displayLink,
    const CVTimeStamp* now,
    const CVTimeStamp* outputTime,
    CVOptionFlags flagsIn,
    CVOptionFlags* flagsOut,
    void* target)
{
    (void)displayLink;
    (void)now;
    (void)outputTime;
    (void)flagsIn;
    (void)flagsOut;
    (void)target;
    renderer->draw_frame();
    return kCVReturnSuccess;
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
    metalWindow.reset(new AppWindow);
    metalWindow->init(512, 512, "Vulkan Cube Example");

    auto metal_view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, metalWindow->width(), metalWindow->height())];
    if (![metal_view.layer isKindOfClass:[CAMetalLayer class]])
    {
        [metal_view setLayer:[CAMetalLayer layer]];
        [metal_view setWantsLayer:YES];
    }

    auto view_controller = [[MetalViewController alloc] init];
    view_controller.view = metal_view;

    [[metalWindow->getNativeWindowHandle() contentView] addSubview:metal_view];

    metalWindow->show();

    /* -- go... -- */
    [NSApp run];

    [pool drain];
    return 0;
}
