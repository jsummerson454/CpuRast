# CpuRast
Very simple software rasterizer project created mainly as a recap of the rasterization pipeline and a deeper investigation of the rasterization algorithm in practice.

## Features / TODOs:
- [x] User-programmable vertex and fragment shaders
- [x] (Semi-)hardware faithful rasterization implementation in C++
- [x] Perspective correct interpolation
- [x] Fixed-precision z-buffering
- [x] Back-face culling
- [x] Subpixel precision rendering (currently only supports 28.4 fixed point precision)
- [ ] Improve rasterization loop using triangle setup and barycentric incrementors
- [ ] Top-left rule for consistent triangle edge renderings
- [x] Simple texture sampling functionality
- [ ] Render to window, rather than image
- [ ] Proper clipping after vertex shader, rather than pseudo-infite guardband clipping currently used
- [ ] Profiling and optimisation

## Examples
### Basic example
Basic scene that tests core rasterization features against OpenGL, most prominently testing correct vertex attribute interpolation and depth testing.
Output is neigh identical to OpenGL and only differs only in edge pixels due to differences in sub-pixel precision.

![image](https://user-images.githubusercontent.com/50595887/223506134-6efa64c6-069e-4268-bfc2-ec6ef4dd77db.png)

### Checkerboard example
Example used to showcase importance of perspective correct interpolation (as well as testing texture sampling and index buffers).

![image](https://user-images.githubusercontent.com/50595887/223507316-273faeec-b800-404e-84af-53eaffcfa4cd.png)

### Model example
Full scale example usage of renderer to render a model loaded using ASSIMP, with Blinn-Phong shading alongside diffuse, normal and specular mapping.

![image](https://user-images.githubusercontent.com/50595887/223507637-27809e54-a266-4802-8321-6b3893dcbff4.png)

## Disclaimer
Code is free to use if desired, but wholly not recommended. At its core, this renderer implements a rasterization method intended for massively parallel execution in a completely serial manner.
A scan-line approach would be far faster in this scenario, but as the intention was to learn more in depth on how rasterization is performed on hardware, the parallel algorithm was implemented.
This also allows for future optimisations to utilize SIMD instruction execution and/or multi-threading.

## Useful resources
[A trip though the graphics pipeline](https://alaingalvan.gitbook.io/a-trip-through-the-graphics-pipeline/)

[Rgy series on software occlusion culling](https://fgiesen.wordpress.com/2013/02/17/optimizing-sw-occlusion-culling-index/)

[Scratchapixel's 'Rasterization: a Practical Implementation'](https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-practical-implementation.html)

[Tiny Renderer](https://github.com/ssloy/tinyrenderer/wiki)
