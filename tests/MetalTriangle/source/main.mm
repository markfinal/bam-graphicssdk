#include "windowlibrary/graphicswindow.h"

#include <memory>

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>

/* ---------------------------------------------------------------------- */

@interface MetalViewController : NSViewController
{}
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
    /*
    // https://www.haroldserrano.com/blog/getting-started-with-metal-api (iOS)
    // https://developer.apple.com/documentation/metal/hello_triangle
    auto mtlDevice = MTLCreateSystemDefaultDevice();
    auto metalLayer = [CAMetalLayer layer];
    metalLayer.device = mtlDevice;
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metalLayer.frame = self.view.bounds;
    [self.view.layer addSublayer:metalLayer];

    auto mtlCommandQueue = [mtlDevice newCommandQueue];
    auto mtlCommandBuffer = [mtlCommandQueue commandBuffer];

    auto renderPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
    auto mtlRenderEncoder = [mtlCommandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

    auto mtlLibrary = [mtlDevice newDefaultLibrary];
    auto vertexProgram = [mtlLibrary newFunctionWithName:@"clip_space_colour_vertex_function"];
    auto fragmentProgram = [mtlLibrary newFunctionWithName:@"pass_through_colour_fragment_function"];

    auto mtlRenderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    [mtlRenderPipelineDescriptor setVertexFunction:vertexProgram];
    [mtlRenderPipelineDescriptor setFragmentFunction:fragmentProgram];
    mtlRenderPipelineDescriptor.colorAttachments[0].pixelFormat=MTLPixelFormatBGRA8Unorm;

    auto renderPipelineState = [mtlDevice newRenderPipelineStateWithDescriptor:mtlRenderPipelineDescriptor error:nil];
    [mtlRenderEncoder setRenderPipelineState:renderPipelineState];

    static float quadVertexData[] =
    {
        0.5, -0.5, 0.0, 1.0,
        -0.5, -0.5, 0.0, 1.0,
        -0.5,  0.5, 0.0, 1.0,

        0.5,  0.5, 0.0, 1.0,
        0.5, -0.5, 0.0, 1.0,
        -0.5,  0.5, 0.0, 1.0
    };

    [mtlRenderEncoder setVertexBytes:quadVertexData
                    length:sizeof(quadVertexData)
                    atIndex:0];

    [mtlRenderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                      vertexStart:0
                      vertexCount:3];

    //auto vertexBuffer = [mtlDevice newBufferWithBytes:quadVertexData length:sizeof(quadVertexData) options:MTLResourceOptionCPUCacheModeDefault];
    //(void)vertexBuffer;

    //auto displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(renderScene)];

    //[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    */
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

@interface MetalViewDelegate : NSViewController<MTKViewDelegate>
- (void)drawInMTKView:(nonnull MTKView *)view;
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size;
- (void)encodeWithCoder:(nonnull NSCoder *)aCoder;
@end

/* ---------------------------------------------------------------------- */

@implementation MetalViewDelegate : NSViewController
- (void)drawInMTKView:(nonnull MTKView *)view
{
    NSLog((@"%s [Line %d] "), __PRETTY_FUNCTION__, __LINE__);
    (void)view;
}
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    NSLog((@"%s [Line %d] "), __PRETTY_FUNCTION__, __LINE__);
    (void)view;
    (void)size;
}
- (void)encodeWithCoder:(nonnull NSCoder *)aCoder
{
    NSLog((@"%s [Line %d] "), __PRETTY_FUNCTION__, __LINE__);
    (void)aCoder;
}
@end

/* ---------------------------------------------------------------------- */

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
