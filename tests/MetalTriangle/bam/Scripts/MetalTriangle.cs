using MetalExtensions;
namespace MetalTriangle
{
    class MetalTest :
        C.Cxx.GUIApplication
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

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
            shaderLibrary.Macros["OutputName"] = Bam.Core.TokenizedString.CreateVerbatim("default");

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
            if (Bam.Core.Graph.Instance.Mode != "Xcode")
            {
                // not on Xcode, since this auto-generates the Metal shader library
                this.registerMetalMappings();
            }

            this.Include<MetalTest>(C.Cxx.GUIApplication.ExecutableKey);
        }
    }
}
