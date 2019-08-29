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
using Bam.Core;
namespace MoltenVK
{
    class MoltenVK : C.Cxx.DynamicLibrary
    {
        protected override void
        Init()
        {
            base.Init();

            this.SetSemanticVersion(1, 0, 10);

            var cxx_source = this.CreateCxxSourceCollection();
            cxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/Utility/*.cpp");
            cxx_source.AddFiles("$(packagedir)/MoltenVKShaderConverter/MoltenVKSPIRVToMSLConverter/*.cpp");

            cxx_source.PrivatePatch(settings =>
            {
                var compiler = settings as C.ICommonCompilerSettings;
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/Common"));

                var cxx_compiler = settings as C.ICxxOnlyCompilerSettings;
                cxx_compiler.ExceptionHandler = C.Cxx.EExceptionHandler.Asynchronous;
                cxx_compiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;

                var clang_compiler = settings as ClangCommon.ICommonCompilerSettings;
                if (null != clang_compiler)
                {
                    clang_compiler.AllWarnings = true;
                    clang_compiler.ExtraWarnings = true;
                    clang_compiler.Pedantic = true;
                }
            });

            cxx_source["Utility/MVKBaseObject.cpp"].ForEach(item => item.PrivatePatch(settings =>
            {
                var clang_compiler = settings as ClangCommon.ICommonCompilerSettings;
                if (null != clang_compiler)
                {
                    var compiler = settings as C.ICommonCompilerSettings;
                    compiler.DisableWarnings.AddUnique("missing-braces");
                }
            }));

            cxx_source["MoltenVKShaderConverter/MoltenVKSPIRVToMSLConverter/SPIRVToMSLConverter.cpp"].ForEach(item => item.PrivatePatch(settings =>
            {
                var clang_compiler = settings as ClangCommon.ICommonCompilerSettings;
                if (null != clang_compiler)
                {
                    var compiler = settings as C.ICommonCompilerSettings;
                    compiler.DisableWarnings.AddUnique("import-preprocessor-directive-pedantic");
                }
            }));

            var objcxx_source = this.CreateObjectiveCxxSourceCollection();
            objcxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/Commands/*.mm");
            objcxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/GPUObjects/*.mm");
            objcxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/Loader/*.mm");
            objcxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/Utility/*.mm");
            objcxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/Vulkan/*.mm");
            objcxx_source.AddFiles("$(packagedir)/MoltenVKShaderConverter/MoltenVKSPIRVToMSLConverter/*.mm");

            this.CompileAgainst<VulkanHeaders.VkHeaders>(cxx_source, objcxx_source);
            this.CompileAgainst<cereal.cereal>(objcxx_source);

            this.CompileAndLinkAgainst<SPIRVTools.SPIRVTools>(cxx_source);
            this.CompileAndLinkAgainst<SPIRVCross.SPIRVCross>(cxx_source, objcxx_source);

            objcxx_source.PrivatePatch(settings =>
            {
                var compiler = settings as C.ICommonCompilerSettings;
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/Common"));
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/API"));
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/Commands"));
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/GPUObjects"));
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/Loader"));
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/Utility"));
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVKShaderConverter"));

                compiler.DisableWarnings.AddUnique("unguarded-availability-new"); // MoltenVK-1.0.10/MoltenVK/MoltenVK/Commands/MVKCmdTransfer.mm:582:19: error: 'dispatchThreads:threadsPerThreadgroup:' is only available on macOS 10_13 or newer [-Werror,-Wunguarded-availability-new]
                compiler.DisableWarnings.AddUnique("nonportable-include-path"); // MoltenVK-1.0.10/MoltenVK/MoltenVK/Vulkan/vulkan.mm:33:10: error: non-portable path to file '"MVKRenderPass.h"'; specified path differs in case from file name on disk [-Werror,-Wnonportable-include-path]

                var cxx_compiler = settings as C.ICxxOnlyCompilerSettings;
                cxx_compiler.ExceptionHandler = C.Cxx.EExceptionHandler.Asynchronous;
                cxx_compiler.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;
                cxx_compiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;

                var clang_compiler = settings as ClangCommon.ICommonCompilerSettings;
                if (null != clang_compiler)
                {
                    clang_compiler.AllWarnings = false;
                    clang_compiler.ExtraWarnings = false;
                    clang_compiler.Pedantic = false;
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
        }
    }
}
