#
# A function to search for XInput header and lib in a given SDK root path (Windows Platform SDK or DirectX SDK).
# This function looks for the x64 lib if compiling in 64 bits, x86 otherwise.
# The function sets the following variables:
#  XINPUT_FOUND 
#  XInput_INCLUDE_DIR 
#  XInput_LIBRARY 
# 
FUNCTION( FINDXINPUT SDK_ROOT_PATH )

	# Determine the header path
	MESSAGE("Searching for XInput in ${SDK_ROOT_PATH}")
	FIND_PATH( XInput_INCLUDE_DIR NAMES XInput.h 
				PATHS 
				"${SDK_ROOT_PATH}/Include/um"
				"${SDK_ROOT_PATH}/Include/" 
				NO_DEFAULT_PATH  )
		
	# Determine the library path depending on the architecture (x86 or x64)
	# The structure of the SDKs (DirectX and Platform) varies significantly from version to version.
	# In the DirectX SDK (at least for the June 2010 one):
	#	/?????
	#		/<Root>
	#			/Lib              (for x86) 
	#				/x64          (for x64)
	#
	# In old Platform SDKs:
	#	/Microsoft SDKs
	#		/<Root>               (v7.0A, v7.1A, v8.0, etc)
	#			/Lib     
	#		 		/x86          (for x86)
	#		 		/x64          (for x64)
	# 
	# In more recent Platform SDKs called "Windows Kits" (for eg: Microsoft Windows SDK for Windows 7 and .NET Framework 4)
	#	/Windows Kits
	#		/<Root>               (8.0)
	#			/Lib
	#				/win8
	#					/um
	#						/x86  (for x86)
	#						/x64  (for x64)
	#
	# Or in Microsoft Windows SDK for Windows 8.1 and .NET Framework 4.5.1:
	#	/Windows Kits
	#		/<Root>               (8.1)
	#			/Lib
	#				/winv6.3
	#					/um
	#						/x86  (for x86)
	#						/x64  (for x64)
	IF ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		FIND_LIBRARY( XInput_LIBRARY NAMES XInput 
						PATHS 
						"${SDK_ROOT_PATH}/Lib/winv6.3/um/x64" 
						"${SDK_ROOT_PATH}/Lib/win8/um/x64" 
						"${SDK_ROOT_PATH}/Lib/x64" 
						NO_DEFAULT_PATH )
	ELSE()
		FIND_LIBRARY( XInput_LIBRARY NAMES XInput 
						PATHS 
						"${SDK_ROOT_PATH}/Lib/winv6.3/um/x86" 
						"${SDK_ROOT_PATH}/Lib/win8/um/x86" 
						"${SDK_ROOT_PATH}/Lib/x86" 
						"${SDK_ROOT_PATH}/Lib" 
						NO_DEFAULT_PATH )
	ENDIF()
		
	INCLUDE( FindPackageHandleStandardArgs )
	FIND_PACKAGE_HANDLE_STANDARD_ARGS( XInput DEFAULT_MSG XInput_LIBRARY XInput_INCLUDE_DIR )
	# or mode2: FIND_PACKAGE_HANDLE_STANDARD_ARGS( XInput REQUIRED_VARS XInput_LIBRARY XInput_INCLUDE_DIR )
	
	# Promote the XINPUT_FOUND variable to parent scope (otherwise it disappears at the function return).
	# Not needed for XInput_INCLUDE_DIR and XInput_LIBRARY as they are put in the cache which makes them global
	SET( XINPUT_FOUND ${XINPUT_FOUND} PARENT_SCOPE )	

ENDFUNCTION()

#
# We go through a list of SDK root-paths and try to find XInput there.
# The list is stored in the cached variable XInput_SDK_SEARCH_PATHS.
# The paths are checked one after the other in order.
#
# Once done, the following variables are defined:
#  XINPUT_FOUND 
#  XInput_INCLUDE_DIR 
#  XInput_LIBRARY 
#
# The user can set XInput_SDK_SEARCH_PATHS with his own search path(s)
# if he wants a specific version of XInput or if it has been installed
# in an exotic SDK folder location.
#
# He can of course by-pass the search completely and provide the exact
# paths of the header and lib using the XInput_INCLUDE_DIR and XInput_LIBRARY 
# variables.
#
IF( CMAKE_SYSTEM_NAME MATCHES Windows )
	
	# The default paths where to look for XInput. This can be user modifiable
	# We look for the most recent SDKs first
	SET( XInput_SDK_SEARCH_PATHS 
		 "C:/Program Files (x86)/Windows Kits/8.1"							# XInput 1.4
		 "C:/Program Files (x86)/Windows Kits/8.0"							# XInput 1.4
		 "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)"			# XInput 1.3
		 "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A"				# XInput 9.0.1
		 "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A"				# XInput 9.0.1
		 CACHE STRING 
		 "The list of SDK root-paths where to search for XInput" )
	
	# Goes through each path and try it
	FOREACH( SDK_ROOT_PATH ${XInput_SDK_SEARCH_PATHS} )
		IF( NOT XINPUT_FOUND )
			FINDXINPUT( ${SDK_ROOT_PATH} )
		ENDIF()
	ENDFOREACH()

ENDIF()

