using Bam.Core;
namespace MoltenVK
{
    public sealed class MoltenVKConfigureOSX :
        Bam.Core.IPackageMetaDataConfigure<Clang.MetaData>
    {
        void
        Bam.Core.IPackageMetaDataConfigure<Clang.MetaData>.Configure(
            Clang.MetaData instance)
        {
            instance.MinimumVersionSupported = "macosx10.9";
        }
    }

    sealed class MoltenVK : C.Cxx.DynamicLibrary
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var cxx_source = this.CreateObjectiveCxxSourceContainer();
            cxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/Commands/*.mm");
            cxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/GPUObjects/*.mm");
            cxx_source.AddFiles("$(packagedir)/MoltenVK/MoltenVK/Utility/*.mm");

            this.CompileAgainst<VulkanHeaders.VkHeaders>(cxx_source);
            this.CompileAgainst<SPIRVCross.SPIRVCross>(cxx_source);

            cxx_source.PrivatePatch(settings =>
            {
                var compiler = settings as C.ICommonCompilerSettings;
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/Common"));
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/API"));
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/Commands"));
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/GPUObjects"));
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/Loader"));
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVK/MoltenVK/Utility"));
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/MoltenVKShaderConverter"));

                compiler.DisableWarnings.AddUnique("unguarded-availability-new"); // MoltenVK-1.0.10/MoltenVK/MoltenVK/Commands/MVKCmdTransfer.mm:582:19: error: 'dispatchThreads:threadsPerThreadgroup:' is only available on macOS 10_13 or newer [-Werror,-Wunguarded-availability-new]

                var cxx_compiler = settings as C.ICxxOnlyCompilerSettings;
                cxx_compiler.StandardLibrary = C.Cxx.EStandardLibrary.libcxx;
                cxx_compiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;
            });

            this.PrivatePatch(settings =>
            {
                var macos_linker = settings as C.ICommonLinkerSettingsOSX;
                macos_linker.Frameworks.AddUnique("Metal");
                macos_linker.Frameworks.AddUnique("Foundation");
                macos_linker.Frameworks.AddUnique("IOKit");
            });
        }
    }
}
