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
#include "renderer.h"
#include "errorhandler.h"
#include "windowlibrary/glcontext.h"

#include <GL/glew.h>
#if defined(D_BAM_PLATFORM_OSX)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <string>

#if defined(D_BAM_PLATFORM_WINDOWS)
#include <Windows.h>
#include <process.h>
#endif

//#define USE_UNIFORM_BUFFER

#define GLFN(_call) _call; CheckForGLErrors(__FILE__, __LINE__, true)
#define GLFN_DONOTBREAK(_call) _call; CheckForGLErrors(__FILE__, __LINE__, false)

void CheckForGLErrors(const char *file, int line, bool breakOnError)
{
    GLenum error = glGetError();
    if (GL_NO_ERROR != error)
    {
        switch (error)
        {
        case GL_INVALID_ENUM:
            ErrorHandler::Report(file, line, "GL invalid enum");
            break;

        case GL_INVALID_VALUE:
            ErrorHandler::Report(file, line, "GL invalid value");
            break;

        case GL_INVALID_OPERATION:
            ErrorHandler::Report(file, line, "GL invalid operation");
            break;

        case GL_STACK_OVERFLOW:
            ErrorHandler::Report(file, line, "GL stack overflow");
            break;

        case GL_STACK_UNDERFLOW:
            ErrorHandler::Report(file, line, "GL stack underflow");
            break;

        case GL_OUT_OF_MEMORY:
            ErrorHandler::Report(file, line, "GL out of memory");
            break;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            ErrorHandler::Report(file, line, "Invalid framebuffer operation");
            break;

        default:
            ErrorHandler::Report(file, line, "Unrecognized GL error 0x%x", error);
        }

        if (breakOnError)
        {
#if defined(D_BAM_PLATFORM_WINDOWS)
            ::DebugBreak();
#endif
        }
    }
}

void CheckForFrameBufferErrors(const GLenum status, const char *file, int line, bool breakOnError)
{
    CheckForGLErrors(file, line, breakOnError);
    if (GL_FRAMEBUFFER_COMPLETE == status)
    {
        return;
    }
    switch (status)
    {
        case GL_FRAMEBUFFER_UNDEFINED:
            ErrorHandler::Report(file, line, "Default framebuffer does not exist");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            ErrorHandler::Report(file, line, "Incomplete framebuffer attachments");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            ErrorHandler::Report(file, line, "Need at least one attachment to the framebuffer");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            ErrorHandler::Report(file, line, "Incomplete drawbuffer");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            ErrorHandler::Report(file, line, "Incomplete readbuffer");
            break;

        case GL_FRAMEBUFFER_UNSUPPORTED:
            ErrorHandler::Report(file, line, "Framebuffer formats not supported");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            ErrorHandler::Report(file, line, "Inconsistent multisample for attachments");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            ErrorHandler::Report(file, line, "Framebuffer layers incomplete");
            break;

        default:
            ErrorHandler::Report(file, line, "Unrecognized GL framebuffer status 0x%x",  status);
    }

    if (breakOnError)
    {
#if defined(D_BAM_PLATFORM_WINDOWS)
        ::DebugBreak();
#endif
    }
}

Renderer::Renderer(
    WindowLibrary::GraphicsWindow *inWindow)
    :
    _window(inWindow),
    _glContext(new WindowLibrary::GLContext(inWindow)),
    _thread(nullptr),
    mhVertexShader(0),
    mhFragmentShader(0),
    mhProgram(0),
    miTimerQuery(0),
    mbQuitFlag(false)
{}

Renderer::~Renderer() = default;

void *Renderer::operator new(size_t size)
{
    void *memory = ::malloc(size);
    return memory;
}

void Renderer::operator delete(void *object)
{
    ::free(object);
}

void Renderer::Initialize()
{
    this->_glContext->init();

    // create a thread for OpenGL rendering
    this->_thread.reset(new std::thread(threadFunction, this));
}

void Renderer::Release()
{
    this->Exit();
    this->_glContext.reset();
}

void Renderer::InitializeGLEW()
{
    ::GLenum result = ::glewInit();
    if (GLEW_OK != result)
    {
        REPORTERROR("GLEW was not initialized");
        return;
    }
}

void Renderer::ReleaseGLEW()
{
}

void Renderer::Exit()
{
    if (nullptr == this->_thread)
    {
        return;
    }
    mbQuitFlag = true;
    REPORTERROR("Request thread shutdown");
    this->_thread->join();
    this->_thread.reset();
}

// route to the worker thread
void Renderer::threadFunction(Renderer *inRenderer)
{
    inRenderer->runThread();
}

// rendering loop
void Renderer::runThread()
{
    this->_glContext->makeCurrent();

    const GLubyte *lacVendor = GLFN(::glGetString(GL_VENDOR));
    const GLubyte *lacRenderer = GLFN(::glGetString(GL_RENDERER));
    const GLubyte *lacVersion = GLFN(::glGetString(GL_VERSION));
    const GLubyte *lacGLSLVersion = GLFN_DONOTBREAK(::glGetString(GL_SHADING_LANGUAGE_VERSION));
    REPORTERROR1("GL_VENDOR   = '%s'", lacVendor);
    REPORTERROR1("GL_RENDERER = '%s'", lacRenderer);
    REPORTERROR1("GL_VERSION  = '%s'", lacVersion);
    REPORTERROR1("GL_SHADING_LANGUAGE_VERSION = '%s'", lacGLSLVersion);

    bool lbHasGLSL = (0 != lacGLSLVersion);

    this->InitializeGLEW();

    ::CheckForFrameBufferErrors(
        glCheckFramebufferStatus(GL_FRAMEBUFFER),
        __FILE__,
        __LINE__,
        true
    );

    this->CreateTimerQuery();

    // TODO: what is the best way to do this?
    if (lbHasGLSL && GLEW_ARB_vertex_program && GLEW_ARB_fragment_program)
    {
        this->CreateShaders();
#ifdef USE_UNIFORM_BUFFER
        this->SetupFragmentUniformBuffer();
#endif
    }

    // rendering loop
    while(!this->mbQuitFlag)
    {
        std::this_thread::yield();

        this->StartTimerQuery();

        GLFN(::glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
        GLFN(::glClear(GL_COLOR_BUFFER_BIT));

        GLFN(::glMatrixMode(GL_PROJECTION));
        GLFN(::glLoadIdentity());

        GLFN(::glMatrixMode(GL_MODELVIEW));
        GLFN(::glLoadIdentity());

        GLFN(::glViewport(0, 0, this->_window->width(), this->_window->height()));

        GLFN(::glDisable(GL_CULL_FACE));
        GLFN(::glDisable(GL_DEPTH_TEST));
        GLFN(::glDepthMask(GL_FALSE));
        GLFN(::glDisable(GL_LIGHTING));
        GLFN(::glDisable(GL_BLEND));

        ::glBegin(GL_TRIANGLES);
            ::glColor3f(1.0f, 0, 0);
            ::glVertex2f(-0.5f, -0.5f);
            ::glColor3f(0, 1.0f, 0);
            ::glVertex2f(0.5f, -0.5f);
            ::glColor3f(0, 0, 1.0f);
            ::glVertex2f(0.5f, 0.5f);
        GLFN(::glEnd());

        this->EndTimerQuery();

        if (GLEW_ARB_timer_query)
        {
            // Note: this involves a synchronous wait
            GLuint64 lui64TimeElapsed = this->GetTimeElapsed(); // this is in nanoseconds
            REPORTERROR1("GPU time = %.4f ms", lui64TimeElapsed / 1024.0f / 1024.f);
        }

        this->_glContext->swapBuffers();
    }

    REPORTERROR("Begin shutting down render thread");

    if (lbHasGLSL && GLEW_ARB_vertex_program && GLEW_ARB_fragment_program)
    {
        this->DestroyShaders();
    }

    this->DestroyTimerQuery();
    this->ReleaseGLEW();

    // terminate rendering thread
    this->_glContext->detachCurrent();

    REPORTERROR("Render thread has terminated");
}

void Renderer::CreateShaders()
{
    GLhandleARB v = GLFN(::glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB));
    GLhandleARB f = GLFN(::glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB));

    std::string vertexSource;
    vertexSource += "varying vec4 color;\n";
    vertexSource += "void main(void)\n";
    vertexSource += "{\n";
    vertexSource += "\tgl_Position = ftransform();\n";
    vertexSource += "\tcolor = gl_Color;\n";
    vertexSource += "}\n";
    const char *vertexSourceC = vertexSource.c_str();
    GLFN(::glShaderSourceARB(v, 1, &vertexSourceC, NULL));

    std::string fragmentSource;
#if defined(USE_UNIFORM_BUFFER)
    fragmentSource += "#extension GL_EXT_bindable_uniform : enable\n";
    fragmentSource += "bindable uniform vec4 patch_attributes[4096];\n";
#endif
    fragmentSource += "varying vec4 color;\n";
    fragmentSource += "void main(void)\n";
    fragmentSource += "{\n";
#if defined(USE_UNIFORM_BUFFER)
    fragmentSource += "\tgl_FragColor = color * patch_attributes[0];\n";
#else
    fragmentSource += "\tgl_FragColor = color;\n";
#endif
    fragmentSource += "}\n";
    const char *fragmentSourceC = fragmentSource.c_str();
    GLFN(::glShaderSourceARB(f, 1, &fragmentSourceC, NULL));

    GLFN(::glCompileShaderARB(v));
    this->PrintShaderLog(v);
    GLFN(::glCompileShaderARB(f));
    this->PrintShaderLog(f);

    GLhandleARB p = GLFN(::glCreateProgramObjectARB());

    GLFN(::glAttachObjectARB(p, v));
    GLFN(::glAttachObjectARB(p, f));

    GLFN(::glLinkProgramARB(p));
    this->PrintShaderLog(p);

    GLFN(::glValidateProgram(p));

    GLint validateStatus;
    GLFN(::glGetProgramiv(p, GL_VALIDATE_STATUS, &validateStatus));
    REPORTERROR1("Validation status of the program is %d", validateStatus);

    GLint linkStatus;
    GLFN(::glGetProgramiv(p, GL_LINK_STATUS, &linkStatus));
    REPORTERROR1("Link status of the program is %d", linkStatus);

    GLFN(::glUseProgramObjectARB(p));

    this->mhVertexShader = v;
    this->mhFragmentShader = f;
    this->mhProgram = p;
}

void Renderer::DestroyShaders()
{
    GLFN(::glUseProgramObjectARB(0));
    GLFN(::glDetachObjectARB(this->mhProgram, this->mhFragmentShader));
    GLFN(::glDetachObjectARB(this->mhProgram, this->mhVertexShader));
    GLFN(::glDeleteObjectARB(this->mhProgram));
    GLFN(::glDeleteObjectARB(this->mhFragmentShader));
    GLFN(::glDeleteObjectARB(this->mhVertexShader));

    this->mhVertexShader = 0;
    this->mhFragmentShader = 0;
    this->mhProgram = 0;
}

void Renderer::PrintShaderLog(int handle)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    GLFN(::glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infologLength));
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        GLFN(::glGetInfoLogARB(handle, infologLength, &charsWritten, infoLog));
        REPORTERROR1("%s", infoLog);
        free(infoLog);
    }
}

void Renderer::SetupFragmentUniformBuffer()
{
    std::string uniformBufferName("patch_attributes");

    GLint loc = GLFN(::glGetUniformLocation(this->mhProgram, uniformBufferName.c_str()));
    if (-1 == loc)
    {
        REPORTERROR1("Uniform '%s' is not present in the program", uniformBufferName.c_str());
        return;
    }

    GLuint buffer;
    GLFN(::glGenBuffersARB(1, &buffer));

    GLint size = GLFN(::glGetUniformBufferSizeEXT(this->mhProgram, loc));
    REPORTERROR1("Uniform buffer size is %d bytes", size);

    float *bufferData = static_cast<float *>(malloc(size));
    int numFloats = size / sizeof(float);
    REPORTERROR1("Uniform buffer contains %d float values", numFloats);
    for (int i = 0; i < numFloats; ++i)
    {
        *(bufferData + i) = 1.0f;
    }

    GLFN(::glBindBufferARB(GL_UNIFORM_BUFFER_EXT, buffer));
    GLFN(::glBufferDataARB(GL_UNIFORM_BUFFER_EXT, size, bufferData, GL_STATIC_READ));
    GLFN(::glUniformBufferEXT(this->mhProgram, loc, buffer));
    GLFN(::glBindBufferARB(GL_UNIFORM_BUFFER_EXT, 0));
}

void Renderer::CreateTimerQuery()
{
    if (!GLEW_ARB_timer_query)
    {
        return;
    }

    GLFN(::glGenQueriesARB(1, static_cast< ::GLuint *>(&this->miTimerQuery)));
}

void Renderer::DestroyTimerQuery()
{
    if (!GLEW_ARB_timer_query)
    {
        return;
    }

    GLFN(::glDeleteQueriesARB(1, static_cast< ::GLuint *>(&this->miTimerQuery)));
}

void Renderer::StartTimerQuery()
{
    if (!GLEW_ARB_timer_query)
    {
        return;
    }

    GLFN(::glBeginQueryARB(GL_TIME_ELAPSED, this->miTimerQuery));
}

void Renderer::EndTimerQuery()
{
    if (!GLEW_ARB_timer_query)
    {
        return;
    }

    GLFN(::glEndQueryARB(GL_TIME_ELAPSED));
}

uint64 Renderer::GetTimeElapsed()
{
    if (!GLEW_ARB_timer_query)
    {
        return 0;
    }

    GLint available = 0;
    while (0 == available)
    {
        GLFN(::glGetQueryObjectivARB(this->miTimerQuery, GL_QUERY_RESULT_AVAILABLE, &available));
    }

    uint64 lui64TimeElapsed;
    GLFN(::glGetQueryObjectui64v(this->miTimerQuery, GL_QUERY_RESULT, &lui64TimeElapsed));
    return lui64TimeElapsed;
}
