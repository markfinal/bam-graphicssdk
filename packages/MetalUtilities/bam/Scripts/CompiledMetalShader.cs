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
namespace MetalUtilities
{
    class CompiledMetalShader :
        Bam.Core.Module
    {
        public const string CompiledMetalShaderKey = "Compiled Metal shader";

        protected override void
        Init()
        {
            base.Init();

            this.Tool = Bam.Core.Graph.Instance.FindReferencedModule<MetalShaderCompilerTool>();

            this.RegisterGeneratedFile(
                CompiledMetalShaderKey,
                this.CreateTokenizedString(
                    "$(packagebuilddir)/$(moduleoutputdir)/@changeextension(@trimstart(@relativeto($(0),$(packagedir)),../),.air)",
                    (this.ShaderSource as Bam.Core.IInputPath).InputPath
                )
            );
        }

        protected override void
        EvaluateInternal()
        {
            // always execute
        }

        protected override void
        ExecuteInternal(
            Bam.Core.ExecutionContext context)
        {
            switch (Bam.Core.Graph.Instance.Mode)
            {
#if D_PACKAGE_NATIVEBUILDER
                case "MakeFile":
                    MakeFileBuilder.Support.Add(this);
                    break;
#endif

#if D_PACKAGE_NATIVEBUILDER
                case "Native":
                    NativeBuilder.Support.RunCommandLineTool(this, context);
                    break;
#endif
#if D_PACKAGE_XCODEBUILDER
                case "Xcode":
                    {
                        var encapsulating = this.EncapsulatingModule;
                        var workspace = Bam.Core.Graph.Instance.MetaData as XcodeBuilder.WorkspaceMeta;
                        var target = workspace.EnsureTargetExists(encapsulating);
                        var configuration = target.GetConfiguration(encapsulating);
                        var buildFile = target.EnsureSourceBuildFileExists(
                            this.ShaderSource.GeneratedPaths[MetalShaderSource.ShaderSourceKey],
                            XcodeBuilder.FileReference.EFileType.MetalShaderSource
                        );
                        configuration.BuildFiles.Add(buildFile);
                    }
                    break;
#endif
            }
        }

        public MetalShaderSource ShaderSource
        {
            get;
            set;
        }

        public override System.Collections.Generic.IEnumerable<(Bam.Core.Module module, string pathKey)> InputModulePaths
        {
            get
            {
                yield return (this.ShaderSource, MetalShaderSource.ShaderSourceKey);
            }
        }
    }

    [CommandLineProcessor.OutputPath(CompiledMetalShader.CompiledMetalShaderKey, "-o ")]
    [CommandLineProcessor.InputPaths(MetalShaderSource.ShaderSourceKey, "-c ", max_file_count: 1)]
    class MetalShaderCompilerSettings :
        Bam.Core.Settings
    {
        public MetalShaderCompilerSettings(
            Bam.Core.Module module)
        {
            this.InitializeAllInterfaces(module, false, true);
        }

        public override void
        AssignFileLayout()
        {
            this.FileLayout = ELayout.Cmds_Inputs_Outputs;
        }
    }

    class MetalShaderCompilerTool :
        Bam.Core.PreBuiltTool
    {
        private static Bam.Core.TokenizedString executablePath;
        private Bam.Core.TokenizedStringArray arguments = new Bam.Core.TokenizedStringArray();

        static MetalShaderCompilerTool()
        {
            executablePath = Bam.Core.TokenizedString.CreateVerbatim(Bam.Core.OSUtilities.GetInstallLocation("xcrun").First());
        }

        public MetalShaderCompilerTool()
        {
            var clangMeta = Bam.Core.Graph.Instance.PackageMetaData<Bam.Core.PackageMetaData>("Clang");
            var discovery = clangMeta as C.IToolchainDiscovery;
            discovery.discover(null);

            this.arguments.Add(Bam.Core.TokenizedString.CreateVerbatim(System.String.Format("--sdk {0}", clangMeta["SDK"]))); // could use clangMeta.SDK, but avoids compile-time dependency on the Clang packages
            this.arguments.Add(Bam.Core.TokenizedString.CreateVerbatim("metal"));
        }

        /// <summary>
        /// \copydoc Bam.Core.ITool.SettingsType
        /// </summary>
        public override System.Type SettingsType => typeof(MetalShaderCompilerSettings);

        public override Bam.Core.TokenizedString Executable
        {
            get
            {
                return executablePath;
            }
        }

        public override Bam.Core.TokenizedStringArray InitialArguments
        {
            get
            {
                return this.arguments;
            }
        }
    }
}
