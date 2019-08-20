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
using Bam.Core;
namespace VulkanSDK
{
    static class DefaultGLSLangValidatorSettings
    {
        public static void
        Defaults(
            this IGLSLangValidatorSettings settings,
            Bam.Core.Module module)
        {
            settings.Binary = true;
        }
    }

    [Bam.Core.SettingsExtensions(typeof(DefaultGLSLangValidatorSettings))]
    interface IGLSLangValidatorSettings :
        Bam.Core.ISettingsBase
    {
        bool Binary { get; set; }
    }

    [CommandLineProcessor.OutputPath(SPIRVModule.SPIRVKey, "-o ")]
    [CommandLineProcessor.InputPaths(GLSLSource.GLSLKey, "")]
    class GLSLangValidatorSettings :
        Bam.Core.Settings,
        IGLSLangValidatorSettings
    {
        public GLSLangValidatorSettings(
            Bam.Core.Module module) => this.InitializeAllInterfaces(module, false, true);

        [CommandLineProcessor.Bool("-V", "")]
        bool IGLSLangValidatorSettings.Binary { get; set; }

        public override void
        AssignFileLayout()
        {
            this.FileLayout = ELayout.Cmds_Outputs_Inputs;
        }
    }

    [C.Prebuilt]
    class LunarGGLSLangValidatorTool :
        Bam.Core.PreBuiltTool
    {
        protected override void
        Init()
        {
            var latest_version_path = GetInstallDir.Find(this.BuildEnvironment.Platform);
            this.Macros[Bam.Core.ModuleMacroNames.PackageDirectory].Set(latest_version_path, null);

            this.Macros.Add("Executable", this.CreateTokenizedString("$(packagedir)/Bin/glslangValidator.exe"));

            base.Init();
        }

        /// <summary>
        /// \copydoc Bam.Core.ITool.SettingsType
        /// </summary>
        public override System.Type SettingsType => typeof(GLSLangValidatorSettings);

        public override Bam.Core.TokenizedString Executable => this.Macros["Executable"];
    }

    class SPIRVModule :
        Bam.Core.Module
    {
        public const string SPIRVKey = "SPIRV";

        public GLSLSource Source { get; set; }

        protected override void
        Init()
        {
            base.Init();
            if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.OSX))
            {
                this.Tool = Bam.Core.Graph.Instance.FindReferencedModule<glslang.GLSLangValidator>();
            }
            else
            {
                this.Tool = Bam.Core.Graph.Instance.FindReferencedModule<LunarGGLSLangValidatorTool>();
            }
            this.RegisterGeneratedFile(
                SPIRVKey,
                this.CreateTokenizedString("$(packagebuilddir)/@basename($(0))_@extension($(0)).spv", new[] { this.Source.InputPath })
            );
        }

        protected override void
        EvaluateInternal()
        {
            this.ReasonToExecute = null;

            var outputPath = this.GeneratedPaths[SPIRVKey].ToString();
            if (!System.IO.File.Exists(outputPath))
            {
                this.ReasonToExecute = Bam.Core.ExecuteReasoning.FileDoesNotExist(this.GeneratedPaths[SPIRVKey]);
                return;
            }
            var outputPathWriteTime = System.IO.File.GetLastWriteTime(outputPath);

            // is the source file newer than the object file?
            var sourcePath = this.Source.InputPath.ToString();
            var sourceWriteTime = System.IO.File.GetLastWriteTime(sourcePath);
            if (sourceWriteTime > outputPathWriteTime)
            {
                this.ReasonToExecute = Bam.Core.ExecuteReasoning.InputFileNewer(
                    this.GeneratedPaths[SPIRVKey],
                    this.Source.InputPath
                );
            }
        }

        protected override void
        ExecuteInternal(
            Bam.Core.ExecutionContext context)
        {
            switch (Bam.Core.Graph.Instance.Mode)
            {
#if D_PACKAGE_MAKEFILEBUILDER
                case "MakeFile":
                    MakeFileBuilder.Support.Add(this);
                    break;
#endif

#if D_PACKAGE_NATIVEBUILDER
                case "Native":
                    NativeBuilder.Support.RunCommandLineTool(this, context);
                    break;
#endif

#if D_PACKAGE_VSSOLUTIONBUILDER
                case "VSSolution":
                    VSSolutionBuilder.Support.AddCustomBuildStepForCommandLineTool(
                        this,
                        this.GeneratedPaths[SPIRVKey],
                        "Compiling",
                        true
                    );
                    break;
#endif

#if D_PACKAGE_XCODEBUILDER
                case "Xcode":
                    {
                        XcodeBuilder.Support.AddPreBuildStepForCommandLineTool(
                            this,
                            out XcodeBuilder.Target target,
                            out XcodeBuilder.Configuration configuration,
                            XcodeBuilder.FileReference.EFileType.GLSLShaderSource,
                            true,
                            false,
                            outputPaths: new Bam.Core.TokenizedStringArray(this.GeneratedPaths[SPIRVKey])
                        );
                    }
                    break;
#endif

                default:
                    throw new System.NotImplementedException();
            }
        }

        public override System.Collections.Generic.IEnumerable<(Bam.Core.Module module, string pathKey)> InputModulePaths
        {
            get
            {
                yield return (this.Source, GLSLSource.GLSLKey);
            }
        }
    }

    class GLSLSource :
        Bam.Core.Module,
        Bam.Core.IInputPath
    {
        public const string GLSLKey = "GLSL";

        public Bam.Core.TokenizedString InputPath { get; set; }

        protected override void
        Init()
        {
            base.Init();
            // TODO: this seems a little backward
            this.RegisterGeneratedFile(
                GLSLKey,
                this.InputPath
            );
        }

        protected override void
        EvaluateInternal()
        {
            // up-to-date
            this.ReasonToExecute = null;
        }

        protected override void
        ExecuteInternal(
            Bam.Core.ExecutionContext context)
        {
            // do nothing - file must exist on disk
        }
    }
}
