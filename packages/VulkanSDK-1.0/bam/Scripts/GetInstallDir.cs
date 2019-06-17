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
using System.Linq;
using Bam.Core;
namespace VulkanSDK
{
    static class GetInstallDir
    {
        public static string
        Find(
            Bam.Core.EPlatform platform)
        {
            var latest_version_path = System.String.Empty;
            if (platform.Includes(Bam.Core.EPlatform.Windows))
            {
                using (var key = Bam.Core.Win32RegistryUtilities.OpenLMSoftwareKey(@"LunarG\VulkanSDK"))
                {
                    if (null == key)
                    {
                        throw new Bam.Core.Exception("Unable to locate any Vulkan SDK installations");
                    }
                    var linearised_paths = key.GetStringValue("VK_SDK_PATHs");
                    var paths = new Bam.Core.StringArray(linearised_paths.Split(new[] { ';' }, System.StringSplitOptions.RemoveEmptyEntries));
                    System.Diagnostics.Debug.Assert(paths.Count > 0);
                    var applicable_paths = new System.Collections.Generic.SortedDictionary<string, string>();
                    foreach (var path in paths)
                    {
                        var dir_split = path.Split(new[] { System.IO.Path.DirectorySeparatorChar }, System.StringSplitOptions.RemoveEmptyEntries);
                        var version = dir_split[dir_split.Length - 1];
                        if (version.StartsWith("1.0", System.StringComparison.Ordinal))
                        {
                            applicable_paths.Add(version, path);
                        }
                    }
                    System.Diagnostics.Debug.Assert(applicable_paths.Count > 0);
                    latest_version_path = applicable_paths.First().Value;
                }
            }
            else if (platform.Includes(Bam.Core.EPlatform.Linux))
            {
                // TODO: is this compatible with the Linux installer?
                latest_version_path = System.Environment.ExpandEnvironmentVariables("VK_SDK_PATH");
                System.Diagnostics.Debug.Assert(latest_version_path.StartsWith("1.0", System.StringComparison.Ordinal));
            }
            Bam.Core.Log.Info($"Using VulkanSDK installed at {latest_version_path}");
            return latest_version_path;
        }
    }
}
