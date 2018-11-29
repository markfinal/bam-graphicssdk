#region License
// Copyright (c) 2010-2018, Mark Final
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

    class GLSLangValidatorTool :
        Bam.Core.Module,
        Bam.Core.ICommandLineTool
    {
        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            if (this.BuildEnvironment.Platform.Includes(Bam.Core.EPlatform.OSX))
            {
                var glslangValidator = Bam.Core.Graph.Instance.FindReferencedModule<glslang.GLSLangValidator>();
                this.DependsOn(glslangValidator);

                this.Macros.Add("Executable", glslangValidator.GeneratedPaths[C.Cxx.ConsoleApplication.ExecutableKey]);
            }
            else
            {
                var latest_version_path = GetInstallDir.Find(this.BuildEnvironment.Platform);
                this.Macros["packagedir"].Set(latest_version_path, null);

                this.Macros.Add("Executable", this.CreateTokenizedString("$(packagedir)/Bin/glslangValidator.exe"));
            }
        }

        protected override void
        EvaluateInternal()
        {
            // is up-to-date
            this.ReasonToExecute = null;
        }

        protected override void
        ExecuteInternal(
            Bam.Core.ExecutionContext context)
        {
            // do nothing - either prebuilt, or will be built as a dependency
        }

        System.Collections.Generic.Dictionary<string, Bam.Core.TokenizedStringArray> Bam.Core.ICommandLineTool.EnvironmentVariables => null;
        Bam.Core.StringArray Bam.Core.ICommandLineTool.InheritedEnvironmentVariables => null;
        Bam.Core.TokenizedString Bam.Core.ICommandLineTool.Executable => this.Macros["Executable"];
        Bam.Core.TokenizedStringArray Bam.Core.ICommandLineTool.InitialArguments => null;
        Bam.Core.TokenizedStringArray Bam.Core.ICommandLineTool.TerminatingArguments => null;
        string Bam.Core.ICommandLineTool.UseResponseFileOption => null;
        Bam.Core.Array<int> Bam.Core.ICommandLineTool.SuccessfulExitCodes => new Bam.Core.Array<int> { 0 };

        Bam.Core.Settings
        Bam.Core.ITool.CreateDefaultSettings<T>(
            T module)
        {
            return new GLSLangValidatorSettings(module);
        }
    }

    class SPIRVModule :
        Bam.Core.Module
    {
        public const string SPIRVKey = "SPIRV";

        public GLSLSource Source { get; set; }

        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);
            this.Tool = Bam.Core.Module.Create<GLSLangValidatorTool>();
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
            NativeBuilder.Support.RunCommandLineTool(this, context);
        }

        public override System.Collections.Generic.IEnumerable<System.Collections.Generic.KeyValuePair<string, Bam.Core.Module>> InputModules
        {
            get
            {
                yield return new System.Collections.Generic.KeyValuePair<string, Bam.Core.Module>(GLSLSource.GLSLKey, this.Source);
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
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);
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