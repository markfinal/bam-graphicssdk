using Bam.Core;
namespace SPIRVCross
{
    class SPIRVCross : C.StaticLibrary
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var header = this.CreateHeaderContainer("$(packagedir)/**.h");
            header.AddFiles("$(packagedir)/**.hpp");

            var source = this.CreateCxxSourceContainer("$(packagedir)/*.cpp");
            source.PrivatePatch(settings =>
            {
                var cxx_compiler = settings as C.ICxxOnlyCompilerSettings;
                cxx_compiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;
                cxx_compiler.ExceptionHandler = C.Cxx.EExceptionHandler.Asynchronous;

                var clang_compiler = settings as ClangCommon.ICommonCompilerSettings;
                if (null != clang_compiler)
                {
                    clang_compiler.AllWarnings = true;
                    clang_compiler.ExtraWarnings = true;
                    clang_compiler.Pedantic = true;
                }
            });

            this.PublicPatch((Settings, appliedTo) =>
            {
                var compiler = Settings as C.ICommonCompilerSettings;
                if (null != compiler)
                {
                    compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)"));
                }
            });
        }
    }
}