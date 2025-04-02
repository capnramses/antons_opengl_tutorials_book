@echo off
REM Build File for MSVC (Visual Studio).

REM Uncomment whichever one that you have installed:
REM See https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170#developer_command_file_locations
REM set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x64
REM set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x64
REM set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
REM set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio 13.0\VC\vcvarsall.bat" x64
REM set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
REM set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
REM set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
set VCVARS="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

set DIR_LIST=^
00_hello_triangle ^
00_hello_triangle_gl2.1 ^
01_extended_init ^
02_shaders ^
03_vertex_buffer_objects ^
04_mats_and_vecs ^
05_virtual_camera ^
06_vcam_with_quaternion ^
07_ray_picking ^
08_phong ^
09_texture_mapping ^
10_screen_capture ^
11_video_capture ^
12_debugging_shaders ^
13_mesh_import ^
14_multi_tex ^
15_phongtextures ^
16_frag_reject ^
17_alpha_blending ^
18_spotlights ^
19_fog ^
20_normal_mapping ^
21_cube_mapping ^
22_geom_shaders ^
23_tessellation_shaders ^
24_gui_panels ^
25_sprite_sheets ^
26_bitmap_fonts ^
27_font_atlas ^
28_uniform_buffer_object ^
29_particle_systems ^
30_skinning_part_one ^
31_skinning_part_two ^
32_skinning_part_three ^
33_extension_check ^
34_framebuffer_switch ^
35_image_kernel ^
36_colour_picking ^
37_deferred_shading ^
38_texture_shadows ^
39_texture_mapping_srgb ^
40_compute_shader ^
41_shader_hot_reload

FOR %%A IN (00_hello_triangle) DO (
  echo ~~~ %%A ~~~
  cd %%A
  call build.bat %VCVARS%
  cd ..
)
