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
namespace VulkanSDK
{
    [C.Prebuilt]
    public class Vulkan :
        C.DynamicLibrary
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var latest_version_path = System.String.Empty;
            if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.Windows))
            {
                using (var key = Bam.Core.Win32RegistryUtilities.OpenLMSoftwareKey(@"LunarG\VulkanSDK"))
                {
                    if (null == key)
                    {
                        throw new Bam.Core.Exception("Unable to locate any Vulkan SDK installations");
                    }
                    var linearised_paths = key.GetValue("VK_SDK_PATHs") as string;
                    var paths = new Bam.Core.StringArray(linearised_paths.Split(new[] { ';' }, System.StringSplitOptions.RemoveEmptyEntries));
                    System.Diagnostics.Debug.Assert(paths.Count > 0);
                    var applicable_paths = new System.Collections.Generic.SortedDictionary<string, string>();
                    foreach (var path in paths)
                    {
                        var dir_split = path.Split(new[] { System.IO.Path.DirectorySeparatorChar }, System.StringSplitOptions.RemoveEmptyEntries);
                        var version = dir_split[dir_split.Length - 1];
                        if (version.StartsWith("1.0"))
                        {
                            applicable_paths.Add(version, path);
                        }
                    }
                    System.Diagnostics.Debug.Assert(applicable_paths.Count > 0);
                    latest_version_path = applicable_paths.First().Value;
                }
            }
            else
            {
                // TODO: is this compatible with the Linux installer?
                latest_version_path = System.Environment.ExpandEnvironmentVariables("VK_SDK_PATH");
                System.Diagnostics.Debug.Assert(latest_version_path.StartsWith("1.0"));
            }
            Bam.Core.Log.Info("Using VulkanSDK installed at {0}", latest_version_path);
            this.Macros["packagedir"].Set(latest_version_path, null);

            if (Bam.Core.OSUtilities.Is64Bit(this.BuildEnvironment.Platform))
            {
                this.Macros["VulkanLibDir"] = this.CreateTokenizedString("$(packagedir)/Source/Lib");
            }
            else
            {
                this.Macros["VulkanLibDir"] = this.CreateTokenizedString("$(packagedir)/Source/Lib32");
            }

            this.Macros["OutputName"] = this.CreateTokenizedString("vulkan-1");
            this.GeneratedPaths[Key] = this.CreateTokenizedString("$(VulkanLibDir)/$(dynamicprefix)$(OutputName)$(dynamicext)"); // note: 64-bit
            if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.Windows))
            {
                this.GeneratedPaths[ImportLibraryKey] = this.CreateTokenizedString("$(VulkanLibDir)/$(libprefix)$(OutputName)$(libext)");
            }

            var headers = this.CreateHeaderContainer();
            headers.Macros["packagedir"].Set(latest_version_path, null); // must set this as well as on this, since it doesn't inherit
            headers.AddFile("$(packagedir)/Include/vulkan/*.h");
            headers.AddFile("$(packagedir)/Include/vulkan/*.hpp");

            this.PublicPatch((settings, appliedTo) =>
                {
                    var compiler = settings as C.ICommonCompilerSettings;
                    if (null != compiler)
                    {
                        compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/Include"));

                        if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.Windows))
                        {
                            compiler.PreprocessorDefines.Add("VK_USE_PLATFORM_WIN32_KHR");
                        }
                        else if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.Linux))
                        {
                            compiler.PreprocessorDefines.Add("VK_USE_PLATFORM_XLIB_KHR");
                        }
                    }
                });
        }
    }
}
