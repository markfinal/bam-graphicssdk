using MetalExtensions;
using System.Linq;
namespace MetalTriangle
{
    class MetalShaderSource :
        Bam.Core.Module,
        Bam.Core.IInputPath
    {
        public const string ShaderSourceKey = "Metal shader source";

        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            this.RegisterGeneratedFile(
                ShaderSourceKey,
                (this as Bam.Core.IInputPath).InputPath
            );
        }

        protected override void
        EvaluateInternal()
        {
            // never execute, as always a file on disk
            this.ReasonToExecute = null;
        }

        protected override void ExecuteInternal(
            Bam.Core.ExecutionContext context)
        {
            // do nothing, as the file always exists
        }

        Bam.Core.TokenizedString Bam.Core.IInputPath.InputPath
        {
            get;
            set;
        }
    }

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
                        target.EnsureFileOfTypeExists(
                            this.ShaderSource.GeneratedPaths[MetalShaderSource.ShaderSourceKey],
                            XcodeBuilder.FileReference.EFileType.MetalShaderSource
                        );
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

    class MetalShaderLibrary :
        Bam.Core.Module,
        Bam.Core.IModuleGroup
    {
        public const string ShaderLibraryKey = "Metal shader library";

        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);
            this.Tool = Bam.Core.Graph.Instance.FindReferencedModule<MetalShaderLibraryTool>();
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

    [CommandLineProcessor.OutputPath(MetalShaderLibrary.ShaderLibraryKey, "-o ")]
    [CommandLineProcessor.InputPaths(CompiledMetalShader.CompiledMetalShaderKey, "")]
    class MetalShaderLibrarySettings :
        Bam.Core.Settings
    {
        public MetalShaderLibrarySettings(
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

    class MetalShaderLibraryTool :
        Bam.Core.PreBuiltTool
    {
        private static Bam.Core.TokenizedString executablePath;
        private Bam.Core.TokenizedStringArray arguments = new Bam.Core.TokenizedStringArray();

        static MetalShaderLibraryTool()
        {
            executablePath = Bam.Core.TokenizedString.CreateVerbatim(Bam.Core.OSUtilities.GetInstallLocation("xcrun").First());
        }

        public MetalShaderLibraryTool()
        {
            var clangMeta = Bam.Core.Graph.Instance.PackageMetaData<Bam.Core.PackageMetaData>("Clang");
            var discovery = clangMeta as C.IToolchainDiscovery;
            discovery.discover(null);

            this.arguments.Add(Bam.Core.TokenizedString.CreateVerbatim(System.String.Format("--sdk {0}", clangMeta["SDK"]))); // could use clangMeta.SDK, but avoids compile-time dependency on the Clang packages
            this.arguments.Add(Bam.Core.TokenizedString.CreateVerbatim("metallib"));
        }

        public override Bam.Core.Settings CreateDefaultSettings<T>(T module)
        {
            return new MetalShaderLibrarySettings(module);
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

    class MetalTest :
        C.Cxx.GUIApplication
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            // TODO: source probably not required if not procedurally generated
            var shaderSource = Bam.Core.Module.Create<MetalShaderSource>(
                preInitCallback: module =>
                    {
                        (module as Bam.Core.IInputPath).InputPath = this.CreateTokenizedString("$(packagedir)/resources/shaders.metal");
                    }
            );
            var shaderCompiled = Bam.Core.Module.Create<CompiledMetalShader>(
                preInitCallback: module =>
                    {
                        module.ShaderSource = shaderSource;
                        module.DependsOn(shaderSource);
                    }
            );
            var shaderLibrary = Bam.Core.Module.Create<MetalShaderLibrary>();
            shaderLibrary.DependsOn(shaderCompiled);

            var source = this.CreateObjectiveCxxSourceContainer("$(packagedir)/source/*.mm");
            source.PrivatePatch(settings =>
            {
                var compiler = settings as C.ICommonCompilerSettings;
                compiler.WarningsAsErrors = true;

                var cxxCompiler = settings as C.ICxxOnlyCompilerSettings;
                cxxCompiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;
                cxxCompiler.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;

                var clangCompiler = settings as ClangCommon.ICommonCompilerSettings;
                clangCompiler.AllWarnings = true;
                clangCompiler.ExtraWarnings = true;
                clangCompiler.Pedantic = true;
            });

            this.CompileAndLinkAgainst<WindowLibrary.GraphicsWindow>(source);
            this.DependsOn(shaderLibrary);

            this.PrivatePatch(settings =>
            {
                var cxxLinker = settings as C.ICxxOnlyLinkerSettings;
                cxxLinker.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;

                var osxLinker = settings as C.ICommonLinkerSettingsOSX;
                osxLinker.Frameworks.AddUnique("Cocoa");
                osxLinker.Frameworks.AddUnique("Metal");
                osxLinker.Frameworks.AddUnique("MetalKit");
                osxLinker.Frameworks.AddUnique("QuartzCore"); // including Core Animation
                osxLinker.MacOSMinimumVersionSupported = "10.9";
            });

            //this.addMetalResources(this.CreateTokenizedString("$(packagedir)/resources/*.metal"));
        }
    }

    sealed class Runtime :
        Publisher.Collation
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            this.SetDefaultMacrosAndMappings(EPublishingType.WindowedApplication);
            this.Include<MetalTest>(C.Cxx.GUIApplication.ExecutableKey);
        }
    }
}
