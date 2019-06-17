/*
Copyright (c) 2010-2019, Mark Final
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of BuildAMation nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "windowlibrary/graphicswindow.h"

#include <memory>

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>

/* ---------------------------------------------------------------------- */

struct MBEVertex
{
    vector_float4 position;
    vector_float4 colour;
};

typedef uint16_t MBEIndex;
const MTLIndexType MBEIndexType = MTLIndexTypeUInt16;

/* ---------------------------------------------------------------------- */

@interface MetalViewController : NSViewController<MTKViewDelegate>
{
    id<MTLDevice> _device;
    id<MTLLibrary> _library;
    id<MTLCommandQueue> _cmdQueue;
    CAMetalLayer *_metalLayer;
    id<MTLDepthStencilState> _depthStencilState;
    id<MTLRenderPipelineState> _renderPipelineState;
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
    self->_metalLayer.frame = NSRectToCGRect(self.view.bounds);
    [self.view.layer addSublayer:self->_metalLayer];

    auto pipelineDescriptor = [MTLRenderPipelineDescriptor new];
    pipelineDescriptor.vertexFunction = [self->_library newFunctionWithName:@"clip_space_colour_vertex_function"];
    pipelineDescriptor.fragmentFunction = [self->_library newFunctionWithName:@"pass_through_colour_fragment_function"];
    pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    //pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

    auto depthStencilDescriptor = [MTLDepthStencilDescriptor new];
    depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLess;
    depthStencilDescriptor.depthWriteEnabled = YES;
    self->_depthStencilState = [self->_device newDepthStencilStateWithDescriptor:depthStencilDescriptor];

    NSError *error = nil;
    self->_renderPipelineState = [self->_device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
}
-(void)renderScene
{
    auto drawable = [self->_metalLayer nextDrawable];
    auto texture = drawable.texture;

    auto passDescriptor = [[MTLRenderPassDescriptor alloc] init];
    passDescriptor.colorAttachments[0].texture = texture;
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(1.0, 1.0, 1.0, 1.0);

    static const MBEVertex vertices[] =
    {
        { { -0.5f, -0.5f, 0, 1 }, {1,0,0,1}, },
        { { +0.5f, -0.5f, 0, 1 }, {0,1,0,1}, },
        { { +0.5f, +0.5f, 0, 1 }, {0,0,1,1} }
    };

    static const MBEIndex indices[] =
    {
        0, 1, 2
    };

    auto _vertexBuffer = [self->_device newBufferWithBytes:vertices
                                        length:sizeof(vertices)
                                        options:MTLResourceOptionCPUCacheModeDefault];
    [_vertexBuffer setLabel:@"Vertices"];

    auto _indexBuffer = [self->_device newBufferWithBytes:indices
                                       length:sizeof(indices)
                                       options:MTLResourceOptionCPUCacheModeDefault];
    [_indexBuffer setLabel:@"Indices"];

    auto cmdBuffer = [self->_cmdQueue commandBuffer];
    auto cmdEncoder = [cmdBuffer renderCommandEncoderWithDescriptor:passDescriptor];

    [cmdEncoder setRenderPipelineState:self->_renderPipelineState];
    //[cmdEncoder setDepthStencilState:self->_depthStencilState];
    [cmdEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [cmdEncoder setCullMode:MTLCullModeBack];

    [cmdEncoder setVertexBuffer:_vertexBuffer offset:0 atIndex:0];

    [cmdEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                indexCount:[_indexBuffer length] / sizeof(MBEIndex)
                indexType:MBEIndexType
                indexBuffer:_indexBuffer
                indexBufferOffset:0];

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
    metalWindow->init(512, 512, "Metal Triangle");

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
