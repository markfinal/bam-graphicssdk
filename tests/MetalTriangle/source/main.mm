#include "windowlibrary/graphicswindow.h"

#include <memory>

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>

@interface MetalViewController : NSViewController
{}
-(void)viewDidAppear;
@end

@implementation MetalViewController : NSViewController
-(void)viewDidAppear
{
    // https://www.haroldserrano.com/blog/getting-started-with-metal-api
    auto mtlDevice = MTLCreateSystemDefaultDevice();
    auto metalLayer = [CAMetalLayer layer];
    metalLayer.device = mtlDevice;
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metalLayer.frame = self.view.bounds;
    [self.view.layer addSublayer:metalLayer];

    auto mtlLibrary = [mtlDevice newDefaultLibrary];
    auto vertexProgram = [mtlLibrary newFunctionWithName:@"vertexShader"];
    auto fragmentProgram = [mtlLibrary newFunctionWithName:@"fragmentShader"];

    auto mtlRenderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    [mtlRenderPipelineDescriptor setVertexFunction:vertexProgram];
    [mtlRenderPipelineDescriptor setFragmentFunction:fragmentProgram];

    mtlRenderPipelineDescriptor.colorAttachments[0].pixelFormat=MTLPixelFormatBGRA8Unorm;

    auto renderPipelineState = [mtlDevice newRenderPipelineStateWithDescriptor:mtlRenderPipelineDescriptor error:nil];
    (void)renderPipelineState;

    static float quadVertexData[] =
    {
        0.5, -0.5, 0.0, 1.0,
        -0.5, -0.5, 0.0, 1.0,
        -0.5,  0.5, 0.0, 1.0,

        0.5,  0.5, 0.0, 1.0,
        0.5, -0.5, 0.0, 1.0,
        -0.5,  0.5, 0.0, 1.0
    };

    auto vertexBuffer = [mtlDevice newBufferWithBytes:quadVertexData length:sizeof(quadVertexData) options:MTLResourceOptionCPUCacheModeDefault];
    (void)vertexBuffer;

    //auto displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(renderScene)];

    //[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];

    auto mtlCommandQueue = [mtlDevice newCommandQueue];
    (void)mtlCommandQueue;
}
@end

@interface MetalViewDelegate : NSViewController<MTKViewDelegate>
- (void)drawInMTKView:(nonnull MTKView *)view;
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size;
- (void)encodeWithCoder:(nonnull NSCoder *)aCoder;
@end

@implementation MetalViewDelegate : NSViewController
- (void)drawInMTKView:(nonnull MTKView *)view
{
    (void)view;
}
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    (void)view;
    (void)size;
}
- (void)encodeWithCoder:(nonnull NSCoder *)aCoder
{
    (void)aCoder;
}
@end

int
main()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];

    std::unique_ptr<WindowLibrary::GraphicsWindow> metalWindow(new WindowLibrary::GraphicsWindow);
    metalWindow->init(512, 512, "Metal Example");

    auto frame = NSMakeRect(0, 0, metalWindow->width(), metalWindow->height());
    auto metal_view = [[MTKView alloc] initWithFrame:frame];
    metal_view.delegate = [[MetalViewDelegate alloc] init];
    [[metalWindow->getNativeWindowHandle() contentView] addSubview:metal_view];

    auto view_controller = [[MetalViewController alloc] init];
    view_controller.view = metal_view;

    metalWindow->show();

    [NSApp run];

    [pool drain];
    return 0;
}
