# CuteEngine



## Enviroment

OS: Window10 - x64

IDE: Visual Studio 2022

C++:  (/std:c++20)

### Setup Visual Studio Project

1. Download & Install cmake (require version 3.22): [Download | CMake](https://cmake.org/download/)

2. run cmake . 

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