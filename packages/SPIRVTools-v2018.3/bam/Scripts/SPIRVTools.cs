using Bam.Core;
using System.Linq;
namespace SPIRVTools
{
    class PythonSourceGenerator :
        C.ExternalSourceGenerator
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            this.Executable = Bam.Core.TokenizedString.CreateVerbatim(Bam.Core.OSUtilities.GetInstallLocation("python").FirstOrDefault());

            this.PublicPatch((settings, appliedTo) =>
            {
                var compiler = settings as C.ICommonCompilerSettings;
                if (null != compiler)
                {
                    compiler.IncludePaths.AddUnique(this.OutputDirectory);
                }
            });
        }
    }

    class ExtensionEnumInc :
        PythonSourceGenerator
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.AddVerbatim("Version", "unified1");

            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/generate_grammar_tables.py"));
            this.AddInputFile("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/spirv.core.grammar.json", new[] { spirvheaders.Macros["IncludeDir"] }));
            this.AddInputFile("DebugGrammarFile", this.CreateTokenizedString("$(packagedir)/source/extinst.debuginfo.grammar.json"));

            this.OutputDirectory = this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)");
            this.Macros.Add("GrammarExtensionEnumIncFile", "$(packagebuilddir)/$(moduleoutputdir)/extension_enum.inc");
            this.Macros.Add("GrammarEnumStringMappingIncFile", "$(packagebuilddir)/$(moduleoutputdir)/enum_string_mapping.inc");
            this.ExpectedOutputFiles.Add(this.CreateTokenizedString("$(GrammarExtensionEnumIncFile)"));
            this.ExpectedOutputFiles.Add(this.CreateTokenizedString("$(GrammarEnumStringMappingIncFile)"));

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("--spirv-core-grammar=$(GrammarJsonFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-debuginfo-grammar=$(DebugGrammarFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--extension-enum-output=$(GrammarExtensionEnumIncFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--enum-string-mapping-output=$(GrammarEnumStringMappingIncFile)"));
        }
    }

    class DebugInfo :
        PythonSourceGenerator
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/generate_language_headers.py"));
            this.AddInputFile("DebugGrammarFile", this.CreateTokenizedString("$(packagedir)/source/extinst.debuginfo.grammar.json"));

            this.OutputDirectory = this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)");
            this.ExpectedOutputFiles.Add(this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/DebugInfo.h"));

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-name=DebugInfo"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-grammar=$(DebugGrammarFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-output-base=$(packagebuilddir)/$(moduleoutputdir)/DebugInfo"));
        }
    }

    class GLSLTables :
        PythonSourceGenerator
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.AddVerbatim("Version", "unified1");

            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/generate_grammar_tables.py"));
            this.AddInputFile("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/extinst.glsl.std.450.grammar.json", new[] { spirvheaders.Macros["IncludeDir"] }));

            this.OutputDirectory = this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)");
            this.Macros.Add("GLSLHeader", this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/glsl.std.450.insts.inc"));
            this.ExpectedOutputFiles.Add(this.CreateTokenizedString("$(GLSLHeader)"));

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-glsl-grammar=$(GrammarJsonFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--glsl-insts-output=$(GLSLHeader)"));
        }
    }

    class OpenCLTables :
        PythonSourceGenerator
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.AddVerbatim("Version", "unified1");
            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/generate_grammar_tables.py"));
            this.AddInputFile("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/extinst.opencl.std.100.grammar.json", new[] { spirvheaders.Macros["IncludeDir"] }));

            this.OutputDirectory = this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)");
            this.Macros.Add("OpenCLHeader", this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/opencl.std.insts.inc"));
            this.ExpectedOutputFiles.Add(this.CreateTokenizedString("$(OpenCLHeader)"));

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-opencl-grammar=$(GrammarJsonFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--opencl-insts-output=$(OpenCLHeader)"));
        }
    }

    class GenerateVendorTables :
        PythonSourceGenerator
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/generate_grammar_tables.py"));
            this.AddInputFile("DebugGrammarFile", this.CreateTokenizedString("$(packagedir)/source/extinst.$(TableName).grammar.json"));

            this.OutputDirectory = this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)");
            this.Macros.Add("VendorTable", this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/$(TableName).insts.inc"));
            this.ExpectedOutputFiles.Add(this.Macros["VendorTable"]);

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-vendor-grammar=$(DebugGrammarFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--vendor-insts-output=$(VendorTable)"));
        }

        public string TableName
        {
            set
            {
                this.Macros.AddVerbatim("TableName", value);
            }
        }
    }

    class CoreTables :
        PythonSourceGenerator
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.AddVerbatim("Version", "unified1");
            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/generate_grammar_tables.py"));
            this.AddInputFile("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/spirv.core.grammar.json", new[] { spirvheaders.Macros["IncludeDir"] }));
            this.AddInputFile("DebugGrammarFile", this.CreateTokenizedString("$(packagedir)/source/extinst.debuginfo.grammar.json"));

            this.OutputDirectory = this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)");
            this.Macros.Add("CoreInstIncFile", this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/core.insts-$(Version).inc"));
            this.Macros.Add("GrammarKindsIncFile", this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/operand.kinds-$(Version).inc"));
            this.ExpectedOutputFiles.Add(this.Macros["CoreInstIncFile"]);
            this.ExpectedOutputFiles.Add(this.Macros["GrammarKindsIncFile"]);

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("--spirv-core-grammar=$(GrammarJsonFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-debuginfo-grammar=$(DebugGrammarFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--core-insts-output=$(CoreInstIncFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--operand-kinds-output=$(GrammarKindsIncFile)"));
        }
    }

    class Generators :
        PythonSourceGenerator
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/generate_registry_tables.py"));
            this.AddInputFile("XmlRegistryFile", this.CreateTokenizedString("$(0)/spirv/spir-v.xml", new[] { spirvheaders.Macros["IncludeDir"] }));

            this.OutputDirectory = this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)");
            this.Macros.Add("GeneratorsInc", this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/generators.inc"));
            this.ExpectedOutputFiles.Add(this.Macros["GeneratorsInc"]);

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("--xml=$(XmlRegistryFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--generator-output=$(GeneratorsInc)"));
        }
    }

    class BuildVersion :
        PythonSourceGenerator
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/update_build_version.py"));

            this.OutputDirectory = this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)");
            this.Macros.Add("BuildVersionInc", this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/build-version.inc"));
            this.ExpectedOutputFiles.Add(this.Macros["BuildVersionInc"]);

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("$(packagedir)"));
            this.Arguments.Add(this.CreateTokenizedString("$(BuildVersionInc)"));
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
            source.AddFiles("$(packagedir)/source/util/*.cpp");

            this.CompileAgainst<SPIRVHeaders.SPIRVHeaders>(source);

            var extensionEnumInc = Bam.Core.Graph.Instance.FindReferencedModule<ExtensionEnumInc>();
            source.DependsOn(extensionEnumInc);
            source.UsePublicPatches(extensionEnumInc);

            var debugInfoHeader = Bam.Core.Graph.Instance.FindReferencedModule<DebugInfo>();
            source.DependsOn(debugInfoHeader);
            source.UsePublicPatches(debugInfoHeader);

            var glslTablesInc = Bam.Core.Graph.Instance.FindReferencedModule<GLSLTables>();
            source.DependsOn(glslTablesInc);
            source.UsePublicPatches(glslTablesInc);

            var openclTablesInc = Bam.Core.Graph.Instance.FindReferencedModule<OpenCLTables>();
            source.DependsOn(openclTablesInc);
            source.UsePublicPatches(openclTablesInc);

            var vendorTables = new Bam.Core.StringArray(
                "debuginfo",
                "spv-amd-gcn-shader",
                "spv-amd-shader-ballot",
                "spv-amd-shader-explicit-vertex-parameter",
                "spv-amd-shader-trinary-minmax"
            );
            foreach (var vt in vendorTables)
            {
                var vendorTablesInc = Bam.Core.Module.Create<GenerateVendorTables>(preInitCallback: module =>
                {
                    module.TableName = vt;
                });
                source.DependsOn(vendorTablesInc);
                source.UsePublicPatches(vendorTablesInc);
            }

            var coreTables = Bam.Core.Graph.Instance.FindReferencedModule<CoreTables>();
            source.DependsOn(coreTables);
            source.UsePublicPatches(coreTables);

            var generators = Bam.Core.Graph.Instance.FindReferencedModule<Generators>();
            source.DependsOn(generators);
            source.UsePublicPatches(generators);

            var buildVersion = Bam.Core.Graph.Instance.FindReferencedModule<BuildVersion>();
            source.DependsOn(buildVersion);
            source.UsePublicPatches(buildVersion);

            source.PrivatePatch(settings =>
            {
                var compiler = settings as C.ICommonCompilerSettings;
                compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/source"));

                var cxx_compiler = settings as C.ICxxOnlyCompilerSettings;
                cxx_compiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;

                var clang_compiler = settings as ClangCommon.ICommonCompilerSettings;
                if (null != clang_compiler)
                {
                    clang_compiler.AllWarnings = true;
                    clang_compiler.ExtraWarnings = true;
                    clang_compiler.Pedantic = true;
                }
            });

            this.PublicPatch((settings, appliedTo) =>
            {
                var compiler = settings as C.ICommonCompilerSettings;
                if (null != compiler)
                {
                    compiler.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/include"));
                }
            });
        }
    }
}
