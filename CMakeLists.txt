cmake_minimum_required(VERSION 2.8)
project(MarkKiller)

include_directories("${MarkKiller_SOURCE_DIR}/include")

find_package(OpenCV REQUIRED)

find_package(Threads)

find_package(ZLIB)

find_package(LibLZMA)

set(FFMPEG_DIR /opt/ffmpeg)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${FFMPEG_DIR}/include)

find_library(FFMPEG_AVFORMAT NAMES avformat HINTS "${FFMPEG_DIR}/lib" "$ENV{FFMPEG_DIR}/lib")
find_library(FFMPEG_AVCODEC NAMES avcodec HINTS "${FFMPEG_DIR}/lib" "$ENV{FFMPEG_DIR}/lib")
find_library(FFMPEG_AVUTIL NAMES avutil HINTS "${FFMPEG_DIR}/lib" "$ENV{FFMPEG_DIR}/lib")
find_library(FFMPEG_SWSCALE NAMES swscale HINTS "${FFMPEG_DIR}/lib" "$ENV{FFMPEG_DIR}/lib")
find_library(FFMPEG_SWRESAMPLE NAMES swresample HINTS "${FFMPEG_DIR}/lib" "$ENV{FFMPEG_DIR}/lib")

find_library(LIB_VORBIS NAMES vorbis)
find_library(LIB_VORBISENC NAMES vorbisenc)
find_library(LIB_VORBISFILE NAMES vorbisfile)
find_library(LIB_X264 NAMES x264)
find_library(LIB_X265 NAMES x265)
find_library(LIB_VPX NAMES vpx)
find_library(LIB_OGG NAMES ogg)
find_library(LIB_OPUS NAMES opus)
find_library(LIB_XVID NAMES xvidcore)
find_library(LIB_MP3LAME NAMES mp3lame)
find_library(LIB_FDK-AAC NAMES fdk-aac)
find_library(LIB_SPEEX NAMES speex)
find_library(LIB_VA NAMES va)
find_library(LIB_BZ2 NAMES bz2)
find_library(LIB_VO_AACENC NAMES vo-aacenc)
find_library(LIB_DCA NAMES dca)
find_library(LIB_OPENCOREAMR NAMES opencore-amrnb)
find_library(LIB_SOXR NAMES soxr)

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MarkKiller_SOURCE_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MarkKiller_SOURCE_DIR}/lib )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${MarkKiller_SOURCE_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${MarkKiller_SOURCE_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${MarkKiller_SOURCE_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${MarkKiller_SOURCE_DIR}/lib )
set( CMAKE_DEBUG_POSTFIX "d")

set( MARKKILLER_TRANSCODE_SOURCE_FILES transcode/BasicTranscode.cpp transcode/Transcode.cpp)
set( MARKKILLER_TRANSCODE_HEADERS include/transcode/BasicTranscode.h include/transcode/Transcode.h )

set( MARKKILLER_XMARKER_SOURCE_FILES include/dlib/all/source.cpp xmarker/xmarker.cpp xmarker/mark.cpp xmarker/marktextbox.cpp xmarker/marklogo.cpp  )
set( MARKKILLER_XMARKER_HEADERS include/xmarker/xmarker.h include/xmarker/mark.h include/xmarker/marktextbox.h include/xmarker/marklogo.h )

set( MARKKILLER_TEXT_SOURCE_FILES text/erfilter.cpp  )
set( MARKKILLER_TEXT_HEADERS include/text/erfilter.hpp )

if(MSVC)
    foreach(flag
        CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_DEBUG_INIT
        CMAKE_CXX_FLAGS_RELEASE CMAKE_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG_INIT)
        string(REPLACE "/MD" "/MT" "${flag}" "${${flag}}")
    endforeach()
endif()

add_library(transcode STATIC ${MARKKILLER_TRANSCODE_SOURCE_FILES} ${MARKKILLER_TRANSCODE_HEADERS})
add_library(xmarker STATIC ${MARKKILLER_XMARKER_SOURCE_FILES} ${MARKKILLER_XMARKER_HEADERS})
add_library(text STATIC ${MARKKILLER_TEXT_SOURCE_FILES} ${MARKKILLER_TEXT_HEADERS})

set(SOURCE_FILES MarkKiller.cpp include/dlib/all/source.cpp)
add_executable(MarkKiller ${SOURCE_FILES} ${HEADER_FILES})
set(PRIVATE_LINK_LIBS xmarker transcode text)
set(FFMPEG_LINK_LIBS  ${FFMPEG_AVFORMAT} ${FFMPEG_AVCODEC} ${FFMPEG_SWSCALE} ${FFMPEG_AVUTIL} ${FFMPEG_SWRESAMPLE})
set(CODECS_LINK_LIBS ${LIB_X264} ${LIB_X265} ${LIB_FDK-AAC} ${LIB_MP3LAME} ${LIB_OPUS} ${LIB_XVID} ${LIB_VPX} ${LIB_OGG} ${LIB_VORBIS} ${LIB_VORBISENC} ${LIB_VORBISFILE}  ${LIB_SPEEX} ${LIB_VA} ${LIB_VO_AACENC} ${LIB_OPENCORE_AMRNB} ${LIB_SOXR})
set(PUBLIC_LINK_LIBS  ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${ZLIB_LIBRARIES} ${LIBLZMA_LIBRARIES} ${CMAKE_DL_LIBS} ${LIB_BZ2} ${LIB_DCA})
set(LINK_LIBS ${FFMPEG_LINK_LIBS} ${PUBLIC_LINK_LIBS} ${CODECS_LINK_LIBS} ${PRIVATE_LINK_LIBS})
target_link_libraries(MarkKiller ${LINK_LIBS} ${LINK_LIBS} ${LINK_LIBS})
                                                               	