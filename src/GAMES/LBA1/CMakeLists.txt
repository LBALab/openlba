# General sources
set(SOURCES_FILES VERSION.C PERSO.C OBJECT.C GLOBAL.C
        FLIPBOX.C DISKFUNC.C FICHE.C EXTRA.C INCRUST.C GRILLE.C
         MESSAGE.C AMBIANCE.C GAMEMENU.C
        GERETRAK.C GERELIFE.C HOLOMAP.C PLAYFLA.C 
        MSG_CUST.C)
set(SOURCES_FILES_ASM CPYMASK.ASM BALANCE.ASM FIRE.ASM ADFLI_A.ASM)

# DOS specific sources
#set(SOURCES_FILES_DOS)
set(SOURCES_FILES_ASM_DOS MCGA.ASM)

add_executable(lba ${SOURCES_FILES} ${SOURCES_FILES_ASM} ${SOURCES_FILES_WIN})
target_link_libraries(lba PRIVATE 3D ail ani fio obj poly svg sys SDL2::SDL2 SDL2::SDL2main ${SDL2_LIBRARIES})

target_include_directories(lba PRIVATE ../../LIB386/v3/H)
target_compile_definitions(lba PRIVATE ONE_GAME_DIRECTORY LBA_GAME CDROM
        $<$<STREQUAL:MVIDEO_BACKEND,none>:USE_MVIDEO_NONE>)

# Configure all ASM files to be recognized as such (workaround for bug where .ASM uppercase extension is not recognized)
foreach (FILE_ASM IN LISTS SOURCES_FILES_ASM SOURCES_FILES_ASM_DOS)
    set_source_files_properties(${FILE_ASM} PROPERTIES LANGUAGE ASM_MASM)
endforeach ()
