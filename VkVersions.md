Since this information is slightly outdated with 1.3 now out, check:
https://www.khronos.org/registry/vulkan/specs/
and then browse to
A) .../**X**-extensions/html/
B) .../**X**-khr-extensions/html/
C) .../**X**/html/
with **X** being the latest version, for example **1.3**.
and then find `Appendix D: Core Revisions (Informative)` near the bottom of the left navigation bar
These pages will also have the info regarding all older major revisions and are basically identical between the three options.

# Vulkan 1.0
Always go for this version unless your application **requires** something introduced in later versions.
### Extensions
- Multiview (VR, cubemaps, etc.)
- Multi-GPU
- Enhanced Windows Sys Integration
- Increased Shader Flexibility:
	+ 16-bit storage
	+ variable pointers
- Enhanced CrossProcess and Cross-API sharing

# Vulkan 1.1
### Introduced
- Subgroup Operations
### Integrated
All 1.0 Extensions have been integrated into the core of Vulkan.
### Extensions
- Reduced precision arithmetic types in shaders
- Bindless resources
- HLSL-compatible memory layouts
- Formal memory model
- Buffer references
- Timeline semaphores

# Vulkan 1.2
### Introduced
- SPIR-V 1.5
### Integrated
All 1.1 extensions have been integrated into the core of Vulkan
### Extensions
These may not have been released yet!
- Machine Learning
- Ray Tracing
- Video encode/decode
- Variable Rate Shading
- Mesh Shaders
