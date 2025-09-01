# General sources
set(SOURCES_FILES DECORS.CPP GLOBEXT.CPP LBA_EXT.CPP LOADISLE.CPP MAPTOOLS.CPP TERRAIN.CPP
                  BOXZBUF.CPP LINERAIN.CPP)

add_library(3dext ${SOURCES_FILES})
target_compile_definitions(3dext PRIVATE ONE_GAME_DIRECTORY LBA_GAME CDROM)
target_link_libraries(3dext PUBLIC sys svg 3D obj poly ani fio)
target_include_directories(3dext PUBLIC ../../../LIB386/v3/H)

# Source files in folder but not used on CMakeScript:
# - DRAWSKY.CPP (but used as included on source file "TERRAIN.CPP")
# - VAR_EXT.CPP (but used as included on source file "GLOBEXT.CPP")
