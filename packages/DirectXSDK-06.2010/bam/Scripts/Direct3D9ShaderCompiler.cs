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
namespace DirectXSDK
{
    [C.Prebuilt]
    sealed class Direct3D9ShaderCompiler :
        C.Cxx.DynamicLibrary
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var meta = this.PackageDefinition.MetaData as IDirectXSDKInstallMeta;
            if (meta.UseWindowsSDK)
            {
                var vcMetaData = Bam.Core.Graph.Instance.PackageMetaData<VisualC.MetaData>("VisualC");
                var vcEnv = vcMetaData.Environment(this.BitDepth);
                if (vcEnv.ContainsKey("WindowsSdkDir"))
                {
                    System.Diagnostics.Debug.Assert(1 == vcEnv["WindowsSdkDir"].Count, "Only one WindowsSDK directory");
                    if (this.BitDepth == C.EBit.SixtyFour)
                    {
                        this.GeneratedPaths[C.DynamicLibrary.Key] = this.CreateTokenizedString("$(0)/Redist/D3D/x64/d3dcompiler_47$(dynamicext)", vcEnv["WindowsSdkDir"].ToArray());
                    }
                    else if (this.BitDepth == C.EBit.ThirtyTwo)
                    {
                        this.GeneratedPaths[C.DynamicLibrary.Key] = this.CreateTokenizedString("$(0)/Redist/D3D/x86/d3dcompiler_47$(dynamicext)", vcEnv["WindowsSdkDir"].ToArray());
                    }
                }
                else
                {
                    throw new Bam.Core.Exception("Unable to determine WindowsSDK installation directory in order to find the D3D9 shader compiler");
                }
            }
            else
            {
                throw new Bam.Core.Exception("DXSDK shader compiler is part of the redist");
            }
        }
    }
}
