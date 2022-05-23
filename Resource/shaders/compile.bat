@echo off 

set ROOT_DIR=D:\cute\CuteEngine\CuteEngine

echo %ROOT_DIR%

%ROOT_DIR%\Libraries\VulkanSDK\1.2.198.0\Bin\glslc.exe shader.vert -O0 -o vert.spv
%ROOT_DIR%\Libraries\VulkanSDK\1.2.198.0\Bin\glslc.exe shader.frag -O0 -o frag.spv

pause