include(CheckCXXSourceCompiles)
include(CheckSymbolExists)

CHECK_CXX_SOURCE_COMPILES("
    #include <unicode/udat.h>
    int main() { UDateFormatSymbolType e = UDAT_STANDALONE_SHORTER_WEEKDAYS; }
" HAVE_UDAT_STANDALONE_SHORTER_WEEKDAYS)

set(CMAKE_REQUIRED_INCLUDES ${ICU_HOMEBREW_INC_PATH})
check_symbol_exists(
    ucol_setMaxVariable
    "unicode/ucol.h"
    HAVE_SET_MAX_VARIABLE)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h)
