# CuteEngine

## Enviroment

OS: Window10 - x64

IDE: Visual Studio 2022

C++:  (/std:c++20)

### Setup

1. Download & Install cmake (require version 3.22): [Download | CMake](https://cmake.org/download/)
2. Download & install [VulkanSDK](https://vulkan.lunarg.com/) (the same version is better :) 1.2.198.0) 
3. run cmake . 
4. after 2. you will get visual studio project, open it
5. set "CuteEngine" project as the start up Project
6. Compile and Run

### Libraries Version

+ boost 1.79.0
+ VulkanSDK 1.2.198.0
+ tinyobjloader 1.06
+ glfw 3.3.5

### TODO List

+ FileSystem
  + ~~Support FBX format file~~
  + ~~Create And Load Scene File~~
  + ~~Debug Log~~
  + Resource Module
+ Basement Architecture
  + ~~InputSystem~~
  + ~~Event Binding~~
  + ~~Shader Factory~~
  + Vertex Factory
  + Culling
+ Rendering
  + ~~Support MultiModel Render(IPrimitive Component)~~
  + ~~ImGUI RenderPass~~
  + Shader Reflection: Generate Desciptor and Input Attribute Automatically 
  + Deferred Shading ( Multi-Render Target and Subpass)
  + Shadow
  + Compute Shader
  + Fancy Rendering Effect
    + TSAA
    + Hi-Z
+ Multi-Threading
+ Terrain
+ Physx
+ Animation
+ Navigation
+ Audio