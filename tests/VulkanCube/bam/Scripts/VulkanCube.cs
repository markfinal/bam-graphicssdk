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
namespace VulkanCube
{
    sealed class ConfigureOSX :
        Bam.Core.IPackageMetaDataConfigure<Clang.MetaData>
    {
        void
        Bam.Core.IPackageMetaDataConfigure<Clang.MetaData>.Configure(
            Clang.MetaData instance) => instance.MacOSXMinimumVersionSupported = "10.13";
    }

    class Cube :
        C.Cxx.GUIApplication
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            this.CreateHeaderContainer("$(packagedir)/source/**.h");
            var source = this.CreateCxxSourceContainer("$(packagedir)/source/*.cpp");
            source.AddFiles("$(packagedir)/source/renderer/*.cpp");

            if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.OSX))
            {
                var objCxxSource = this.CreateObjectiveCxxSourceContainer("$(packagedir)/source/entry/macos/*.mm");
                objCxxSource.PrivatePatch(settings =>
                    {
                        var cxxcompiler = settings as C.ICxxOnlyCompilerSettings;
                        cxxcompiler.ExceptionHandler = C.Cxx.EExceptionHandler.Asynchronous;
                        cxxcompiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;

                        var preprocessor = settings as C.ICommonPreprocessorSettings;
                        preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/source"));

                        if (settings is ClangCommon.ICommonCompilerSettings clang_compiler)
                        {
                            clang_compiler.AllWarnings = true;
                            clang_compiler.ExtraWarnings = true;
                            clang_compiler.Pedantic = true;
                        }
                    });

                this.CompileAndLinkAgainst<WindowLibrary.GraphicsWindow>(source, objCxxSource);
                this.CompileAndLinkAgainst<MoltenVK.MoltenVK>(source);
                this.CompileAgainst<VulkanHeaders.VkHeaders>(source);
            }
            else
            {
                this.CompileAndLinkAgainst<WindowLibrary.GraphicsWindow>(source);
                source.AddFiles("$(packagedir)/source/entry/windows/*.cpp");
                this.CompileAndLinkAgainst<VulkanSDK.Vulkan>(source);
            }

            source.PrivatePatch(settings =>
                {
                    var cxxcompiler = settings as C.ICxxOnlyCompilerSettings;
                    cxxcompiler.ExceptionHandler = C.Cxx.EExceptionHandler.Asynchronous;
                    cxxcompiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;

                    var preprocessor = settings as C.ICommonPreprocessorSettings;
                    preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/source"));

                    switch (settings)
                    {
                        case ClangCommon.ICommonCompilerSettings clang_compiler:
                            clang_compiler.AllWarnings = true;
                            clang_compiler.ExtraWarnings = true;
                            clang_compiler.Pedantic = true;
                            break;
                        case GccCommon.ICommonCompilerSettings gcc_compiler:
                            gcc_compiler.AllWarnings = true;
                            gcc_compiler.ExtraWarnings = true;
                            gcc_compiler.Pedantic = true;
                            break;
                        case VisualCCommon.ICommonCompilerSettings vc_compiler:
                            vc_compiler.WarningLevel = VisualCCommon.EWarningLevel.Level4;
                            preprocessor.PreprocessorDefines.Add("NOMINMAX"); // so std::numeric_limits<type>::max() will work
                            break;
                    }
                });

            this.PrivatePatch(settings =>
            {
                if (settings is ClangCommon.ICommonLinkerSettings clang_linker)
                {
                    clang_linker.RPath.AddUnique(@"@executable_path/../Frameworks");
                }

                if (settings is C.ICommonLinkerSettingsOSX linkerOSX)
                {
                    linkerOSX.Frameworks.AddUnique("Cocoa");
                    linkerOSX.Frameworks.AddUnique("Metal");
                    linkerOSX.Frameworks.AddUnique("MetalKit");
                    linkerOSX.Frameworks.AddUnique("QuartzCore");
                }

                if (settings is VisualCCommon.ICommonLinkerSettings)
                {
                    var linker = settings as C.ICommonLinkerSettings;
                    linker.Libraries.Add("user32.lib");
                }
            });

            var vertexShaderGLSL = Bam.Core.Module.Create<VulkanSDK.GLSLSource>(preInitCallback: module =>
                {
                    module.InputPath = this.CreateTokenizedString("$(packagedir)/shaders/shader.vert");
                });
            var vertexShaderSPIRV = Bam.Core.Module.Create<VulkanSDK.SPIRVModule>(preInitCallback: module =>
                {
                    module.Source = vertexShaderGLSL;
                    module.DependsOn(vertexShaderGLSL);
                });
            this.Requires(vertexShaderSPIRV);

            var fragmentShaderGLSL = Bam.Core.Module.Create<VulkanSDK.GLSLSource>(preInitCallback: module =>
            {
                module.InputPath = this.CreateTokenizedString("$(packagedir)/shaders/shader.frag");
            });
            var fragmentShaderSPIRV = Bam.Core.Module.Create<VulkanSDK.SPIRVModule>(preInitCallback: module =>
            {
                module.Source = fragmentShaderGLSL;
                module.DependsOn(fragmentShaderGLSL);
            });
            this.Requires(fragmentShaderSPIRV);
        }
    }

    sealed class CubeRuntime :
        Publisher.Collation
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            this.SetDefaultMacrosAndMappings(EPublishingType.WindowedApplication);

            this.Mapping.Register(
                typeof(VulkanSDK.SPIRVModule),
                VulkanSDK.SPIRVModule.SPIRVKey,
                this.CreateTokenizedString(
                    "$(0)",
                    new[] { this.ExecutableDir }
                ),
                true);

            var appAnchor = this.Include<Cube>(C.Cxx.GUIApplication.ExecutableKey);

            var app = appAnchor.SourceModule as Cube;
            if (this.BuildEnvironment.Configuration != Bam.Core.EConfiguration.Debug &&
                app.Linker is VisualCCommon.LinkerBase)
            {
                var runtimeLibrary = Bam.Core.Graph.Instance.PackageMetaData<VisualCCommon.IRuntimeLibraryPathMeta>("VisualC");
                this.IncludeFiles(runtimeLibrary.CRuntimePaths(app.BitDepth), this.ExecutableDir, appAnchor);
                this.IncludeFiles(runtimeLibrary.CxxRuntimePaths(app.BitDepth), this.ExecutableDir, appAnchor);
            }
        }
    }
}
