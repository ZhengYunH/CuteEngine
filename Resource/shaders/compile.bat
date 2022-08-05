@echo off 

set ROOT_DIR=D:\cute\CuteEngine\CuteEngine

echo %ROOT_DIR%

%ROOT_DIR%\Libraries\VulkanSDK\1.2.198.0\Bin\glslc.exe shader.vert -O0 -o vert.spv
%ROOT_DIR%\Libraries\VulkanSDK\1.2.198.0\Bin\glslc.exe shader.frag -O0 -o frag.spv

%ROOT_DIR%\Libraries\VulkanSDK\1.2.198.0\Bin\glslc.exe ui.vert -O0 -o ui.vert.spv
%ROOT_DIR%\Libraries\VulkanSDK\1.2.198.0\Bin\glslc.exe ui.frag -O0 -o ui.frag.spv

%ROOT_DIR%\Libraries\VulkanSDK\1.2.198.0\Bin\glslc.exe terrain.vert -O0 -o terrain.vert.spv
%ROOT_DIR%\Libraries\VulkanSDK\1.2.198.0\Bin\glslc.exe terrain.frag -O0 -o terrain.frag.spv
pause