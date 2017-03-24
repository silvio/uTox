project(utoxAV LANGUAGES C)

add_library(utoxAV STATIC
    utox_av.c
    audio.c
    video.c
    )

if(APPLE)
elseif(WIN32)
    target_link_libraries(utoxAV OpenAL32)
else()
    target_link_libraries(utoxAV openal)
else()
    target_link_libraries(utoxAV opus)
endif()


if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    include(FindPkgConfig)
    pkg_check_modules(LIBTOXAV REQUIRED libtoxav)
    pkg_check_modules(OPENAL REQUIRED openal)
    pkg_check_modules(FILTERAUDIO REQUIRED filteraudio)

    include_directories(
        ${LIBTOXAV_INCLUDEDIR}
        ${OPENAL_INCLUDEDIR}
        ${FILTERAUDIO_INCLUDEDIR}
    )
    set(LIB_FILTERAUDIO ${FILTERAUDIO_LIBRARIES})
else()
    set(LIB_FILTERAUDIO filteraudio)
    set(FILTERAUDIO_FOUND 1)
endif()

if(FILTERAUDIO_FOUND)
    target_link_libraries(utoxAV ${LIB_FILTERAUDIO})
endif()
