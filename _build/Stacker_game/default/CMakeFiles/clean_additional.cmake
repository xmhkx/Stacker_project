# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "C:\\Users\\Matth\\Stacker_project\\out\\Stacker_game\\default.cmf"
  "C:\\Users\\Matth\\Stacker_project\\out\\Stacker_game\\default.hex"
  "C:\\Users\\Matth\\Stacker_project\\out\\Stacker_game\\default.hxl"
  "C:\\Users\\Matth\\Stacker_project\\out\\Stacker_game\\default.mum"
  "C:\\Users\\Matth\\Stacker_project\\out\\Stacker_game\\default.o"
  "C:\\Users\\Matth\\Stacker_project\\out\\Stacker_game\\default.sdb"
  "C:\\Users\\Matth\\Stacker_project\\out\\Stacker_game\\default.sym"
  )
endif()
