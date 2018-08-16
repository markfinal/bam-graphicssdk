namespace MetalTriangle
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
            // do nothing, as the file always exists
        }

        Bam.Core.TokenizedString Bam.Core.IInputPath.InputPath
        {
            get;
            set;
        }
    }
}
