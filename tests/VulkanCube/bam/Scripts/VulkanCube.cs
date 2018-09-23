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
            Clang.MetaData instance)
        {
            instance.MacOSXMinimumVersionSupported = "10.9";
        }
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

                        var compiler = settings as C.ICommonCompilerSettings;
                        compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/source"));

                        var clang_compiler = settings as ClangCommon.ICommonCompilerSettings;
                        if (null != clang_compiler)
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
                source.AddFile("$(packagedir)/source/entry/windows/*.cpp");
                this.CompileAndLinkAgainst<VulkanSDK.Vulkan>(source);
            }

            source.PrivatePatch(settings =>
                {
                    var cxxcompiler = settings as C.ICxxOnlyCompilerSettings;
                    cxxcompiler.ExceptionHandler = C.Cxx.EExceptionHandler.Asynchronous;
                    cxxcompiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;

                    var compiler = settings as C.ICommonCompilerSettings;
                    compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/source"));

                    var clang_compiler = settings as ClangCommon.ICommonCompilerSettings;
                    if (null != clang_compiler)
                    {
                        clang_compiler.AllWarnings = true;
                        clang_compiler.ExtraWarnings = true;
                        clang_compiler.Pedantic = true;
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

                var linker = settings as C.ICommonLinkerSettings;
                if (settings is VisualCCommon.ICommonLinkerSettings)
                {
                    linker.Libraries.Add("user32.lib");
                }
            });
        }
    }

    sealed class RuntimePackage :
        Publisher.Collation
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            this.SetDefaultMacrosAndMappings(EPublishingType.WindowedApplication);

            this.Include<Cube>(C.Cxx.GUIApplication.ExecutableKey);
        }
    }
}
