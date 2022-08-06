
This Engine is a hobby project. I'm using it to learn but also as a showcase.
Some things could be redone. My vulkan wrapper to help with vulkan is a few projects old and some even remains from when I
was first learning vulkan, so it could do with a rewrite following the best practices. 

Progress as of August 6th: 
![Current Progress Of the engine on August 6th 2022](https://github.com/Progalt/HobbyEngine/blob/master/Images/3DEngineShowcase-August6.png)

and a bit earlier on August 4th: 
![Current Progress Of the engine on August 4th 2022](https://github.com/Progalt/HobbyEngine/blob/master/Images/3DEngineShowcase-August4.png)

### Current Features: 
- Deferred Rendering
- Post Process Stack
     + FXAA
     + Fog
     + Chromatic Aberration
     + Film Grain
- Physically Based Lighting - Still Also work in Progress as well. 
    + Diffuse Irradiance.
- Light Casters
    + Directional Light
- Scene System
- Component System
- Cascade shadow maps
- Custom Model format
- CPU Frustum Culling
- Light Probes - Only Diffuse Irradiance at the moment. 

### WIP Features: 
- TAA
- SSAO - I might use AMD OpenGPU CACAO for this.
- Screen Space Reflections
- Clustered Light culling - Cull into tiles on XY and into a linear Z buffer
- Scene loading and saving. 
- Resource Streaming
    I have an idea of how I want to do this for textures. When loading the scene load a smaller mip like 32x32 for example and then when the camera gets closer request the high resolution mips be loaded.


### Planned Features: 
These are features I want to implement if I get time. 
Some of these features I would need to research. 
- GPU Triangle culling - I know how I would do this and while the engine in its current state does not support it. Some changes would allow it. 
- GPU Skinning - Stores two buffers. One of Normal pose from mesh import and another with the skinned animation. The second buffer is then used for all draw calls where the model is visible. This 
would require adding the support to my model format.
- Physics
- Audio
- Particle systems - CPU and GPU