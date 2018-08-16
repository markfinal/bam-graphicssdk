namespace MetalExtensions
{
    public static class MetalExtensions
    {
        public static void
        registerMetalMappings(
            this Publisher.Collation collation)
        {
            collation.Mapping.Register(
                typeof(MetalTriangle.MetalShaderLibrary),
                MetalTriangle.MetalShaderLibrary.ShaderLibraryKey,
                collation.ResourceDir,
                true
            );
        }
    }
}
