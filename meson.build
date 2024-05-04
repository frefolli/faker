project(
  'sigmod',
  'cpp',
  version : '0.1',
  default_options : [
    'warning_level=3',
    'optimization=3',
    'cpp_std=c++17'
  ]
)

include = include_directories('include')

sigmod = library(
  'sigmod', [
    'src/sigmod/database.cc',
    'src/sigmod/debug.cc',
    'src/sigmod/query.cc',
    'src/sigmod/query_set.cc',
    'src/sigmod/random.cc',
    'src/sigmod/record.cc',
    'src/sigmod/scoreboard.cc',
  ], include_directories: include)

openmp = dependency('openmp')

main = executable(
  'main.exe', [
    'src/main.cc',
  ], dependencies : [
    openmp,
  ], link_with: [
    sigmod,
  ], install : true,
  include_directories: include
)
