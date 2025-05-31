workspace "WuWaShaderSandbox"
   architecture "x64"
   configurations { "Debug", "Release" }
   startproject "ShaderApp"

project "ShaderApp"
   location "ShaderApp"
   kind "WindowedApp"
   language "C++"
   cppdialect "C++20"
   staticruntime "on"

   targetdir ("bin/%{cfg.buildcfg}")
   objdir ("bin-int/%{cfg.buildcfg}")

   files {
       "src/**.h",
       "src/**.cpp",
       "src/shaders/**.hlsl"
   }

   includedirs {
       "include",
       "$(WindowsSDK_IncludePath)"
   }

   links {
       "d3d12.lib",
       "dxgi.lib",
       "dxguid.lib"
   }

   filter "system:windows"
       systemversion "latest"

   filter "configurations:Debug"
       symbols "On"

   filter "configurations:Release"
       optimize "On"
