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
namespace VulkanSDK
{
    [C.Prebuilt]
    class Vulkan :
        C.DynamicLibrary
    {
        protected override void
        Init()
        {
            base.Init();

            var latest_version_path = GetInstallDir.Find(this.BuildEnvironment.Platform);
            this.Macros[Bam.Core.ModuleMacroNames.PackageDirectory].Set(latest_version_path, null);

            if (Bam.Core.OSUtilities.Is64Bit(this.BuildEnvironment.Platform))
            {
                this.Macros["VulkanLibDir"] = this.CreateTokenizedString("$(packagedir)/Source/Lib");
            }
            else
            {
                this.Macros["VulkanLibDir"] = this.CreateTokenizedString("$(packagedir)/Source/Lib32");
            }

            this.Macros[Bam.Core.ModuleMacroNames.OutputName] = this.CreateTokenizedString("vulkan-1");
            this.RegisterGeneratedFile(
                ExecutableKey,
                this.CreateTokenizedString("$(VulkanLibDir)/$(dynamicprefix)$(OutputName)$(dynamicext)") // note: 64-bit
            );
            if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.Windows))
            {
                this.RegisterGeneratedFile(
                    ImportLibraryKey,
                    this.CreateTokenizedString("$(VulkanLibDir)/$(libprefix)$(OutputName)$(libext)")
                );
            }

            var headers = this.CreateHeaderCollection();
            headers.Macros[Bam.Core.ModuleMacroNames.PackageDirectory].Set(latest_version_path, null); // must set this as well as on this, since it doesn't inherit
            headers.AddFiles("$(packagedir)/Include/vulkan/*.h");
            headers.AddFiles("$(packagedir)/Include/vulkan/*.hpp");

            this.PublicPatch((settings, appliedTo) =>
                {
                    if (settings is C.ICommonPreprocessorSettings preprocessor)
                    {
                        preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/Include"));

                        if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.Windows))
                        {
                            preprocessor.PreprocessorDefines.Add("VK_USE_PLATFORM_WIN32_KHR");
                        }
                        else if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.Linux))
                        {
                            preprocessor.PreprocessorDefines.Add("VK_USE_PLATFORM_XLIB_KHR");
                        }
                    }
                });
        }
    }
}
