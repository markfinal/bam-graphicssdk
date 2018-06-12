using Bam.Core;
namespace SPIRVHeaders
{
    class SPIRVHeaders : C.HeaderLibrary
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            this.Macros.Add("IncludeDir", this.CreateTokenizedString("$(packagedir)/include"));

            var headers = this.CreateHeaderContainer("$(packagedir)/include/**.hpp");
            headers.AddFiles("$(packagedir)/include/**.h");

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
