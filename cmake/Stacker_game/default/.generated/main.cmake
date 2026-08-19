include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(Stacker_game_default_library_list )

# Handle files with suffix (s|as|asm|AS|ASM|As|aS|Asm), for group default-XC8
if(Stacker_game_default_default_XC8_FILE_TYPE_assemble)
add_library(Stacker_game_default_default_XC8_assemble OBJECT ${Stacker_game_default_default_XC8_FILE_TYPE_assemble})
    Stacker_game_default_default_XC8_assemble_rule(Stacker_game_default_default_XC8_assemble)
    list(APPEND Stacker_game_default_library_list "$<TARGET_OBJECTS:Stacker_game_default_default_XC8_assemble>")

endif()

# Handle files with suffix S, for group default-XC8
if(Stacker_game_default_default_XC8_FILE_TYPE_assemblePreprocess)
add_library(Stacker_game_default_default_XC8_assemblePreprocess OBJECT ${Stacker_game_default_default_XC8_FILE_TYPE_assemblePreprocess})
    Stacker_game_default_default_XC8_assemblePreprocess_rule(Stacker_game_default_default_XC8_assemblePreprocess)
    list(APPEND Stacker_game_default_library_list "$<TARGET_OBJECTS:Stacker_game_default_default_XC8_assemblePreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC8
if(Stacker_game_default_default_XC8_FILE_TYPE_compile)
add_library(Stacker_game_default_default_XC8_compile OBJECT ${Stacker_game_default_default_XC8_FILE_TYPE_compile})
    Stacker_game_default_default_XC8_compile_rule(Stacker_game_default_default_XC8_compile)
    list(APPEND Stacker_game_default_library_list "$<TARGET_OBJECTS:Stacker_game_default_default_XC8_compile>")

endif()


# Main target for this project
add_executable(Stacker_game_default_image_eU9zVWPZ ${Stacker_game_default_library_list})

set_target_properties(Stacker_game_default_image_eU9zVWPZ PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    ADDITIONAL_CLEAN_FILES "${output_extensions}"
    RUNTIME_OUTPUT_DIRECTORY "${Stacker_game_default_output_dir}")
target_link_libraries(Stacker_game_default_image_eU9zVWPZ PRIVATE ${Stacker_game_default_default_XC8_FILE_TYPE_link})

# Add the link options from the rule file.
Stacker_game_default_link_rule( Stacker_game_default_image_eU9zVWPZ)


