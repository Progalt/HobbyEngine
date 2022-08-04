
This Engine is a hobby project. I'm using it to learn but also as a showcase.


I've tried to comment the code as well, although that may of failed in some areas. 
The main render function is nicely commented though and it explains some of my choices with it. 


![Current Progress Of the engine on August 4th 2022](https://github.com/Progalt/HobbyEngine/blob/master/Images/3DEngineShowcase-August4.png)

### Current Features: 
- Deferred Rendering
- Post Process Stack
     + FXAA
     + Fog
     + Chromatic Aberration
- Physically Based Lighting - Still Also work in Progress as well. 
- Light Casters
    + Directional Light
- Scene System
- Component System
- Cascade shadow maps
- Custom Model format

### WIP Features: 
- TAA
- SSAO - I might use AMD OpenGPU CACAO for this.
- CPU Frustum Culling
- Clustered Light culling - Cull into tiles on XY and into a linear Z buffer


### Planned Features: 
These are features I want to implement if I get time. 
- GPU Triangle culling - I know how I would do this and while the engine in its current state does not support it. Some changes would allow it. 
- GPU Skinning - Stores two buffers. One of Normal pose from mesh import and another with the skinned animation. The second buffer is then used for all draw calls where the model is visible. This 
would require adding the support to my model format.
- Physics
- Audio