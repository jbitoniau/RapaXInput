CMAKE_MINIMUM_REQUIRED( VERSION 3.0 )

PROJECT( "RapaXInput" )

SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake" )

IF( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	
	IF( MSVC )
		INCLUDE( RapaConfigureVisualStudio )
	ENDIF()

	INCLUDE( RapaFindXInput )
	IF( XINPUT_FOUND )
		
		INCLUDE_DIRECTORIES( ${XInput_INCLUDE_DIR} )
		INCLUDE_DIRECTORIES( include )
						
		SET	( 	HEADERS
				include/RXITimestamp.h
				include/RXIController.h 
				include/RXIControllerManager.h
			)
		SET	(	SOURCES
				src/RXITimestamp.cpp
				src/RXIController.cpp
				src/RXIControllerManager.cpp 
			)
		SOURCE_GROUP("" FILES ${HEADERS} ${SOURCES} )		# Avoid "Header Files" and "Source Files" virtual folders in VisualStudio

		SET(CMAKE_DEBUG_POSTFIX "d")
		ADD_LIBRARY( ${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES} )
		TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${XInput_LIBRARY} ) 
		
		#
		# Install
		#
		INSTALL(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
				LIBRARY DESTINATION lib
				ARCHIVE DESTINATION lib
				RUNTIME DESTINATION bin )
				#INCLUDES DESTINATION include )		# If uncommented, the ${PROJECT_NAME} target contains INCLUDE_DIRECTORIES information. Importing the target automatically adds this directory to the INCLUDE_DIRECTORIES.
		SET( TARGET_NAMESPACE Rapa:: )
		INSTALL( FILES ${HEADERS} DESTINATION include COMPONENT Devel )		
		EXPORT( EXPORT ${PROJECT_NAME}Targets FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake" NAMESPACE ${TARGET_NAMESPACE} )
		CONFIGURE_FILE( cmake/${PROJECT_NAME}Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake" @ONLY )
		SET( ConfigPackageLocation lib/cmake/${PROJECT_NAME} )
		INSTALL(EXPORT ${PROJECT_NAME}Targets
				FILE ${PROJECT_NAME}Targets.cmake
				NAMESPACE ${TARGET_NAMESPACE}
				DESTINATION ${ConfigPackageLocation} )
		INSTALL( FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake" DESTINATION ${ConfigPackageLocation} COMPONENT Devel )

		ADD_SUBDIRECTORY( samples )
		
	ELSE()
		MESSAGE("XInput not found")
	ENDIF()
ELSE()
	MESSAGE("${PROJECT_NAME} is Windows only")
ENDIF()
