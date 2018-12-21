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
namespace glslang
{
    class GLSLangValidator :
        C.Cxx.ConsoleApplication,
        Bam.Core.ICommandLineTool
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var headers = this.CreateHeaderContainer();
            headers.AddFiles("$(packagedir)/glslang/**.h");

            var source = this.CreateCxxSourceContainer();
            source.AddFiles("$(packagedir)/StandAlone/StandAlone.cpp");
            source.AddFiles("$(packagedir)/StandAlone/ResourceLimits.cpp");

            source.PrivatePatch(settings =>
            {
                var compiler = settings as C.ICommonCompilerSettings;
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)"));
                compiler.PreprocessorDefines.Add("ENABLE_OPT", "0"); // don't always enable optimisations

                var cxxCompiler = settings as C.ICxxOnlyCompilerSettings;
                cxxCompiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;
                cxxCompiler.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;
            });

            this.PrivatePatch(settings =>
            {
                var cxxLinker = settings as C.ICxxOnlyLinkerSettings;
                cxxLinker.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;
            });

            this.CompileAndLinkAgainst<GLSLang>(source);
            this.CompileAndLinkAgainst<OGLCompilersDLL>(source);
            this.CompileAndLinkAgainst<SPIRV>(source);
        }

        System.Collections.Generic.Dictionary<string, Bam.Core.TokenizedStringArray> Bam.Core.ICommandLineTool.EnvironmentVariables => null;
        Bam.Core.StringArray Bam.Core.ICommandLineTool.InheritedEnvironmentVariables => null;
        Bam.Core.TokenizedString Bam.Core.ICommandLineTool.Executable => this.GeneratedPaths[C.Cxx.ConsoleApplication.ExecutableKey];
        Bam.Core.TokenizedStringArray Bam.Core.ICommandLineTool.InitialArguments => null;
        Bam.Core.TokenizedStringArray Bam.Core.ICommandLineTool.TerminatingArguments => null;
        string Bam.Core.ICommandLineTool.UseResponseFileOption => null;
        Bam.Core.Array<int> Bam.Core.ICommandLineTool.SuccessfulExitCodes => new Bam.Core.Array<int> { 0 };

        Bam.Core.Settings
        Bam.Core.ITool.CreateDefaultSettings<T>(
            T module)
        {
            return new VulkanSDK.GLSLangValidatorSettings(module);
        }
    }

    class GLSLang :
        C.StaticLibrary
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var source = this.CreateCxxSourceContainer("$(packagedir)/glslang/MachineIndependent/*.cpp");
            source.AddFiles("$(packagedir)/glslang/MachineIndependent/preprocessor/*.cpp");
            source.AddFiles("$(packagedir)/glslang/GenericCodeGen/*.cpp");
            if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.Windows))
            {
                source.AddFiles("$(packagedir)/glslang/OSDependent/Windows/ossource.cpp");
            }
            else
            {
                source.AddFiles("$(packagedir)/glslang/OSDependent/Unix/ossource.cpp");
            }
            source.PrivatePatch(settings =>
            {
                var cxxCompiler = settings as C.ICxxOnlyCompilerSettings;
                cxxCompiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;
                cxxCompiler.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;
            });
        }
    }

    class OGLCompilersDLL :
        C.StaticLibrary
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);
            var source = this.CreateCxxSourceContainer("$(packagedir)/OGLCompilersDLL/*.cpp");
            source.PrivatePatch(settings =>
            {
                var cxxCompiler = settings as C.ICxxOnlyCompilerSettings;
                cxxCompiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;
                cxxCompiler.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;
            });
        }
    }

    class SPIRV :
        C.StaticLibrary
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var source = this.CreateCxxSourceContainer();
            source.AddFiles("$(packagedir)/SPIRV/GlslangToSpv.cpp");
            source.AddFiles("$(packagedir)/SPIRV/InReadableOrder.cpp");
            source.AddFiles("$(packagedir)/SPIRV/Logger.cpp");
            source.AddFiles("$(packagedir)/SPIRV/SpvBuilder.cpp");
            source.AddFiles("$(packagedir)/SPIRV/SpvPostProcess.cpp");
            source.AddFiles("$(packagedir)/SPIRV/doc.cpp");
            source.AddFiles("$(packagedir)/SPIRV/SpvTools.cpp");
            source.AddFiles("$(packagedir)/SPIRV/disassemble.cpp");
            source.PrivatePatch(settings =>
            {
                var cxxCompiler = settings as C.ICxxOnlyCompilerSettings;
                cxxCompiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;
                cxxCompiler.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;
            });
        }
    }
}
