/*
Copyright (c) 2010-2018, Mark Final
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
#include "windowlibrary/exception.h"
#include "macoswinlibimpl.h"

@interface WindowDelegate : NSObject<NSWindowDelegate>
{
}
@property WindowLibrary::GraphicsWindow *Window;
- (id)initWithWindow:(WindowLibrary::GraphicsWindow*)window;
@end

@implementation WindowDelegate : NSObject
- (id)initWithWindow:(WindowLibrary::GraphicsWindow*)window
{
    [self setWindow:window];
    return self;
}

- (BOOL)windowShouldClose:(id)sender
{
    (void)sender;
    [self Window]->onClose();
    [[NSApplication sharedApplication] stop:self];
    return YES;
}
@end

namespace WindowLibrary
{

GraphicsWindow::Impl::Impl(
    GraphicsWindow *inParent)
    :
    _parent(inParent)
{}

GraphicsWindow::Impl::~Impl()
{
    this->destroyWindow();
}

void
GraphicsWindow::Impl::createWindow(
    const uint32_t inWidth,
    const uint32_t inHeight,
    const std::string &inTitle)
{
    NSUInteger windowStyle = NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask;

    NSRect windowRect = NSMakeRect(0, 0, inWidth, inHeight);
    NSWindow *window = [[NSWindow alloc] initWithContentRect:windowRect
                                                    styleMask:windowStyle
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO];
    [window center];
    [window setTitle:[NSString stringWithCString:inTitle.c_str() encoding:[NSString defaultCStringEncoding]]];
    this->_window = window;
    this->_width = inWidth;
    this->_height = inHeight;

    auto wndDelegate = [[WindowDelegate alloc] initWithWindow:this->_parent];
    [window setDelegate: wndDelegate];

    this->_parent->onCreate();
}

void
GraphicsWindow::Impl::show()
{
    [this->_window orderFrontRegardless];
}

void
GraphicsWindow::Impl::destroyWindow()
{
    // TODO
}

} // namespace WindowLibrary
