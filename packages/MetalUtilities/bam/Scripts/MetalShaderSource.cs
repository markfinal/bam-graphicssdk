namespace MetalUtilities
{
    class MetalShaderSource :
        Bam.Core.Module,
        Bam.Core.IInputPath
    {
        public const string ShaderSourceKey = "Metal shader source";

        protected override void
        Init(
            Bam.Core.Module parent)
        {
            base.Init(parent);

            this.RegisterGeneratedFile(
                ShaderSourceKey,
                (this as Bam.Core.IInputPath).InputPath
            );
        }

        protected override void
        EvaluateInternal()
        {
            // never execute, as always a file on disk
            this.ReasonToExecute = null;
        }

        protected override void ExecuteInternal(
            Bam.Core.ExecutionContext context)
        {
            switch (Bam.Core.Graph.Instance.Mode)
            {
#if D_PACKAGE_XCODEBUILDER
                case "Xcode":
                    {
                        var encapsulating = this.GetEncapsulatingReferencedModule();
                        var workspace = Bam.Core.Graph.Instance.MetaData as XcodeBuilder.WorkspaceMeta;
                        var target = workspace.EnsureTargetExists(encapsulating);
                        target.EnsureFileOfTypeExists(
                            this.GeneratedPaths[MetalShaderSource.ShaderSourceKey],
                            XcodeBuilder.FileReference.EFileType.MetalShaderSource
                        );
                    }
                    break;
#endif
            }
        }

        Bam.Core.TokenizedString Bam.Core.IInputPath.InputPath
        {
            get;
            set;
        }
    }
}
