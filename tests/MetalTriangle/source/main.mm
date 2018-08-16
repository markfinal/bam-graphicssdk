#include "windowlibrary/graphicswindow.h"

#include <memory>

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>

/* ---------------------------------------------------------------------- */

@interface MetalViewController : NSViewController<MTKViewDelegate>
{
    id<MTLDevice> _device;
    id<MTLLibrary> _library;
    id<MTLCommandQueue> _cmdQueue;
    CAMetalLayer *_metalLayer;
}
-(void)configureMetal;
-(void)renderScene;
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
    // https://www.haroldserrano.com/blog/getting-started-with-metal-api (iOS)
    // https://developer.apple.com/documentation/metal/hello_triangle
    self->_device = MTLCreateSystemDefaultDevice();
    self->_library = [self->_device newDefaultLibrary];
    self->_cmdQueue = [self->_device newCommandQueue];

    self->_metalLayer = [CAMetalLayer layer];
    self->_metalLayer.device = self->_device;
    self->_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    self->_metalLayer.frame = self.view.bounds;
    [self.view.layer addSublayer:self->_metalLayer];
}
-(void)renderScene
{
    auto drawable = [self->_metalLayer nextDrawable];
    auto texture = drawable.texture;

    auto passDescriptor = [[MTLRenderPassDescriptor alloc] init];
    passDescriptor.colorAttachments[0].texture = texture;
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(1.0, 0.0, 0.0, 1.0);
    passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
    passDescriptor.depthAttachment.clearDepth = 1.0f;

    auto cmdBuffer = [self->_cmdQueue commandBuffer];
    auto cmdEncoder =
    [cmdBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    [cmdEncoder endEncoding];

    [cmdBuffer presentDrawable:drawable];
    [cmdBuffer commit];
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

/* -- MTKViewDelegate -- */

- (void)drawInMTKView:(nonnull MTKView *)view
{
    //NSLog((@"%s [Line %d] "), __PRETTY_FUNCTION__, __LINE__);
    (void)view;
    [self renderScene];
}
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    NSLog((@"%s [Line %d] "), __PRETTY_FUNCTION__, __LINE__);
    (void)view;
    (void)size;
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
    metalWindow->init(512, 512, "Metal Example");

    auto metal_view = [[MTKView alloc] initWithFrame:NSMakeRect(0, 0, metalWindow->width(), metalWindow->height())];
    auto view_controller = [[MetalViewController alloc] init];

    view_controller.view = metal_view;
    metal_view.delegate = view_controller;

    [[metalWindow->getNativeWindowHandle() contentView] addSubview:metal_view];

    metalWindow->show();

    /* -- go... -- */
    [NSApp run];

    [pool drain];
    return 0;
}
