project('antons_opengl_tutorials_book', 'c', 'cpp', default_options: ['warning_level=3'])

assimp_dep = dependency('assimp', required: false)
if not assimp_dep.found()
  cmake = import('cmake')
  assimp_opt = cmake.subproject_options()
  assimp_opt.add_cmake_defines({
    'BUILD_SHARED_LIBS': false,
    'CMAKE_BUILD_TYPE': get_option('buildtype') == 'debug' ? 'Debug' : 'Release',
    'ASSIMP_BUILD_ASSIMP_TOOLS': false,
    'ASSIMP_BUILD_TESTS': false,
    'ASSIMP_BUILD_ZLIB': false,

    'ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT': false,
    'ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT': false,

    'ASSIMP_BUILD_GLB_IMPORTER': true,
    'ASSIMP_BUILD_OBJ_IMPORTER': true,
    'ASSIMP_BUILD_GLTF_IMPORTER': true,
  })
  assimp_dep = cmake.subproject('assimp', options: assimp_opt).dependency('assimp')
endif

freetype_dep = dependency('freetype2', required: true)

dependencies = [dependency('gl'), dependency('glew', required: true), dependency('glfw3', required: true), assimp_dep]

executable('00_hello_triangle', sources: ['00_hello_triangle/main.c'], dependencies: dependencies)
executable('00_hello_triangle_gl2.1', sources: ['00_hello_triangle_gl2.1/main.c'], dependencies: dependencies)
executable('01_extended_init', sources: ['01_extended_init/main.c'], dependencies: dependencies)
executable('02_shaders', sources: ['02_shaders/main.c', '02_shaders/gl_utils.c'], dependencies: dependencies)
executable('03_vertex_buffer_objects', sources: ['03_vertex_buffer_objects/main.cpp', '03_vertex_buffer_objects/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies)
executable('04_mats_and_vecs', sources: ['04_mats_and_vecs/main.cpp', '04_mats_and_vecs/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies)
executable('05_virtual_camera', sources: ['05_virtual_camera/main.cpp', '05_virtual_camera/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('06_vcam_with_quaternion', sources: ['06_vcam_with_quaternion/main.cpp', '06_vcam_with_quaternion/gl_utils.cpp', '06_vcam_with_quaternion/obj_parser.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('07_ray_picking', sources: ['07_ray_picking/main.cpp', '07_ray_picking/gl_utils.cpp', '07_ray_picking/obj_parser.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('08_phong', sources: ['08_phong/main.cpp', '08_phong/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('09_texture_mapping', sources: ['09_texture_mapping/main.cpp', '09_texture_mapping/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('10_screen_capture', sources: ['10_screen_capture/main.cpp', '10_screen_capture/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('11_video_capture', sources: ['11_video_capture/main.cpp', '11_video_capture/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('12_debugging_shaders', sources: ['12_debugging_shaders/main.cpp', '12_debugging_shaders/gl_utils.cpp', '12_debugging_shaders/obj_parser.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('13_mesh_import', sources: ['13_mesh_import/main.cpp', '13_mesh_import/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('14_multi_tex', sources: ['14_multi_tex/main.cpp', '14_multi_tex/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('15_phongtextures', sources: ['15_phongtextures/main.cpp', '15_phongtextures/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('16_frag_reject', sources: ['16_frag_reject/main.cpp', '16_frag_reject/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('17_alpha_blending', sources: ['17_alpha_blending/main.cpp', '17_alpha_blending/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('18_spotlights', sources: ['18_spotlights/main.cpp', '18_spotlights/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('19_fog', sources: ['19_fog/main.cpp', '19_fog/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('20_normal_mapping', sources: ['20_normal_mapping/main.cpp', '20_normal_mapping/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('21_cube_mapping', sources: ['21_cube_mapping/main.cpp', '21_cube_mapping/gl_utils.cpp', '21_cube_mapping/obj_parser.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('22_geom_shaders', sources: ['22_geom_shaders/main.cpp', '22_geom_shaders/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('23_tessellation_shaders', sources: ['23_tessellation_shaders/main.cpp', '23_tessellation_shaders/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('24_gui_panels', sources: ['24_gui_panels/main.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('25_sprite_sheets', sources: ['25_sprite_sheets/main.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('26_bitmap_fonts', sources: ['26_bitmap_fonts/main.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('27_font_atlas_generator', sources: ['27_font_atlas/generator_main.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies + freetype_dep, include_directories: include_directories('common'))
executable('27_font_atlas_viewer', sources: ['27_font_atlas/viewer_main.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('28_uniform_buffer_object', sources: ['28_uniform_buffer_object/main.cpp', '28_uniform_buffer_object/gl_utils.cpp', '28_uniform_buffer_object/obj_parser.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('29_particle_systems', sources: ['29_particle_systems/main.cpp', '29_particle_systems/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('30_skinning_part_one', sources: ['30_skinning_part_one/main.cpp', '30_skinning_part_one/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('31_skinning_part_two', sources: ['31_skinning_part_two/main.cpp', '31_skinning_part_two/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('32_skinning_part_three', sources: ['32_skinning_part_three/main.cpp', '32_skinning_part_three/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('33_extension_check', sources: ['33_extension_check/main.c'], dependencies: dependencies, include_directories: include_directories('common'))
executable('34_framebuffer_switch', sources: ['34_framebuffer_switch/main.cpp', '34_framebuffer_switch/gl_utils.cpp', '34_framebuffer_switch/obj_parser.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('35_image_kernel', sources: ['35_image_kernel/main.cpp', '35_image_kernel/gl_utils.cpp', '35_image_kernel/obj_parser.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('36_colour_picking', sources: ['36_colour_picking/main.cpp', '36_colour_picking/gl_utils.cpp', '36_colour_picking/obj_parser.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('37_deferred_shading', sources: ['37_deferred_shading/main.cpp', '37_deferred_shading/gl_utils.cpp', '37_deferred_shading/obj_parser.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('38_texture_shadows', sources: ['38_texture_shadows/main.cpp', '38_texture_shadows/gl_utils.cpp', '38_texture_shadows/obj_parser.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('39_texture_mapping_srgb', sources: ['39_texture_mapping_srgb/main.cpp', '39_texture_mapping_srgb/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('40_compute_shader', sources: ['40_compute_shader/main.cpp', '40_compute_shader/gl_utils.cpp', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
executable('41_shader_hot_reload', sources: ['41_shader_hot_reload/main.c', 'common/maths_funcs.cpp'], dependencies: dependencies, include_directories: include_directories('common'))
