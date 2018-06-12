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
            this.Macros.Add("GenerateGrammarTablesPy", "$(packagedir)/utils/generate_grammar_tables.py");
            this.Macros.Add("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/spirv.core.grammar.json", new []{spirvheaders.Macros["IncludeDir"]}));
            this.Macros.Add("DebugGrammarFile", "$(packagedir)/source/extinst.debuginfo.grammar.json");
            this.Macros.Add("GrammarEnumStringMappingIncFile", "$(packagebuilddir)/$(moduleoutputdir)/enum_string_mapping.inc");

            var arguments = new System.Text.StringBuilder();
            arguments.Append("$(GenerateGrammarTablesPy) ");
            arguments.Append("--spirv-core-grammar=$(GrammarJsonFile) ");
            arguments.Append("--extinst-debuginfo-grammar=$(DebugGrammarFile) ");
            arguments.Append("--extension-enum-output=$(0) ");
            arguments.Append("--enum-string-mapping-output=$(GrammarEnumStringMappingIncFile) ");
            this.Macros.Add("Arguments", this.CreateTokenizedString(arguments.ToString(), new [] {this.OutputPath}));
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

    class DebugInfo : C.ProceduralHeaderFile
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.Add("GenerateLanguageHeadersPy", "$(packagedir)/utils/generate_language_headers.py");
            this.Macros.Add("DebugGrammarFile", "$(packagedir)/source/extinst.debuginfo.grammar.json");

            var arguments = new System.Text.StringBuilder();
            arguments.Append("$(GenerateLanguageHeadersPy) ");
            arguments.Append("--extinst-name=DebugInfo ");
            arguments.Append("--extinst-grammar=$(DebugGrammarFile) ");
            arguments.Append("--extinst-output-base=$(packagebuilddir)/$(moduleoutputdir)/DebugInfo ");
            this.Macros.Add("Arguments", this.CreateTokenizedString(arguments.ToString()));
        }

        protected override Bam.Core.TokenizedString OutputPath
        {
            get
            {
                return this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/DebugInfo.h");
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

            var debugInfoHeader = Bam.Core.Graph.Instance.FindReferencedModule<DebugInfo>();
            source.DependsOn(debugInfoHeader);
            source.UsePublicPatches(debugInfoHeader);

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
