
%VULKAN_SDK%/Bin/glslc.exe base.glsl.vert -o base.vert.spv
%VULKAN_SDK%/Bin/glslc.exe base.glsl.frag -o base.frag.spv

%VULKAN_SDK%/Bin/glslc.exe fullscreen.glsl.vert -o fullscreen.vert.spv
%VULKAN_SDK%/Bin/glslc.exe fullscreen.glsl.frag -o fullscreen.frag.spv

%VULKAN_SDK%/Bin/glslc.exe lighting.glsl.comp -o lighting.comp.spv

%VULKAN_SDK%/Bin/glslc.exe TAA.glsl.comp -o TAA.comp.spv

%VULKAN_SDK%/Bin/glslc.exe FXAA.glsl.frag -o FXAA.frag.spv

%VULKAN_SDK%/Bin/glslc.exe sky.glsl.vert -o sky.vert.spv
%VULKAN_SDK%/Bin/glslc.exe sky.glsl.frag -o sky.frag.spv

%VULKAN_SDK%/Bin/glslc.exe debug.glsl.vert -o debug.vert.spv
%VULKAN_SDK%/Bin/glslc.exe debug.glsl.frag -o debug.frag.spv

%VULKAN_SDK%/Bin/glslc.exe shadow.glsl.vert -o shadow.vert.spv
%VULKAN_SDK%/Bin/glslc.exe variance.glsl.frag -o variance.frag.spv

%VULKAN_SDK%/Bin/glslc.exe Fog.glsl.comp -o Fog.comp.spv

%VULKAN_SDK%/Bin/glslc.exe FXAA.glsl.comp -o FXAA.comp.spv

%VULKAN_SDK%/Bin/glslc.exe ChromaticAberration.glsl.comp -o ChromaticAberration.comp.spv

%VULKAN_SDK%/Bin/glslc.exe PostProcess/FilmGrain.glsl.comp -o PostProcess/FilmGrain.comp.spv

%VULKAN_SDK%/Bin/glslc.exe genIrradiance.glsl.comp -o genIrradiance.comp.spv

%VULKAN_SDK%/Bin/glslc.exe PostProcess/ApplyAO.glsl.frag -o PostProcess/ApplyAO.frag.spv

%VULKAN_SDK%/Bin/glslc.exe depthCorrector.glsl.frag -o depthCorrector.frag.spv

%VULKAN_SDK%/Bin/glslc.exe shadowResolve.glsl.frag -o shadowResolve.frag.spv

pause
