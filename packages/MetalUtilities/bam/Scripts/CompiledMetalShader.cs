using System.Linq;
namespace MetalUtilities
{
    class CompiledMetalShader :
        Bam.Core.Module
    {
        public const string CompiledMetalShaderKey = "Compiled Metal shader";

        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

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
                        var encapsulating = this.GetEncapsulatingReferencedModule();
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

        public override System.Collections.Generic.IEnumerable<System.Collections.Generic.KeyValuePair<string, Bam.Core.Module>> InputModules
        {
            get
            {
                yield return new System.Collections.Generic.KeyValuePair<string, Bam.Core.Module>(
                    MetalShaderSource.ShaderSourceKey,
                    this.ShaderSource
                );
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

        public override Bam.Core.Settings CreateDefaultSettings<T>(T module)
        {
            return new MetalShaderCompilerSettings(module);
        }

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
