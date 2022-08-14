
Current it does not have a name. I should probably come up with one at some point.

This Engine is a hobby project. I'm using it to learn but also as a showcase.
Some things could be redone. My vulkan wrapper to help with vulkan is a few projects old and some even remains from when I
was first learning vulkan, so it could do with a rewrite following the best practices. 

Progress as of August 6th: 
![Current Progress Of the engine on August 6th 2022](https://github.com/Progalt/HobbyEngine/blob/master/Images/3DEngineShowcase-August6.png)

and a bit earlier on August 4th: 
![Current Progress Of the engine on August 4th 2022](https://github.com/Progalt/HobbyEngine/blob/master/Images/3DEngineShowcase-August4.png)

### Performance
Currently performance is a concern. Running at about 300FPS with Approx. 500 draw calls. Performance could be improved by multi-threading the recording of command lists.
Currently the whole engine is single threaded which is very inefficient and barely uses the CPU or the GPU effectively. 

Though perfomance is not a massive concern as this engine is more for learning. It would be nice to be able to run at good framerates and maybe use it for hobby projects.

### Current Backends: 
- Vulkan

Hopefully DirectX 12 in future.

Currently all shaders are written in GLSL. I want to try and rewrite it in HLSL
as HLSL can be compiled to spir-v for vulkan. 

### Current Features: 
- Deferred Rendering
- Post Process Stack
     + FXAA
     + Fog
     + Chromatic Aberration
     + Film Grain
     + TAA - Still a work in progress to get it right
- Physically Based Lighting - Still Also work in Progress as well. 
    + Diffuse Irradiance. - Can be captured using the scene but its a bit buggy at the moment. 
- Light Casters
    + Directional Light
- Scene System
- Component System
- Cascade shadow maps - Resolved in its own pass and applied in screen space during lighting pass
- Custom Model format
- CPU Frustum Culling
- Light Probes - Only Diffuse Irradiance at the moment. 
- SSAO - Using FidelityFX CACAO

### WIP Features: 
- Screen Space Reflections
- Clustered Light culling - Cull into tiles on XY and into a linear Z buffer
- Scene loading and saving. 
- Resource Streaming
    I have an idea of how I want to do this for textures. When loading the scene load a smaller mip like 32x32 for example and then when the camera gets closer request the high resolution mips be loaded.
- Threading


### Planned Features: 
These are features I want to implement if I get time. 
Some of these features I would need to research. 
- GPU Triangle culling - I know how I would do this and while the engine in its current state does not support it. Some changes would allow it. 
- GPU Skinning - Stores two buffers. One of Normal pose from mesh import and another with the skinned animation. The second buffer is then used for all draw calls where the model is visible. This 
would require adding the support to my model format.
- Physics
- Audio
- Particle systems - CPU and GPU
- Raytracing - I would like to make a hybrid renderer, where shadows and reflections can be raytraced and the rest is rasterised. 