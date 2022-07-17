#
#
#

cmake_minimum_required(VERSION 3.6)
project (antons_opengl_tutorials_book)

#OpenGL
find_package(OpenGL REQUIRED)

#GLFW
if (UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW REQUIRED glfw3)
else()
    #find_package(GLFW)
    find_path(GLFW_INCLUDE_DIRS NAMES "GLFW/glfw3.h")
    find_library(GLFW_LIBRARIES NAMES "glfw3.lib")
endif()

#GLEW
#set (GLEW_VERBOSE YES)
#find_package(GLEW )
#if (NOT GLEW_FOUND)
    find_path(GLFW_INCLUDE_DIRS NAMES "GL/glew.h")
    find_path(GLFW_INCLUDE_DIR NAMES "GL/glew.h")
    find_library(GLEW_LIBRARIES NAMES "glew32s.lib") # static
#endif()



add_subdirectory(00_hello_triangle)
add_subdirectory(01_extended_init)
if (FALSE)
add_subdirectory(02_shaders)
add_subdirectory(03_vertex_buffer_objects)
add_subdirectory(04_mats_and_vecs)
add_subdirectory(05_virtual_camera)
add_subdirectory(06_vcam_with_quaternion)
add_subdirectory(07_ray_picking)
add_subdirectory(08_phong)
add_subdirectory(09_texture_mapping)
add_subdirectory(10_screen_capture)
add_subdirectory(11_video_capture)
add_subdirectory(12_debugging_shaders)
add_subdirectory(13_mesh_import)
add_subdirectory(14_multi_tex)
add_subdirectory(15_phongtextures)
add_subdirectory(16_frag_reject)
add_subdirectory(17_alpha_blending)
add_subdirectory(18_spotlights)
add_subdirectory(19_fog)
add_subdirectory(20_normal_mapping)
add_subdirectory(21_cube_mapping)
add_subdirectory(22_geom_shaders)
add_subdirectory(23_tessellation_shaders)
add_subdirectory(24_gui_panels)
add_subdirectory(25_sprite_sheets)
add_subdirectory(26_bitmap_fonts)
add_subdirectory(27_font_atlas)
add_subdirectory(28_uniform_buffer_object)
add_subdirectory(29_particle_systems)
add_subdirectory(30_skinning_part_one)
add_subdirectory(31_skinning_part_two)
add_subdirectory(32_skinning_part_three)
add_subdirectory(33_extension_check)
add_subdirectory(34_framebuffer_switch)
add_subdirectory(35_image_kernel)
add_subdirectory(36_colour_picking)
add_subdirectory(37_deferred_shading)
add_subdirectory(38_texture_shadows)
add_subdirectory(39_texture_mapping_srgb)
add_subdirectory(40_compute_shader)
add_subdirectory(41_shader_hot_reload)
endif()

# end of file