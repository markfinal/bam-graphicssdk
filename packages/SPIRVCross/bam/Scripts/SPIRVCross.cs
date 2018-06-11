using Bam.Core;
namespace SPIRVCross
{
    class SPIRVCross : C.HeaderLibrary
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var header = this.CreateHeaderContainer("$(packagedir)/**.h");
            header.AddFiles("$(packagedir)/**.hpp");

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
