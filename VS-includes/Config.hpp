
#define WINDOWS

//#define STEAMWORKS

#define HAVE_FMOD

#define EDITOR_EXE_NAME "editor"

#define BASE_DATA_DIR "./"

#define USE_FMOD

//#define DEBUG_ACHIEVEMENTS

#ifdef BARONY_DRM_FREE

#ifdef STEAMWORKS
#undef STEAMWORKS
#endif // STEAMWORKS

#endif
