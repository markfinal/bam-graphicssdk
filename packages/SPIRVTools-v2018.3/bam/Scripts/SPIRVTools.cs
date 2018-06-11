using Bam.Core;
namespace SPIRVTools
{
    class SPIRVTools : C.StaticLibrary
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var header = this.CreateHeaderContainer("$(packagedir)/include/**.h");
            header.AddFiles("$(packagedir)/include/**.hpp");

            var source = this.CreateCxxSourceContainer("$(packagedir)/source/*.cpp");
            this.CompileAgainst<SPIRVHeaders.SPIRVHeaders>(source);

            source.PrivatePatch(settings =>
            {
                var cxx_compiler = settings as C.ICxxOnlyCompilerSettings;
                cxx_compiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;
            });

            this.PublicPatch((Settings, appliedTo) =>
            {
                var compiler = Settings as C.ICommonCompilerSettings;
                if (null != compiler)
                {
                    compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/include"));
                }
            });
        }
    }
}
