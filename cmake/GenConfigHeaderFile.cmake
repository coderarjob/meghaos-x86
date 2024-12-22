
# ==================================================================================================
# Generates C header file to include all the build options for the Kernel.
# The template of the header is taken from another file.
#
# It expects below arguments:
# GenHeader - 1 (always), used to determine if the script was called properly.
# Definitions - Build definitions each separated by space. Individual definitions must in the
#               following format:
#               * Definition
#               * Definition=value (no space around =)
# HeaderFileDestinationPath - Full file name of the header file to generate
# HeaderFileTemplatePath - Full file name of the template header file to generate from.
# ==================================================================================================
if (GenHeader)
    # Convert definitions from string to a CMake list.
    string(REPLACE " " ";" DefinitionList ${Definitions})

    foreach(def IN LISTS DefinitionList)
        string(REPLACE "=" "\t" def ${def})
        set(GEN_HEADER_CONTENTS "${GEN_HEADER_CONTENTS}\n#define ${def}")
    endforeach()

    configure_file(${HeaderFileTemplatePath} ${HeaderFileDestinationPath})
else()
    message(FATAL_ERROR "Should not be called/included on its own")
endif()
# ==================================================================================================
