:: path to glslc.exe may differ for each computer
:: path to glslc.exe will differ for each Vulkan version
:: TODO: look into libshaderc (included by Vulkan) (to compile at runtime?)
C:/VulkanSDK/1.2.162.1/Bin32/glslc.exe shader.vert -o vert.spv
C:/VulkanSDK/1.2.162.1/Bin32/glslc.exe shader.frag -o frag.spv
pause
