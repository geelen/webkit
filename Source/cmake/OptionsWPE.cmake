include(GNUInstallDirs)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 1)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

WEBKIT_OPTION_BEGIN()
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_3D_RENDERING ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_ACCELERATED_2D_CANVAS ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_CSS_IMAGE_SET ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_CSS_REGIONS ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_CSS_SELECTORS_LEVEL4 ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_INSPECTOR ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_MEDIA_CONTROLS_SCRIPT ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_NETSCAPE_PLUGIN_API OFF)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_NETWORK_PROCESS ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_PICTURE_SIZES ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_REQUEST_ANIMATION_FRAME ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_SHARED_WORKERS ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_TEMPLATE_ELEMENT ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_TOUCH_EVENTS ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_VIDEO ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_VIDEO_TRACK ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_VIEW_MODE_CSS_MEDIA ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_WEBGL ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_WEB_TIMING ON)
WEBKIT_OPTION_DEFAULT_PORT_VALUE(ENABLE_XHR_TIMEOUT ON)
WEBKIT_OPTION_END()

set(ENABLE_WEBCORE ON)
set(ENABLE_WEBKIT OFF)
set(ENABLE_WEBKIT2 ON)
set(ENABLE_API_TESTS OFF)

if (DEVELOPER_MODE)
    set(ENABLE_TOOLS ON)
else ()
    set(ENABLE_TOOLS OFF)
endif ()

set(WTF_LIBRARY_TYPE STATIC)
set(JavaScriptCore_LIBRARY_TYPE STATIC)
set(WebCore_LIBRARY_TYPE STATIC)
set(WebKit2_WebProcess_OUTPUT_NAME WPEWebProcess)
set(WebKit2_NetworkProcess_OUTPUT_NAME WPENetworkProcess)

find_package(ICU REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(GLIB 2.40.0 REQUIRED COMPONENTS gio gobject gthread gmodule)

find_package(Cairo 1.10.2 REQUIRED)
find_package(CairoGL 1.10.2 REQUIRED COMPONENTS cairo-egl)
find_package(Fontconfig 2.8.0 REQUIRED)
find_package(Freetype2 2.4.2 REQUIRED)
find_package(HarfBuzz 0.9.2 REQUIRED)
find_package(JPEG REQUIRED)
find_package(LibSoup 2.40.3 REQUIRED)
find_package(Libxkbcommon 0.4.0 REQUIRED)
find_package(LibXml2 2.8.0 REQUIRED)
find_package(LibXslt 1.1.7 REQUIRED)
find_package(PNG REQUIRED)
find_package(Sqlite REQUIRED)
find_package(Wayland 1.6.0 REQUIRED)
find_package(WebP REQUIRED)

find_package(OpenGLES2 REQUIRED)
find_package(EGL REQUIRED)

find_package(Weston 1.6.0 REQUIRED)
if (WESTON_FOUND)
    set(ENABLE_WESTON_SHELL ON)
endif ()

find_package(Athol 0.1)
if (ATHOL_FOUND)
    set(ENABLE_ATHOL_SHELL ON)
endif ()

if (ENABLE_VIDEO)
    set(GSTREAMER_COMPONENTS app audio fft gl pbutils tag video)
    add_definitions(-DWTF_USE_GSTREAMER)

    find_package(GStreamer 1.4.1 REQUIRED COMPONENTS ${GSTREAMER_COMPONENTS})

    # FIXME: What about MPEGTS support? WTF_USE_GSTREAMER_MPEGTS?
endif ()

add_definitions(-DBUILDING_WPE__=1)
add_definitions(-DDATA_DIR="${CMAKE_INSTALL_DATADIR}")

set(WTF_USE_UDIS86 1)

add_definitions(-DWTF_USE_OPENGL=1)
add_definitions(-DWTF_USE_OPENGL_ES_2=1)

set(WTF_USE_3D_GRAPHICS 1)
add_definitions(-DWTF_USE_3D_GRAPHICS=1)
add_definitions(-DENABLE_3D_RENDERING=1)

set(ENABLE_TEXTURE_MAPPER 1)
add_definitions(-DWTF_USE_TEXTURE_MAPPER=1)
add_definitions(-DWTF_USE_TEXTURE_MAPPER_GL=1)

set(WTF_USE_EGL 1)
add_definitions(-DWTF_USE_EGL=1)
add_definitions(-DWTF_PLATFORM_WAYLAND=1)

set(FORWARDING_HEADERS_DIR ${DERIVED_SOURCES_DIR}/ForwardingHeaders)

# Build with -fvisibility=hidden to reduce the size of the shared library.
# Not to be used when building the WebKitTestRunner library.
if (NOT DEVELOPER_MODE)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fvisibility=hidden")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fvisibility=hidden")
endif ()
