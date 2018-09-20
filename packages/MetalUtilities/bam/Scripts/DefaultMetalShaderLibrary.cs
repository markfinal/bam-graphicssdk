using System.Linq;
namespace MetalUtilities
{
    class DefaultMetalShaderLibrary :
        Bam.Core.Module,
        Bam.Core.IModuleGroup
    {
        public const string ShaderLibraryKey = "Metal shader library";

        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            // for consistency with what Xcode will generate
            this.Macros["OutputName"] = Bam.Core.TokenizedString.CreateVerbatim("default");

            this.Tool = Bam.Core.Graph.Instance.FindReferencedModule<MetalShaderLinkerTool>();
            this.RegisterGeneratedFile(
                ShaderLibraryKey,
                this.CreateTokenizedString(
                    "$(packagebuilddir)/$(moduleoutputdir)/$(OutputName).metallib"
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
                    // Xcode automatically generates default.metallib into the application
                    // bundle's Resources folder
#endif
            }
        }

        public override System.Collections.Generic.IEnumerable<System.Collections.Generic.KeyValuePair<string, Bam.Core.Module>> InputModules
        {
            get
            {
                foreach (var dep in this.Dependents)
                {
                    if (!(dep is CompiledMetalShader))
                    {
                        continue;
                    }
                    yield return new System.Collections.Generic.KeyValuePair<string, Bam.Core.Module>(
                        CompiledMetalShader.CompiledMetalShaderKey,
                        dep
                    );
                }
            }
        }
    }

    [CommandLineProcessor.OutputPath(DefaultMetalShaderLibrary.ShaderLibraryKey, "-o ")]
    [CommandLineProcessor.InputPaths(CompiledMetalShader.CompiledMetalShaderKey, "")]
    class MetalShaderLinkerSettings :
        Bam.Core.Settings
    {
        public MetalShaderLinkerSettings(
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

    class MetalShaderLinkerTool :
        Bam.Core.PreBuiltTool
    {
        private static Bam.Core.TokenizedString executablePath;
        private Bam.Core.TokenizedStringArray arguments = new Bam.Core.TokenizedStringArray();

        static MetalShaderLinkerTool()
        {
            executablePath = Bam.Core.TokenizedString.CreateVerbatim(Bam.Core.OSUtilities.GetInstallLocation("xcrun").First());
        }

        public MetalShaderLinkerTool()
        {
            var clangMeta = Bam.Core.Graph.Instance.PackageMetaData<Bam.Core.PackageMetaData>("Clang");
            var discovery = clangMeta as C.IToolchainDiscovery;
            discovery.discover(null);

            this.arguments.Add(Bam.Core.TokenizedString.CreateVerbatim(System.String.Format("--sdk {0}", clangMeta["SDK"]))); // could use clangMeta.SDK, but avoids compile-time dependency on the Clang packages
            this.arguments.Add(Bam.Core.TokenizedString.CreateVerbatim("metallib"));
        }

        public override Bam.Core.Settings CreateDefaultSettings<T>(T module)
        {
            return new MetalShaderLinkerSettings(module);
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
