CMAKE_MINIMUM_REQUIRED( VERSION 2.4 )

PROJECT( nanobrunch )
ADD_EXECUTABLE( nanobrunch nanobrunch.cpp )
INCLUDE( FindImageMagick )

FIND_PACKAGE( ImageMagick COMPONENTS Magick++ )
IF( ImageMagick_FOUND )
    INCLUDE_DIRECTORIES( ${ImageMagick_INCLUDE_DIRS} )
    TARGET_LINK_LIBRARIES( nanobrunch ${ImageMagick_LIBRARIES} )
ENDIF( ImageMagick_FOUND )

FIND_PATH(GMP_INCLUDE_DIR NAMES gmp.h)
FIND_PATH(GMPXX_INCLUDE_DIR NAMES gmpxx.h)
FIND_LIBRARY(GMP_LIBRARIES NAMES gmp)
FIND_LIBRARY(GMPXX_LIBRARIES NAMES gmpxx)
IF( GMPXX_LIBRARIES )
    INCLUDE_DIRECTORIES( ${GMP_INCLUDE_DIR} ${GMPXX_INCLUDE_DIR} )
    TARGET_LINK_LIBRARIES( nanobrunch ${GMP_LIBRARIES} ${GMPXX_LIBRARIES} )
ENDIF( GMPXX_LIBRARIES )
