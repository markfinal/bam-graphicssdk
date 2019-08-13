#region License
// Copyright (c) 2010-2019, Mark Final
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of BuildAMation nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#endregion // License
using System.Linq;
namespace SPIRVTools
{
    class PythonSourceGenerator :
        C.ExternalSourceGenerator
    {
        protected override void
        Init()
        {
            base.Init();

            this.Executable = Bam.Core.TokenizedString.CreateVerbatim(Bam.Core.OSUtilities.GetInstallLocation("python").FirstOrDefault());

            this.PublicPatch((settings, appliedTo) =>
            {
                if (settings is C.ICommonPreprocessorSettings preprocessor)
                {
                    preprocessor.IncludePaths.AddUnique(this.OutputDirectory);
                }
            });
        }
    }

    class ExtensionEnumInc :
        PythonSourceGenerator
    {
        protected override void
        Init()
        {
            base.Init();

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.AddVerbatim("Version", "unified1");

            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/generate_grammar_tables.py"));
            this.AddInputFile("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/spirv.core.grammar.json", new[] { spirvheaders.Macros["IncludeDir"] }));
            this.AddInputFile("DebugGrammarFile", this.CreateTokenizedString("$(packagedir)/source/extinst.debuginfo.grammar.json"));

            this.SetOutputDirectory("$(packagebuilddir)/$(moduleoutputdir)");
            this.AddExpectedOutputFile("GrammarExtensionEnumIncFile", this.CreateTokenizedString("$(0)/extension_enum.inc", new []{this.OutputDirectory}));
            this.AddExpectedOutputFile("GrammarEnumStringMappingIncFile", this.CreateTokenizedString("$(0)/enum_string_mapping.inc", new[] { this.OutputDirectory }));

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("--spirv-core-grammar=$(GrammarJsonFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-debuginfo-grammar=$(DebugGrammarFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--extension-enum-output=$(0)", this.GeneratedPaths["GrammarExtensionEnumIncFile"]));
            this.Arguments.Add(this.CreateTokenizedString("--enum-string-mapping-output=$(0)", this.GeneratedPaths["GrammarEnumStringMappingIncFile"]));
        }
    }

    class DebugInfo :
        PythonSourceGenerator
    {
        protected override void
        Init()
        {
            base.Init();

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/generate_language_headers.py"));
            this.AddInputFile("DebugGrammarFile", this.CreateTokenizedString("$(packagedir)/source/extinst.debuginfo.grammar.json"));

            this.SetOutputDirectory("$(packagebuilddir)/$(moduleoutputdir)");
            this.AddExpectedOutputFile("DebugInfo", this.CreateTokenizedString("$(0)/DebugInfo.h", new []{this.OutputDirectory}));

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-name=DebugInfo"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-grammar=$(DebugGrammarFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-output-base=$(0)/DebugInfo", new []{this.OutputDirectory}));
        }
    }

    class GLSLTables :
        PythonSourceGenerator
    {
        protected override void
        Init()
        {
            base.Init();

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.AddVerbatim("Version", "unified1");

            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/generate_grammar_tables.py"));
            this.AddInputFile("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/extinst.glsl.std.450.grammar.json", new[] { spirvheaders.Macros["IncludeDir"] }));

            this.SetOutputDirectory("$(packagebuilddir)/$(moduleoutputdir)");
            this.AddExpectedOutputFile("GLSLHeader", this.CreateTokenizedString("$(0)/glsl.std.450.insts.inc", new []{this.OutputDirectory}));

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-glsl-grammar=$(GrammarJsonFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--glsl-insts-output=$(0)", this.GeneratedPaths["GLSLHeader"]));
        }
    }

    class OpenCLTables :
        PythonSourceGenerator
    {
        protected override void
        Init()
        {
            base.Init();

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.AddVerbatim("Version", "unified1");
            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/generate_grammar_tables.py"));
            this.AddInputFile("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/extinst.opencl.std.100.grammar.json", new[] { spirvheaders.Macros["IncludeDir"] }));

            this.SetOutputDirectory("$(packagebuilddir)/$(moduleoutputdir)");
            this.AddExpectedOutputFile("OpenCLHeader", this.CreateTokenizedString("$(0)/opencl.std.insts.inc", new []{this.OutputDirectory}));

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-opencl-grammar=$(GrammarJsonFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--opencl-insts-output=$(0)", this.GeneratedPaths["OpenCLHeader"]));
        }
    }

    class GenerateVendorTables :
        PythonSourceGenerator
    {
        protected override void
        Init()
        {
            base.Init();

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/generate_grammar_tables.py"));
            this.AddInputFile("DebugGrammarFile", this.CreateTokenizedString("$(packagedir)/source/extinst.$(TableName).grammar.json"));

            this.SetOutputDirectory("$(packagebuilddir)/$(moduleoutputdir)");
            this.AddExpectedOutputFile("VendorTable", this.CreateTokenizedString("$(0)/$(TableName).insts.inc", new [] {this.OutputDirectory}));

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-vendor-grammar=$(DebugGrammarFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--vendor-insts-output=$(0)", this.GeneratedPaths["VendorTable"]));
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
        Init()
        {
            base.Init();

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.Macros.AddVerbatim("Version", "unified1");
            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/generate_grammar_tables.py"));
            this.AddInputFile("GrammarJsonFile", this.CreateTokenizedString("$(0)/spirv/$(Version)/spirv.core.grammar.json", new[] { spirvheaders.Macros["IncludeDir"] }));
            this.AddInputFile("DebugGrammarFile", this.CreateTokenizedString("$(packagedir)/source/extinst.debuginfo.grammar.json"));

            this.SetOutputDirectory("$(packagebuilddir)/$(moduleoutputdir)");
            this.AddExpectedOutputFile("CoreInstIncFile", this.CreateTokenizedString("$(0)/core.insts-$(Version).inc", new []{this.OutputDirectory}));
            this.AddExpectedOutputFile("GrammarKindsIncFile", this.CreateTokenizedString("$(0)/operand.kinds-$(Version).inc", new []{this.OutputDirectory}));

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("--spirv-core-grammar=$(GrammarJsonFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--extinst-debuginfo-grammar=$(DebugGrammarFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--core-insts-output=$(0)", this.GeneratedPaths["CoreInstIncFile"]));
            this.Arguments.Add(this.CreateTokenizedString("--operand-kinds-output=$(0)", this.GeneratedPaths["GrammarKindsIncFile"]));
        }
    }

    class Generators :
        PythonSourceGenerator
    {
        protected override void
        Init()
        {
            base.Init();

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/generate_registry_tables.py"));
            this.AddInputFile("XmlRegistryFile", this.CreateTokenizedString("$(0)/spirv/spir-v.xml", new[] { spirvheaders.Macros["IncludeDir"] }));

            this.SetOutputDirectory("$(packagebuilddir)/$(moduleoutputdir)");
            this.AddExpectedOutputFile("GeneratorsInc", this.CreateTokenizedString("$(0)/generators.inc", new []{this.OutputDirectory}));

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("--xml=$(XmlRegistryFile)"));
            this.Arguments.Add(this.CreateTokenizedString("--generator-output=$(0)", this.GeneratedPaths["GeneratorsInc"]));
        }
    }

    class BuildVersion :
        PythonSourceGenerator
    {
        protected override void
        Init()
        {
            base.Init();

            var spirvheaders = Bam.Core.Graph.Instance.FindReferencedModule<SPIRVHeaders.SPIRVHeaders>();
            this.DependsOn(spirvheaders);

            this.AddInputFile("PyScript", this.CreateTokenizedString("$(packagedir)/utils/update_build_version.py"));

            this.SetOutputDirectory("$(packagebuilddir)/$(moduleoutputdir)");
            this.AddExpectedOutputFile("BuildVersionInc", this.CreateTokenizedString("$(0)/build-version.inc", new []{this.OutputDirectory}));

            this.Arguments.Add(this.CreateTokenizedString("$(PyScript)"));
            this.Arguments.Add(this.CreateTokenizedString("$(packagedir)"));
            this.Arguments.Add(this.GeneratedPaths["BuildVersionInc"]);
        }
    }

    class SPIRVTools : C.StaticLibrary
    {
        protected override void
        Init()
        {
            base.Init();

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
                var preprocessor = settings as C.ICommonPreprocessorSettings;
                preprocessor.IncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/source"));

                var cxx_compiler = settings as C.ICxxOnlyCompilerSettings;
                cxx_compiler.LanguageStandard = C.Cxx.ELanguageStandard.Cxx11;

                if (settings is ClangCommon.ICommonCompilerSettings clang_compiler)
                {
                    clang_compiler.AllWarnings = true;
                    clang_compiler.ExtraWarnings = true;
                    clang_compiler.Pedantic = true;
                }
            });

            source["validate_builtins.cpp"].ForEach(item => item.PrivatePatch(settings =>
            {
                var compiler = settings as C.ICommonCompilerSettings;
                compiler.DisableWarnings.AddUnique("switch");
            }));
            source["validate_image.cpp"].ForEach(item => item.PrivatePatch(settings =>
            {
                var compiler = settings as C.ICommonCompilerSettings;
                compiler.DisableWarnings.AddUnique("switch");
            }));

            this.PublicPatch((settings, appliedTo) =>
            {
                if (settings is C.ICommonPreprocessorSettings preprocessor)
                {
                    preprocessor.SystemIncludePaths.AddUnique(this.CreateTokenizedString("$(packagedir)/include"));
                }
            });
        }
    }
}
