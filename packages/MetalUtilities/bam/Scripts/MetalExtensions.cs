namespace MetalUtilities
{
    public static class MetalExtensions
    {
        public static void
        registerMetalMappings(
            this Publisher.Collation collation)
        {
            collation.Mapping.Register(
                typeof(MetalShaderLibrary),
                MetalShaderLibrary.ShaderLibraryKey,
                collation.ResourceDir,
                true
            );
        }
    }
}
