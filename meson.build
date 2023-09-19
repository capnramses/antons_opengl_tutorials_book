project('antons_opengl_tutorials_book', 'c', 'cpp', default_options: ['warning_level=3'])

gl_dep = dependency('gl')

assimp_dep = dependency('assimp', method: 'pkg-config', required: false)
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

freetype_dep = dependency('freetype2', method: 'pkg-config', required: false)
if not freetype_dep.found()
  freetype_dep = dependency('freetype2', static: true, required: true)
endif

glew_dep = dependency('glew', method: 'pkg-config', required: false)
if not glew_dep.found()
  glew_dep = dependency('glew', static: true, required: true)
endif

glfw3_dep = dependency('glfw3', method: 'pkg-config', required: false)
if not glfw3_dep.found()
  glfw3_dep = dependency('glfw3', static: true, required: true)
endif

components = {
  '00_hello_triangle': ['00_hello_triangle/main.c'],
  '00_hello_triangle_gl2.1': ['00_hello_triangle_gl2.1/main.c'],
  '01_extended_init': ['01_extended_init/main.c'],
  '02_shaders': ['02_shaders/main.c', '02_shaders/gl_utils.c'],
  '03_vertex_buffer_objects': ['03_vertex_buffer_objects/main.cpp', '03_vertex_buffer_objects/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '04_mats_and_vecs': ['04_mats_and_vecs/main.cpp', '04_mats_and_vecs/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '05_virtual_camera': ['05_virtual_camera/main.cpp', '05_virtual_camera/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '06_vcam_with_quaternion': ['06_vcam_with_quaternion/main.cpp', '06_vcam_with_quaternion/gl_utils.cpp', '06_vcam_with_quaternion/obj_parser.cpp', 'common/maths_funcs.cpp'],
  '07_ray_picking': ['07_ray_picking/main.cpp', '07_ray_picking/gl_utils.cpp', '07_ray_picking/obj_parser.cpp', 'common/maths_funcs.cpp'],
  '08_phong': ['08_phong/main.cpp', '08_phong/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '09_texture_mapping': ['09_texture_mapping/main.cpp', '09_texture_mapping/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '10_screen_capture': ['10_screen_capture/main.cpp', '10_screen_capture/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '11_video_capture': ['11_video_capture/main.cpp', '11_video_capture/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '12_debugging_shaders': ['12_debugging_shaders/main.cpp', '12_debugging_shaders/gl_utils.cpp', '12_debugging_shaders/obj_parser.cpp', 'common/maths_funcs.cpp'],
  '13_mesh_import': ['13_mesh_import/main.cpp', '13_mesh_import/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '14_multi_tex': ['14_multi_tex/main.cpp', '14_multi_tex/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '15_phongtextures': ['15_phongtextures/main.cpp', '15_phongtextures/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '16_frag_reject': ['16_frag_reject/main.cpp', '16_frag_reject/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '17_alpha_blending': ['17_alpha_blending/main.cpp', '17_alpha_blending/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '18_spotlights': ['18_spotlights/main.cpp', '18_spotlights/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '19_fog': ['19_fog/main.cpp', '19_fog/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '20_normal_mapping': ['20_normal_mapping/main.cpp', '20_normal_mapping/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '21_cube_mapping': ['21_cube_mapping/main.cpp', '21_cube_mapping/gl_utils.cpp', '21_cube_mapping/obj_parser.cpp', 'common/maths_funcs.cpp'],
  '22_geom_shaders': ['22_geom_shaders/main.cpp', '22_geom_shaders/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '23_tessellation_shaders': ['23_tessellation_shaders/main.cpp', '23_tessellation_shaders/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '24_gui_panels': ['24_gui_panels/main.cpp', 'common/maths_funcs.cpp'],
  '25_sprite_sheets': ['25_sprite_sheets/main.cpp', 'common/maths_funcs.cpp'],
  '26_bitmap_fonts': ['26_bitmap_fonts/main.cpp', 'common/maths_funcs.cpp'],
  '27_font_atlas': ['27_font_atlas/viewer_main.cpp', 'common/maths_funcs.cpp'],
  '28_uniform_buffer_object': ['28_uniform_buffer_object/main.cpp', '28_uniform_buffer_object/gl_utils.cpp', '28_uniform_buffer_object/obj_parser.cpp', 'common/maths_funcs.cpp'],
  '29_particle_systems': ['29_particle_systems/main.cpp', '29_particle_systems/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '30_skinning_part_one': ['30_skinning_part_one/main.cpp', '30_skinning_part_one/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '31_skinning_part_two': ['31_skinning_part_two/main.cpp', '31_skinning_part_two/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '32_skinning_part_three': ['32_skinning_part_three/main.cpp', '32_skinning_part_three/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '33_extension_check': ['33_extension_check/main.c'],
  '34_framebuffer_switch': ['34_framebuffer_switch/main.cpp', '34_framebuffer_switch/gl_utils.cpp', '34_framebuffer_switch/obj_parser.cpp', 'common/maths_funcs.cpp'],
  '35_image_kernel': ['35_image_kernel/main.cpp', '35_image_kernel/gl_utils.cpp', '35_image_kernel/obj_parser.cpp', 'common/maths_funcs.cpp'],
  '36_colour_picking': ['36_colour_picking/main.cpp', '36_colour_picking/gl_utils.cpp', '36_colour_picking/obj_parser.cpp', 'common/maths_funcs.cpp'],
  '37_deferred_shading': ['37_deferred_shading/main.cpp', '37_deferred_shading/gl_utils.cpp', '37_deferred_shading/obj_parser.cpp', 'common/maths_funcs.cpp'],
  '38_texture_shadows': ['38_texture_shadows/main.cpp', '38_texture_shadows/gl_utils.cpp', '38_texture_shadows/obj_parser.cpp', 'common/maths_funcs.cpp'],
  '39_texture_mapping_srgb': ['39_texture_mapping_srgb/main.cpp', '39_texture_mapping_srgb/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '40_compute_shader': ['40_compute_shader/main.cpp', '40_compute_shader/gl_utils.cpp', 'common/maths_funcs.cpp'],
  '41_shader_hot_reload': ['41_shader_hot_reload/main.c', 'common/maths_funcs.cpp'],
}

foreach name, sources: components
  exe = executable(name, sources: sources, dependencies: [gl_dep, glew_dep, glfw3_dep, assimp_dep], include_directories: include_directories('common'))

  # Just in 27_font_atlas, first run the generator then run the excutable itself
  if name == '27_font_atlas'
    generator = executable('27_font_atlas_generator', sources: ['27_font_atlas/generator_main.cpp'], dependencies: freetype_dep)
    test('27_font_atlas_generator', generator, workdir: meson.current_source_dir() / name, is_parallel: false)
  endif

  test(name, exe, workdir: meson.current_source_dir() / name, is_parallel: false)
endforeach
