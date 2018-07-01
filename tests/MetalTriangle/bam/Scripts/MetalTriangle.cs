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
            this.CompileAndLinkAgainst<WindowLibrary.GraphicsWindow>(source);

            this.PrivatePatch(settings =>
            {
                var cxxLinker = settings as C.ICxxOnlyLinkerSettings;
                cxxLinker.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;

                var osxLinker = settings as C.ICommonLinkerSettingsOSX;
                osxLinker.Frameworks.AddUnique("Cocoa");
                osxLinker.Frameworks.AddUnique("Metal");
            });
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
