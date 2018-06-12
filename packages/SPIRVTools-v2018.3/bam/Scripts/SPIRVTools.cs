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

            this.Macros.Add("PyScript", "$(packagedir)/utils/generate_grammar_tables.py");
            this.Macros.Add("Version", "unified1");
            this.Macros.Add("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/spirv.core.grammar.json", new []{spirvheaders.Macros["IncludeDir"]}));
            this.Macros.Add("DebugGrammarFile", "$(packagedir)/source/extinst.debuginfo.grammar.json");
            this.Macros.Add("GrammarEnumStringMappingIncFile", "$(packagebuilddir)/$(moduleoutputdir)/enum_string_mapping.inc");

            var arguments = new System.Text.StringBuilder();
            arguments.Append("$(PyScript) ");
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

            this.Macros.Add("PyScript", "$(packagedir)/utils/generate_language_headers.py");
            this.Macros.Add("DebugGrammarFile", "$(packagedir)/source/extinst.debuginfo.grammar.json");

            var arguments = new System.Text.StringBuilder();
            arguments.Append("$(PyScript) ");
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

    class GenerateVendorTables : C.ProceduralHeaderFile
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.Add("PyScript", "$(packagedir)/utils/generate_grammar_tables.py");
            this.Macros.Add("DebugGrammarFile", "$(packagedir)/source/extinst.$(TableName).grammar.json");

            var arguments = new System.Text.StringBuilder();
            arguments.Append("$(PyScript) ");
            arguments.Append("--extinst-vendor-grammar=$(packagedir)/source/extinst.$(TableName).grammar.json ");
            arguments.Append("--vendor-insts-output=$(0) ");
            this.Macros.Add("Arguments", this.CreateTokenizedString(arguments.ToString(), new[] { this.OutputPath }));
        }

        public string TableName
        {
            set
            {
                this.Macros.AddVerbatim("TableName", value);
            }
        }

        protected override Bam.Core.TokenizedString OutputPath
        {
            get
            {
                return this.CreateTokenizedString("$(packagebuilddir)/$(moduleoutputdir)/$(TableName).insts.inc");
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
