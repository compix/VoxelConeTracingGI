# Real-Time Indirect Illumination with Voxel Cone Tracing (VCT)
## Overview
Check out *Implementation-Overview.pdf*

![Overview](https://github.com/compix/VoxelConeTracingGI/blob/master/images/Overview.png)

## Features
* One bounce indirect illumination (diffuse + specular) with VCT
* Emission with VCT
* 3D Clipmap to support a huge area around the camera
* Ambient Occlusion with VCT

## Issues
* No converage based voxelization (anti-aliasing voxelization)
  * Leads to discrete indirect illumination changes when objects move
* Hardly any performance optimizations
* Noticeable indirect illumination changes when the camera moves (because a clipmap is used)
* Perfect reflections aren't practical with VCT because the voxelized scene is only a rough approximation

## Screenshots
![GI](https://github.com/compix/VoxelConeTracingGI/blob/master/images/GI.png)
![Specularity](https://github.com/compix/VoxelConeTracingGI/blob/master/images/Specularity.png)
![Emission](https://github.com/compix/VoxelConeTracingGI/blob/master/images/Emission.png)
![Ambient](https://github.com/compix/VoxelConeTracingGI/blob/master/images/AO.png)

## Relevant Sources
* Crassin, Cyril, Fabrice Neyret, Miguel Sainz, Simon Green and Elmar Eisemann: Interactive Indirect Illumination Using Voxel Cone Tracing. In: Computer Graphics Forum, volume 30, pages 1921-1930. Wiley Online Library, 2011.
* Engel, Klaus, Markus Hadwiger, Joe Kniss, Christof Rezk-Salama and Daniel Weiskopf: Real-Time Volume Graphics. A K Peters, 2006.
* McLaren, James: The Technology Of The Tomorrow Children. Q-Games Ltd., Game Developers Conference, 2015.
* Panteleev, Alexey: Practical Real-Time Voxel-Based Global Illumination For Current GPUs. NVIDIA, GPU Technology Conference, 2014.
* Tanner, Christopher C, Christopher J Migdal and Michael T Jones: The Clipmap: A Virtual Mipmap. In: Proceedings of the 25th annual conference on Computer graphics and interactive techniques, pages 151-158. ACM, 1998.
* [McGuire2011] McGuire, Computer Graphics Archive, Aug 2011 (http://graphics.cs.williams.edu/data)
