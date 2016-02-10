#region License
// <copyright>
//  Mark Final
// </copyright>
// <author>Mark Final</author>
#endregion // License
using Bam.Core;
namespace glew
{
    [Bam.Core.ModuleGroup("Thirdparty/GLEW")]
    sealed class GLEWStatic :
        C.StaticLibrary
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            this.CreateHeaderContainer("$(packagedir)/include/GL/*.h");
            var source = this.CreateCSourceContainer();
            source.AddFile("$(packagedir)/src/glew.c");
            this.PublicPatch((settings, appliedTo) =>
                {
                    var compiler = settings as C.ICommonCompilerSettings;
                    if (null != compiler)
                    {
                        compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/include"));
                        compiler.PreprocessorDefines.Add("GLEW_STATIC");
                        compiler.PreprocessorDefines.Add("GLEW_NO_GLU");
                    }
                });

            if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.Windows))
            {
                this.CompileAgainst<WindowsSDK.WindowsSDK>(source);
            }
        }
    }
}
