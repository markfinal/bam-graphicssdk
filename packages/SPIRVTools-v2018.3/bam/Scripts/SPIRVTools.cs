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
            this.Macros.Add("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/spirv.core.grammar.json", new[] { spirvheaders.Macros["IncludeDir"] }));
            this.Macros.Add("DebugGrammarFile", "$(packagedir)/source/extinst.debuginfo.grammar.json");
            this.Macros.Add("GrammarExtensionEnumIncFile", "$(packagebuilddir)/$(moduleoutputdir)/extension_enum.inc");
            this.Macros.Add("GrammarEnumStringMappingIncFile", "$(packagebuilddir)/$(moduleoutputdir)/enum_string_mapping.inc");

            this.OutputDirectory = this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)");

            this.ExpectedOutputFiles.Add(this.CreateTokenizedString("$(GrammarExtensionEnumIncFile)"));
            this.ExpectedOutputFiles.Add(this.CreateTokenizedString("$(GrammarEnumStringMappingIncFile)"));

            this.Arguments.Add(this.CreateTokenizedString("$(packagedir)/utils/generate_grammar_tables.py"));
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

            this.Macros.Add("PyScript", "");
            this.Macros.Add("DebugGrammarFile", "$(packagedir)/source/extinst.debuginfo.grammar.json");

            this.OutputDirectory = this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)");

            this.ExpectedOutputFiles.Add(this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/DebugInfo.h"));

            this.Arguments.Add(this.CreateTokenizedString("$(packagedir)/utils/generate_language_headers.py"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-name=DebugInfo"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-grammar=$(DebugGrammarFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-output-base=$(packagebuilddir)/$(moduleoutputdir)/DebugInfo"));
        }
    }

    class GLSLTables : C.ProceduralHeaderFile
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.Add("PyScript", "$(packagedir)/utils/generate_grammar_tables.py");
            this.Macros.Add("Version", "unified1");
            this.Macros.Add("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/extinst.glsl.std.450.grammar.json", new[] { spirvheaders.Macros["IncludeDir"] }));

            var arguments = new System.Text.StringBuilder();
            arguments.Append("$(PyScript) ");
            arguments.Append("--extinst-glsl-grammar=$(GrammarJsonFile) ");
            arguments.Append("--glsl-insts-output=$(0) ");
            this.Macros.Add("Arguments", this.CreateTokenizedString(arguments.ToString(), new[] { this.OutputPath }));
        }

        protected override Bam.Core.TokenizedString OutputPath
        {
            get
            {
                return this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/glsl.std.450.insts.inc");
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

    class OpenCLTables : C.ProceduralHeaderFile
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.Add("PyScript", "$(packagedir)/utils/generate_grammar_tables.py");
            this.Macros.Add("Version", "unified1");
            this.Macros.Add("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/extinst.opencl.std.100.grammar.json", new[] { spirvheaders.Macros["IncludeDir"] }));

            var arguments = new System.Text.StringBuilder();
            arguments.Append("$(PyScript) ");
            arguments.Append("--extinst-opencl-grammar=$(GrammarJsonFile) ");
            arguments.Append("--opencl-insts-output=$(0) ");
            this.Macros.Add("Arguments", this.CreateTokenizedString(arguments.ToString(), new[] { this.OutputPath }));
        }

        protected override Bam.Core.TokenizedString OutputPath
        {
            get
            {
                return this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/opencl.std.insts.inc");
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

            this.Macros.Add("DebugGrammarFile", this.CreateTokenizedString("$(packagedir)/source/extinst.$(TableName).grammar.json"));
            this.Macros.Add("VendorTable", this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/$(TableName).insts.inc"));

            this.OutputDirectory = this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)");

            this.ExpectedOutputFiles.Add(this.Macros["VendorTable"]);

            this.Arguments.Add(this.CreateTokenizedString("$(packagedir)/utils/generate_grammar_tables.py"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-vendor-grammar=$(packagedir)/source/extinst.$(TableName).grammar.json"));
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

    class CoreTables : C.ProceduralHeaderFile
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.Add("PyScript", "$(packagedir)/utils/generate_grammar_tables.py");
            this.Macros.Add("Version", "unified1");
            this.Macros.Add("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/spirv.core.grammar.json", new[] { spirvheaders.Macros["IncludeDir"] }));
            this.Macros.Add("DebugGrammarFile", "$(packagedir)/source/extinst.debuginfo.grammar.json");
            this.Macros.Add("GrammarKindsIncFile", "$(packagebuilddir)/$(moduleoutputdir)/operand.kinds-$(Version).inc");

            var arguments = new System.Text.StringBuilder();
            arguments.Append("$(PyScript) ");
            arguments.Append("--spirv-core-grammar=$(GrammarJsonFile) ");
            arguments.Append("--extinst-debuginfo-grammar=$(DebugGrammarFile) ");
            arguments.Append("--core-insts-output=$(0) ");
            arguments.Append("--operand-kinds-output=$(GrammarKindsIncFile) ");
            this.Macros.Add("Arguments", this.CreateTokenizedString(arguments.ToString(), new[] { this.OutputPath }));
        }

        protected override Bam.Core.TokenizedString OutputPath
        {
            get
            {
                return this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/core.insts-$(Version).inc");
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

    class Generators : C.ProceduralHeaderFile
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.Add("PyScript", "$(packagedir)/utils/generate_registry_tables.py");
            this.Macros.Add("XmlRegistryFile", this.CreateTokenizedString("$(0)/spirv/spir-v.xml", new[] { spirvheaders.Macros["IncludeDir"] }));

            var arguments = new System.Text.StringBuilder();
            arguments.Append("$(PyScript) ");
            arguments.Append("--xml=$(XmlRegistryFile) ");
            arguments.Append("--generator-output=$(0) ");
            this.Macros.Add("Arguments", this.CreateTokenizedString(arguments.ToString(), new[] { this.OutputPath }));
        }

        protected override Bam.Core.TokenizedString OutputPath
        {
            get
            {
                return this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/generators.inc");
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

    class BuildVersion : C.ProceduralHeaderFile
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.Add("PyScript", "$(packagedir)/utils/update_build_version.py");

            var arguments = new System.Text.StringBuilder();
            arguments.Append("$(PyScript) ");
            arguments.Append("$(packagedir) ");
            arguments.Append("$(0) ");
            this.Macros.Add("Arguments", this.CreateTokenizedString(arguments.ToString(), new[] { this.OutputPath }));
        }

        protected override Bam.Core.TokenizedString OutputPath
        {
            get
            {
                return this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/build-version.inc");
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
            source.AddFiles("$(packagedir)/source/util/*.cpp");

            this.CompileAgainst<SPIRVHeaders.SPIRVHeaders>(source);

            var extensionEnumInc = Bam.Core.Graph.Instance.FindReferencedModule<ExtensionEnumInc>();
            source.DependsOn(extensionEnumInc);
            source.UsePublicPatches(extensionEnumInc);

            var debugInfoHeader = Bam.Core.Graph.Instance.FindReferencedModule<DebugInfo>();
            source.DependsOn(debugInfoHeader);
            source.UsePublicPatches(debugInfoHeader);

            /*
            var glslTablesInc = Bam.Core.Graph.Instance.FindReferencedModule<GLSLTables>();
            source.DependsOn(glslTablesInc);
            source.UsePublicPatches(glslTablesInc);

            var openclTablesInc = Bam.Core.Graph.Instance.FindReferencedModule<OpenCLTables>();
            source.DependsOn(openclTablesInc);
            source.UsePublicPatches(openclTablesInc);
            */

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

            /*
            var coreTables = Bam.Core.Graph.Instance.FindReferencedModule<CoreTables>();
            source.DependsOn(coreTables);
            source.UsePublicPatches(coreTables);

            var generators = Bam.Core.Graph.Instance.FindReferencedModule<Generators>();
            source.DependsOn(generators);
            source.UsePublicPatches(generators);

            var buildVersion = Bam.Core.Graph.Instance.FindReferencedModule<BuildVersion>();
            source.DependsOn(buildVersion);
            source.UsePublicPatches(buildVersion);
            */

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
