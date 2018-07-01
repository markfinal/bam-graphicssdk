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

            var source = this.CreateObjectiveCxxSourceContainer("$(packagedir)/source/*.mm");
            source.AddFiles("$(packagedir)/resources/*.metal");
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

            this.PrivatePatch(settings =>
            {
                var cxxLinker = settings as C.ICxxOnlyLinkerSettings;
                cxxLinker.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;

                var osxLinker = settings as C.ICommonLinkerSettingsOSX;
                osxLinker.Frameworks.AddUnique("Cocoa");
                osxLinker.Frameworks.AddUnique("Metal");
                osxLinker.Frameworks.AddUnique("MetalKit");
                osxLinker.Frameworks.AddUnique("QuartzCore"); // including Core Animation
                osxLinker.MinimumVersionSupported = "macos10.9";
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
            this.Include<MetalTest>(C.Cxx.GUIApplication.Key);
        }
    }
}
