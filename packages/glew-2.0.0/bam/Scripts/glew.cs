#region License
// Copyright (c) 2010-2017, Mark Final
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
namespace glew
{
    [Bam.Core.ModuleGroup("Thirdparty/GLEW")]
    class GLEWStatic :
        C.StaticLibrary
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            this.CreateHeaderContainer("$(packagedir)/include/GL/*.h");
            var source = this.CreateCSourceContainer();
            source.AddFile("$(packagedir)/src/glew.c");

            source.PrivatePatch(settings =>
                {
                    var cCompiler = settings as C.ICOnlyCompilerSettings;
                    cCompiler.LanguageStandard = C.ELanguageStandard.C89;

                    var visualCCompiler = settings as VisualCCommon.ICommonCompilerSettings;
                    if (null != visualCCompiler)
                    {
                        visualCCompiler.WarningLevel = VisualCCommon.EWarningLevel.Level4;
                        var compiler = settings as C.ICommonCompilerSettings;
                        compiler.DisableWarnings.AddUnique("4456"); // glew-2.0.0\src\glew.c(13538): warning C4456: declaration of 'n' hides previous local declaration
                    }

                    var mingwCompiler = settings as MingwCommon.ICommonCompilerSettings;
                    if (null != mingwCompiler)
                    {
                        mingwCompiler.AllWarnings = true;
                        mingwCompiler.ExtraWarnings = true;
                        mingwCompiler.Pedantic = true;
                    }

                    var gccCompiler = settings as GccCommon.ICommonCompilerSettings;
                    if (null != gccCompiler)
                    {
                        gccCompiler.AllWarnings = true;
                        gccCompiler.ExtraWarnings = true;
                        gccCompiler.Pedantic = true;
                    }

                    var clangCompiler = settings as ClangCommon.ICommonCompilerSettings;
                    if (null != clangCompiler)
                    {
                        clangCompiler.AllWarnings = true;
                        clangCompiler.ExtraWarnings = true;
                        clangCompiler.Pedantic = true;
                    }
                });

            this.PublicPatch((settings, appliedTo) =>
                {
                    var compiler = settings as C.ICommonCompilerSettings;
                    if (null != compiler)
                    {
                        compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/include"));
                        compiler.PreprocessorDefines.Add("GLEW_STATIC");
                        compiler.PreprocessorDefines.Add("GLEW_NO_GLU");
                    }
                });

            if (source.Compiler is VisualCCommon.CompilerBase)
            {
                this.CompileAgainst<WindowsSDK.WindowsSDK>(source);
            }
        }
    }

    namespace tests
    {
        [Bam.Core.ModuleGroup("Thirdparty/GLEW/tests")]
        class GLEWInfo :
            C.ConsoleApplication
        {
            protected override void
            Init(
                Bam.Core.Module parent)
            {
                base.Init(parent);

                var source = this.CreateCSourceContainer("$(packagedir)/src/glewinfo.c");
                this.CompileAndLinkAgainst<GLEWStatic>(source);
                this.CompileAndLinkAgainst<OpenGLSDK.OpenGL>(source);

                if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.Linux))
                {
                    this.PrivatePatch(settings =>
                        {
                            var linker = settings as C.ICommonLinkerSettings;
                            linker.Libraries.AddUnique("-lX11");
                        });
                }
                else if (this.Linker is VisualCCommon.LinkerBase)
                {
                    this.LinkAgainst<WindowsSDK.WindowsSDK>();
                    this.PrivatePatch(settings =>
                        {
                            var linker = settings as C.ICommonLinkerSettings;
                            linker.Libraries.AddUnique("USER32.lib");
                            linker.Libraries.AddUnique("GDI32.lib");
                        });
                }
            }
        }

        sealed class TestRuntime :
            Publisher.Collation
        {
            protected override void
            Init(
                Bam.Core.Module parent)
            {
                base.Init(parent);

#if D_NEW_PUBLISHING
                this.SetDefaultMacrosAndMappings(EPublishingType.ConsoleApplication);
                this.IncludeAllModulesInNamespace("glew.tests", C.ConsoleApplication.Key);
#else
#endif
            }
        }
    }
}
