SET(2GEOM_DEPENDS gtk+-2.0 gtkmm-2.4 cairomm-1.0 cairo gsl blas pycairo)
include(UsePkgConfig)

# Dependencies Packages

FOREACH(dep ${2GEOM_DEPENDS})
    # This is a hack due to a bug in Cmake vars system,temp fix until cmake 2.4.4 is out //verbalshadow
    IF("${dep}" MATCHES "gtk\\+-2.0")
        SET(dep_name "GTK2")
    ELSE("${dep}" MATCHES "gtk\\+-2.0")
        SET(dep_name "${dep}")
    ENDIF("${dep}" MATCHES "gtk\\+-2.0")
    
    PKGCONFIG_FOUND(${dep} "${dep}_FOUND")
    PKGCONFIG(${dep} "${dep_name}_INCLUDE_DIR" "${dep_name}_LINK_DIR" "${dep_name}_LINK_FLAGS" "${dep_name}_CFLAGS")
#    PKGCONFIG_VERSION(${dep} "${dep}_VERSION")
    IF("${dep}_FOUND")
        message(STATUS "${dep} Includes, Compile and Link Flags: FOUND")
    ELSE("${dep}_FOUND")
        message(STATUS "${dep} Includes, Compile and Link Flags: NOT FOUND")
    ENDIF("${dep}_FOUND")
ENDFOREACH(dep)
# end Dependencies

INCLUDE(FindBoost)

# WTF! All other standard checking macros don't work!
INCLUDE (CheckCXXSourceCompiles)

MACRO(CHECK_MATH_FUNCTION FUNCTION VAR)
    CHECK_CXX_SOURCE_COMPILES("#include <math.h>\nint main() { double a=0.5,b=0.5,c=0.5; int i=1,j=2,k=3; ${FUNCTION}; return 0; }" ${VAR})
ENDMACRO(CHECK_MATH_FUNCTION)

CHECK_MATH_FUNCTION("sincos(a,&b,&c)" HAVE_SINCOS)
CHECK_MATH_FUNCTION("round(a)" HAVE_ROUND)
CHECK_MATH_FUNCTION("trunc(a)" HAVE_TRUNC)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/2geom/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
