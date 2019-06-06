#region License
// Copyright (c) 2010-2018, Mark Final
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of BuildAMation nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#endregion // License
using Bam.Core;
using System.Linq;
namespace OpenGLTriangle
{
    class GLUniformBufferTest :
        C.Cxx.GUIApplication
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            this.CreateHeaderContainer("$(packagedir)/source/*.h");

            var source = this.CreateCxxSourceContainer("$(packagedir)/source/*.cpp");
            source.PrivatePatch(settings =>
                {
                    var cxxCompiler = settings as C.ICxxOnlyCompilerSettings;
                    cxxCompiler.ExceptionHandler = C.Cxx.EExceptionHandler.Synchronous;
                    cxxCompiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;
                    cxxCompiler.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;

                    var preprocessor = settings as C.ICommonPreprocessorSettings;
                    preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/source"));
                });

            if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.Windows))
            {
                source.AddFiles("$(packagedir)/source/platform/winmain.cpp");
            }
            else if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.Linux))
            {
                source.AddFiles("$(packagedir)/source/platform/main.cpp");
            }
            else if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.OSX))
            {
                var objCSource = this.CreateObjectiveCxxSourceContainer("$(packagedir)/source/**.mm");
                objCSource.PrivatePatch(settings =>
                    {
                        var cxxCompiler = settings as C.ICxxOnlyCompilerSettings;
                        cxxCompiler.ExceptionHandler = C.Cxx.EExceptionHandler.Synchronous;
                        cxxCompiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;
                        cxxCompiler.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;

                        var preprocessor = settings as C.ICommonPreprocessorSettings;
                        preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/source"));
                    });
            }

            this.CompileAndLinkAgainst<WindowLibrary.OpenGLContext>(source);
            this.CompileAndLinkAgainst<OpenGLSDK.OpenGL>(source);

            var rendererObj = source.Children.Where(item => (item as C.Cxx.ObjectFile).InputPath.ToString().Contains("renderer")).ElementAt(0) as C.Cxx.ObjectFile;
            this.CompileAndLinkAgainst<glew.GLEWStatic>(rendererObj);

            this.PrivatePatch(settings =>
                {
                    var linker = settings as C.ICommonLinkerSettings;
                    var cxxLinker = settings as C.ICxxOnlyLinkerSettings;
                    if (null != cxxLinker)
                    {
                        cxxLinker.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;
                    }
                    if (this.Linker is VisualCCommon.LinkerBase)
                    {
                        linker.Libraries.Add("OPENGL32.lib");
                        linker.Libraries.Add("USER32.lib");
                        linker.Libraries.Add("GDI32.lib");
                    }
                    else if (this.Linker is MingwCommon.LinkerBase)
                    {
                        linker.Libraries.Add("-lopengl32");
                        linker.Libraries.Add("-lgdi32");
                    }
                    else if (this.Linker is GccCommon.LinkerBase)
                    {
                        linker.Libraries.Add("-lX11");
                        linker.Libraries.Add("-lpthread");
                    }
                    else if (this.Linker is ClangCommon.LinkerBase)
                    {
                        var osxLinker = settings as C.ICommonLinkerSettingsOSX;
                        // in order to link against libc++
                        osxLinker.MacOSMinimumVersionSupported = "10.9";
                        osxLinker.Frameworks.AddUnique("Cocoa");
                    }
                });
        }
    }

    sealed class Runtime :
        Publisher.Collation
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            this.SetDefaultMacrosAndMappings(EPublishingType.WindowedApplication);
            this.Include<GLUniformBufferTest>(C.Cxx.ConsoleApplication.ExecutableKey);
        }
    }
}
