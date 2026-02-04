WINDOWS = os.host() == "windows"

project "ImGui"
	kind "StaticLib"
	language "C++"
    staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"imconfig.h",
		"imgui.h",
		"imgui.cpp",
		"imgui_draw.cpp",
		"imgui_internal.h",
		"imgui_tables.cpp",
		"imgui_widgets.cpp",
		"imstb_rectpack.h",
		"imstb_textedit.h",
		"imstb_truetype.h",
		"imgui_demo.cpp",
		"imgui-SFML.cpp",
		"imgui-SFML.h",
		"imconfig-SFML.h",
		"imgui-SFML_export.h"
	}
 
	links
	{
	"freetype",
	"winmm",
	"gdi32",
       --"flac",
	"vorbisenc",
	"vorbisfile",
	"vorbis",
	"ogg",
	"ws2_32",
    "legacy_stdio_definitions" --falls was schiefgeht einfach wieder reinmachen
	}
	includedirs
	{
	 "../glm",
	 "../SFML/SFML-3.0.0/include"
	}

    filter "action:vs*"
        buildoptions { "/Zc:__cplusplus" }

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++20"
        libdirs { "../SFML/SFML-3.0.0/lib" }
        links
        {
        		"opengl32",
        }
        defines {"SFML_STATIC"}


	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++20"
        links
        {
            "GL",
            "sfml-graphics",
			"sfml-window",
			"sfml-system",
			"sfml-audio",
			"sfml-network"
        }

   filter "configurations:Release"
      defines { "RELEASE" }
      if WINDOWS then
        links
        {	
          "FLAC",
          "freetype",
          "ogg",
          "vorbisenc",
		  "vorbisfile",
		  "vorbis",
          "sfml-graphics-s",
          "sfml-window-s",
          "sfml-system-s",
          "sfml-audio-s",
          "sfml-network-s",
        }
      end  
      runtime "Release"
      optimize "On"
      symbols "On"

   filter "configurations:Dist"
      defines { "DIST" }
      if WINDOWS then
        links
        {
          "FLAC",
          "freetype",
          "ogg",
          "vorbisenc",
		  "vorbisfile",
		  "vorbis",
          "sfml-graphics-s",
          "sfml-window-s",
          "sfml-system-s",
          "sfml-audio-s",
          "sfml-network-s"
        }
      end
      runtime "Release"
      optimize "On"
      symbols "Off"
