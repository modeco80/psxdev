set(CMAKE_SYSTEM_NAME PlayStation)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR "mips32")

# Make it so cmake sees modules here, so it can import our PlayStation platform. 
# Otherwise, CMake will yell and complain (rightfully.)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

set(PS1 ON)

if("${NUGGET}" STREQUAL "")
    set(NUGGET "${PROJECT_SOURCE_DIR}/third_party/nugget")
endif()

set(CMAKE_C_COMPILER mipsel-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER mipsel-linux-gnu-g++)

# HACK
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

set(_ARCHFLAGS "-march=mips1 -mabi=32 -EL -fno-pic -mno-shared -mno-abicalls -mfp32 -mno-llsc")


set(CMAKE_ASM_FLAGS_INIT "${_ARCHFLAGS} -I${NUGGET}")

set(CMAKE_C_FLAGS_INIT "${_ARCHFLAGS} -I${NUGGET} -I${NUGGET}/psyq/include -fno-stack-protector -nostdlib -ffreestanding -fno-ident -fno-unwind-tables -fno-asynchronous-unwind-tables")
set(CMAKE_C_FLAGS_RELEASE_INIT "${CMAKE_C_FLAGS_INIT} -fomit-frame-pointer")
set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -fno-rtti -fno-exceptions -fno-threadsafe-statics")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_C_FLAGS_RELEASE_INIT} ${CMAKE_CXX_FLAGS_INIT}")
# I suppose if you really wanted to you could have this output CPE's, but that doesn't really have much of a use
# (outside of e.g: DTL-H2(0|5)00, but those both require way older tooling to really work properly)
set(CMAKE_EXE_LINKER_FLAGS_INIT " -L${NUGGET}/psyq/lib -T ${NUGGET}/nooverlay.ld -T ${NUGGET}/ps-exe.ld -Wl,--oformat=elf32-tradlittlemips")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT "-s")

set(CMAKE_FIND_ROOT_PATH ${NUGGET})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


function(make_psexe target)
	add_custom_command(TARGET ${target}
		POST_BUILD
		COMMAND mipsel-linux-gnu-objcopy -O binary $<TARGET_FILE_DIR:${target}>/$<TARGET_FILE_NAME:${target}> $<TARGET_FILE_DIR:${target}>/${target}.exe
		USES_TERMINAL
		VERBATIM
		COMMENT "Creating ${target}.exe"
    )
	#add_dependencies
endfunction()
	