namespace MetalUtilities
{
    public static class MetalExtensions
    {
        public static void
        registerMetalMappings(
            this Publisher.Collation collation)
        {
            collation.Mapping.Register(
                typeof(DefaultMetalShaderLibrary),
                DefaultMetalShaderLibrary.ShaderLibraryKey,
                collation.ResourceDir,
                true
            );
        }
    }
}
