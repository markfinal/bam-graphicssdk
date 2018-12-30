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
namespace MoltenVK
{
    class MoltenVK :
        C.Cxx.DynamicLibrary
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            this.SetSemanticVersion(1, 0, 21);

            this.CreateHeaderContainer("$(packagedir)/MoltenVK/**.h");

            var cxx_source = this.CreateCxxSourceContainer();
            cxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/Layers/*.cpp");
            cxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/Utility/*.cpp");
            cxx_source.AddFiles("$(packagedir)/MoltenVKShaderConverter/MoltenVKSPIRVToMSLConverter/*.cpp");

            cxx_source.PrivatePatch(settings =>
            {
                var preprocessor = settings as C.ICommonPreprocessorSettings;
                if (this.BuildEnvironment.Configuration.HasFlag(Bam.Core.EConfiguration.Debug))
                {
                    preprocessor.PreprocessorDefines.Add("MVK_DEBUG", "1");
                }
                preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/Common"));
                preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/API"));
                preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/Utility"));

                var cxx_compiler = settings as C.ICxxOnlyCompilerSettings;
                cxx_compiler.ExceptionHandler = C.Cxx.EExceptionHandler.Asynchronous;
                cxx_compiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;

                if (settings is ClangCommon.ICommonCompilerSettings clang_compiler)
                {
                    clang_compiler.AllWarnings = true;
                    clang_compiler.ExtraWarnings = true;
                    clang_compiler.Pedantic = true;

                    var compiler = settings as C.ICommonCompilerSettings;
                    compiler.DisableWarnings.AddUnique("vla-extension");
                    compiler.DisableWarnings.AddUnique("unused-parameter");
                    compiler.DisableWarnings.AddUnique("gnu-zero-variadic-macro-arguments");
                    compiler.DisableWarnings.AddUnique("ignored-qualifiers");
                    compiler.DisableWarnings.AddUnique("gnu-anonymous-struct");
                    compiler.DisableWarnings.AddUnique("nested-anon-types");
                }
            });

            cxx_source["Utility/MVKBaseObject.cpp"].ForEach(item => item.PrivatePatch(settings =>
            {
                if (settings is ClangCommon.ICommonCompilerSettings)
                {
                    var compiler = settings as C.ICommonCompilerSettings;
                    compiler.DisableWarnings.AddUnique("missing-braces");
                }
            }));

            cxx_source["MoltenVKShaderConverter/MoltenVKSPIRVToMSLConverter/SPIRVToMSLConverter.cpp"].ForEach(item => item.PrivatePatch(settings =>
            {
                if (settings is ClangCommon.ICommonCompilerSettings)
                {
                    var compiler = settings as C.ICommonCompilerSettings;
                    compiler.DisableWarnings.AddUnique("import-preprocessor-directive-pedantic");
                }
            }));

            var objc_source = this.CreateObjectiveCSourceContainer();
            objc_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/OS/*.m");

            objc_source.PrivatePatch(settings =>
            {
                var preprocessor = settings as C.ICommonPreprocessorSettings;
                if (this.BuildEnvironment.Configuration.HasFlag(Bam.Core.EConfiguration.Debug))
                {
                    preprocessor.PreprocessorDefines.Add("MVK_DEBUG", "1");
                }
                preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/Common"));
            });

            var objcxx_source = this.CreateObjectiveCxxSourceContainer();
            objcxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/Commands/*.mm");
            objcxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/GPUObjects/*.mm");
            objcxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/Layers/*.mm");
            objcxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/OS/*.mm");
            objcxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/Utility/*.mm");
            objcxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/Vulkan/*.mm");
            objcxx_source.AddFiles("$(packagedir)/MoltenVKShaderConverter/MoltenVKSPIRVToMSLConverter/*.mm");

            this.CompileAgainst<VulkanHeaders.VkHeaders>(cxx_source, objcxx_source);
            this.CompileAgainst<cereal.cereal>(objcxx_source);

            this.CompileAndLinkAgainst<SPIRVTools.SPIRVTools>(cxx_source);
            this.CompileAndLinkAgainst<SPIRVCross.SPIRVCross>(cxx_source, objcxx_source);

            objcxx_source.PrivatePatch(settings =>
            {
                var preprocessor = settings as C.ICommonPreprocessorSettings;
                if (this.BuildEnvironment.Configuration.HasFlag(Bam.Core.EConfiguration.Debug))
                {
                    preprocessor.PreprocessorDefines.Add("MVK_DEBUG", "1");
                }
                preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/Common"));
                preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/API"));
                preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/Commands"));
                preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/GPUObjects"));
                preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/Layers"));
                preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/OS"));
                preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/Utility"));
                preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVKShaderConverter"));

                var compiler = settings as C.ICommonCompilerSettings;
                compiler.DisableWarnings.AddUnique("unguarded-availability-new"); // MoltenVK-1.0.10/MoltenVK/MoltenVK/Commands/MVKCmdTransfer.mm:582:19: error: 'dispatchThreads:threadsPerThreadgroup:' is only available on macOS 10_13 or newer [-Werror,-Wunguarded-availability-new]
                compiler.DisableWarnings.AddUnique("nonportable-include-path"); // MoltenVK-1.0.10/MoltenVK/MoltenVK/Vulkan/vulkan.mm:33:10: error: non-portable path to file '"MVKRenderPass.h"'; specified path differs in case from file name on disk [-Werror,-Wnonportable-include-path]
                compiler.DisableWarnings.AddUnique("deprecated-declarations");

                var cxx_compiler = settings as C.ICxxOnlyCompilerSettings;
                cxx_compiler.ExceptionHandler = C.Cxx.EExceptionHandler.Asynchronous;
                cxx_compiler.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;
                cxx_compiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;

                if (settings is ClangCommon.ICommonCompilerSettings clangCompiler)
                {
                    clangCompiler.AllWarnings = false;
                    clangCompiler.ExtraWarnings = false;
                    clangCompiler.Pedantic = false;
                }
            });

            this.PrivatePatch(settings =>
            {
                var macos_linker = settings as C.ICommonLinkerSettingsOSX;
                macos_linker.Frameworks.AddUnique("Metal");
                macos_linker.Frameworks.AddUnique("Foundation");
                macos_linker.Frameworks.AddUnique("IOKit");
                macos_linker.Frameworks.AddUnique("IOSurface");
                macos_linker.Frameworks.AddUnique("QuartzCore");
            });

            this.PublicPatch((settings, appliedTo) =>
            {
                if (settings is C.ICommonPreprocessorSettings preprocessor)
                {
                    preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/include"));
                }
            });
        }
    }
}
