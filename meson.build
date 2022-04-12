project(
  'isasim',
  ['c', 'cpp'],
  version : run_command(
    [find_program('python', 'python3'), 'bin/meson_get_version.py']
  ).stdout(),
  license : 'MIT',
  meson_version : '>= 0.52',
  default_options : ['buildtype=debug'] #, 'b_ndebug=if-release', 'c_std=c99', 'cpp_std=c++17']
)

cc = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')
buildtype = get_option('buildtype')
builddir = meson.current_build_dir()

null_dep = dependency('', required : false)

# Arguments for the preprocessor, put these in a separate array from the C and
# C++ (cpp in meson terminology) arguments since they need to be added to the
# default arguments for both C and C++.
pre_args = [
  '-D__STDC_CONSTANT_MACROS',
  '-D__STDC_FORMAT_MACROS',
  '-D__STDC_LIMIT_MACROS',
  '-DPACKAGE_VERSION="@0@"'.format(meson.project_version()),
  '-DPACKAGE_BUGREPORT="https://gitlab.freedesktop.org/mesa/mesa/-/issues"',
]

extra_cpp_args = ['-Wall', '-std=c++17']

if buildtype == 'release'
  extra_cpp_args += ['-O3', '-g', '-Wno-strict-aliasing', '-msse4.1', '-flto']
else
  extra_cpp_args += ['-O0', '-g', '-Wno-strict-aliasing', '-msse4.1']
endif

extra_c_args = extra_cpp_args

#if cpp.get_id() == 'gcc'
#  extra_cpp_args += ['-std=c++17']
#else
#  extra_cpp_args += []
#endif

link_args = ['-ldl', '-lpthread', '-L' + meson.current_build_dir(), '-lstdc++' ]

model_symbol_list = meson.current_source_dir() + '/model_symbol_export.def'

#pymodule = import('python3')
#python3 = pymodule.find_python()

DESIGN_ROOT = run_command([find_program('python', 'python3'), '-c', 'import os; print(os.environ["DESIGN_ROOT"])']).stdout().strip()
GEM5_ROOT = run_command([find_program('python', 'python3'), '-c', 'import os; print(os.environ["GEM5_ROOT"])']).stdout().strip()
CUDA_INSTALL_PATH = run_command([find_program('python', 'python3'), '-c', 'import os; print(os.environ["CUDA_INSTALL_PATH"])']).stdout().strip()

message('DESIGN_ROOT is ' + DESIGN_ROOT)
message('GEM5_ROOT is ' + GEM5_ROOT)
message('CUDA_INSTALL_PATH is ' + CUDA_INSTALL_PATH)

executor_link_args = link_args + ['-L' + DESIGN_ROOT + '/cosim', '-lcudart'] + ['-Wl,--version-script='+model_symbol_list]
executor_inc = ['executor',
  DESIGN_ROOT + '/opu/coasm',
  DESIGN_ROOT + '/opu/umd',
  DESIGN_ROOT + '/libcuda',
  CUDA_INSTALL_PATH + '/include'
  ]

executor_src = [
  'executor/IsaSim.cpp',
  'executor/Kernel.cpp',
  'executor/Warp.cpp',
  'executor/WarpState.cpp',
  #'executor/Compute.cpp',
  #'executor/ComputeUnit.cpp',
  #'executor/CopyEngine.cpp',
  #'executor/StreamProcessor.cpp',
  #'executor/Engine.cpp',
  #'executor/Memory.cpp',
  #'executor/MemoryImpl.cc',
  #'executor/Mmio.cpp',
  'executor/ThreadBlock.cpp',
  'executor/ThreadItem.cpp',
  #'executor/Warp.cpp',
  'executor/Instruction.cpp',
  'executor/InstructionDLS.cpp',
  'executor/InstructionSLS.cpp',
  'executor/InstructionVLS.cpp',
  #'executor/InstructionMTBUF.cpp',
  'executor/InstructionVMUBUF.cpp',
  #'executor/InstructionSMRD.cpp',
  'executor/InstructionSOP1.cpp',
  'executor/InstructionSOP2.cpp',
  'executor/InstructionSOPC.cpp',
  'executor/InstructionSOPK.cpp',
  'executor/InstructionSOPP.cpp',
  'executor/InstructionVOP1.cpp',
  'executor/InstructionVOP2.cpp',
  'executor/InstructionVOP3A.cpp',
  'executor/InstructionVOP3B.cpp',
  'executor/InstructionVOPC.cpp'
  ]

executor = shared_library(
  'isasim',
  executor_src,
  include_directories : include_directories(executor_inc),
  install : false,
  cpp_args : extra_cpp_args,
  link_args : executor_link_args,
  )


#test_simple_src = [
#  'test/main.cpp',
#  'test/module/Simple.cpp'
#  ]

#test_simple = executable(
#  'test_simple',
#  test_simple_src,
#  include_directories : include_directories(module_inc),
#  install : false,
#  cpp_args : extra_cpp_args,
#  link_args : module_link_args,
#  #dependencies: declare_dependency(link_with : )
#  )

