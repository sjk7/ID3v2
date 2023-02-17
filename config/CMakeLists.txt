# 'Just My Code' in VS
if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.15" )
    add_compile_options( /JMC)
endif()

# Edit and Continue in VS
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.15")
  add_compile_options(/ZI)
  add_link_options(/INCREMENTAL)
endif()

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()