
%VULKAN_SDK%/Bin/glslc.exe base.glsl.vert -o base.vert.spv
%VULKAN_SDK%/Bin/glslc.exe base.glsl.frag -o base.frag.spv

%VULKAN_SDK%/Bin/glslc.exe fullscreen.glsl.vert -o fullscreen.vert.spv
%VULKAN_SDK%/Bin/glslc.exe fullscreen.glsl.frag -o fullscreen.frag.spv

%VULKAN_SDK%/Bin/glslc.exe lighting.glsl.comp -o lighting.comp.spv

%VULKAN_SDK%/Bin/glslc.exe TAA.glsl.comp -o TAA.comp.spv

pause
