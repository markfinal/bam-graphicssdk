#region License
// Copyright (c) 2010-2019, Mark Final
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
using MetalUtilities;
namespace MetalTriangle
{
    class MetalTriangle :
        C.Cxx.GUIApplication
    {
        protected override void
        Init()
        {
            base.Init();

            var shaderSource = Bam.Core.Module.Create<MetalUtilities.MetalShaderSource>(
                preInitCallback: module =>
                    {
                        (module as Bam.Core.IInputPath).InputPath = this.CreateTokenizedString("$(packagedir)/resources/shaders.metal");
                    }
            );
            var shaderCompiled = Bam.Core.Module.Create<MetalUtilities.CompiledMetalShader>(
                preInitCallback: module =>
                    {
                        module.ShaderSource = shaderSource;
                        module.DependsOn(shaderSource);
                    }
            );

            var defaultShaderLibrary = Bam.Core.Graph.Instance.FindReferencedModule<MetalUtilities.DefaultMetalShaderLibrary>();
            defaultShaderLibrary.DependsOn(shaderCompiled);

            var source = this.CreateObjectiveCxxSourceCollection("$(packagedir)/source/*.mm");
            source.PrivatePatch(settings =>
            {
                var compiler = settings as C.ICommonCompilerSettings;
                compiler.WarningsAsErrors = true;

                var cxxCompiler = settings as C.ICxxOnlyCompilerSettings;
                cxxCompiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;
                cxxCompiler.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;

                var clangCompiler = settings as ClangCommon.ICommonCompilerSettings;
                clangCompiler.AllWarnings = true;
                clangCompiler.ExtraWarnings = true;
                clangCompiler.Pedantic = true;
            });

            this.CompileAndLinkAgainst<WindowLibrary.GraphicsWindow>(source);
            this.DependsOn(defaultShaderLibrary);

            this.PrivatePatch(settings =>
            {
                var cxxLinker = settings as C.ICxxOnlyLinkerSettings;
                cxxLinker.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;

                var osxLinker = settings as C.ICommonLinkerSettingsOSX;
                osxLinker.Frameworks.AddUnique("Cocoa");
                osxLinker.Frameworks.AddUnique("Metal");
                osxLinker.Frameworks.AddUnique("MetalKit");
                osxLinker.Frameworks.AddUnique("QuartzCore"); // including Core Animation
                osxLinker.MacOSMinimumVersionSupported = "10.9";
            });
        }
    }

    sealed class Runtime :
        Publisher.Collation
    {
        protected override void
        Init()
        {
            base.Init();

            this.SetDefaultMacrosAndMappings(EPublishingType.WindowedApplication);
            if (Bam.Core.Graph.Instance.Mode != "Xcode")
            {
                // not on Xcode, since this auto-generates the Metal shader library
                this.registerMetalMappings();
            }

            this.Include<MetalTriangle>(C.Cxx.GUIApplication.ExecutableKey);
        }
    }
}
