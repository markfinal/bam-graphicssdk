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
namespace DirectXSDK
{
    sealed class Direct3D9 :
        C.CSDKModule
    {
        public Direct3D9()
        {
            var meta = this.PackageDefinition.MetaData as IDirectXSDKInstallMeta;
            if (!meta.UseWindowsSDK)
            {
                this.Macros.Add("InstallPath", Bam.Core.TokenizedString.CreateVerbatim(this.PackageDefinition.MetaData["InstallPath"] as string));
                this.Macros.Add("IncludePath", this.CreateTokenizedString("$(InstallPath)/include"));
                this.Macros.Add("LibraryPath", this.CreateTokenizedString("$(InstallPath)/lib"));
            }
        }

        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var meta = this.PackageDefinition.MetaData as IDirectXSDKInstallMeta;
            if (meta.UseWindowsSDK)
            {
                Bam.Core.Log.Info("Using DirectX SDK installed as part of the WindowsSDK");
            }
            else
            {
                Bam.Core.Log.Info($"Using DirectXSDK installed at {meta.InstallPath}");
                this.PublicPatch((settings, appliedTo) =>
                    {
                        if (settings is C.ICommonPreprocessorSettings preprocessor)
                        {
                            preprocessor.IncludePaths.Add(this.Macros["IncludePath"]);
                        }

                        if (settings is C.ICommonLinkerSettings linker)
                        {
                            if ((appliedTo as C.CModule).BitDepth == C.EBit.ThirtyTwo)
                            {
                                linker.LibraryPaths.Add(this.CreateTokenizedString("$(LibraryPath)/x86"));
                            }
                            else
                            {
                                linker.LibraryPaths.Add(this.CreateTokenizedString("$(LibraryPath)/x64"));
                            }
                        }
                    });
            }
        }

        protected override void
        EvaluateInternal()
        {
            this.ReasonToExecute = null;
        }

        protected override void
        ExecuteInternal(
            Bam.Core.ExecutionContext context)
        {
            // do nothing
        }
    }
}
