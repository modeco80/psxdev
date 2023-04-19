macro(_psxdev_target_gcc target)
	set(_CORE_COMPILE_ARGS -Wall -Wextra)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		set(_CORE_COMPILE_ARGS ${_CORE_COMPILE_ARGS} -Werror)
	endif()
endmacro()

function(psxdev_target target)
	target_compile_definitions(${target} PRIVATE "$<$<CONFIG:DEBUG>:PSXDEV_DEBUG>")
	target_compile_features(${target} PUBLIC cxx_std_20)

	_psxdev_target_gcc(${target})

	# default compile options to the core compile flags set by the macro implementation
	target_compile_options(${target} PRIVATE ${_CORE_COMPILE_ARGS})

	# Automatically make anything which does the thing do the thing really thingily.
	make_psexe(${target})
endfunction()
