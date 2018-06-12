using Bam.Core;
namespace SPIRVTools
{
    class ExtensionEnumInc : C.ProceduralHeaderFile
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.Add("Version", "unified1");
            this.Macros.Add("GenerateGrammarTables", "$(packagedir)/utils/generate_grammar_tables.py");
            this.Macros.Add("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/spirv.core.grammar.json", new []{spirvheaders.Macros["IncludeDir"]}));
            this.Macros.Add("DebugGrammarFile", "$(packagedir)/source/extinst.debuginfo.grammar.json");
            this.Macros.Add("GrammarEnumStringMappingIncFile", "$(packagebuilddir)/$(moduleoutputdir)/enum_string_mapping.inc");

            this.Macros.Add("Arguments", this.CreateTokenizedString("$(GenerateGrammarTables) --spirv-core-grammar=$(GrammarJsonFile) --extinst-debuginfo-grammar=$(DebugGrammarFile) --extension-enum-output=$(0) --enum-string-mapping-output=$(GrammarEnumStringMappingIncFile)", new [] {this.OutputPath}));
        }

        protected override Bam.Core.TokenizedString OutputPath
        {
            get
            {
                return this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/extension_enum.inc");
            }
        }

        protected override string Contents
        {
            get
            {
                var output = Bam.Core.OSUtilities.RunExecutable(
                    "python",
                    this.Macros["Arguments"].ToString()
                );
                Bam.Core.Log.MessageAll("Running 'python {0}'", this.Macros["Arguments"].ToString());
                if (!System.String.IsNullOrEmpty(output))
                {
                    Bam.Core.Log.MessageAll("\t{0}", output);
                }
                return null;
            }
        }
    }

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

            var extensionEnumInc = Bam.Core.Graph.Instance.FindReferencedModule<ExtensionEnumInc>();
            source.DependsOn(extensionEnumInc);
            source.UsePublicPatches(extensionEnumInc);

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
