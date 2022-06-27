
#ifndef ToLuau_API_H
#define ToLuau_API_H

#ifdef ToLuau_BUILT_AS_STATIC
#  define ToLuau_API
#  define TOLUAU_NO_EXPORT
#else
#  ifndef ToLuau_API
#    ifdef ToLuau_EXPORTS
        /* We are building this library */
#      define ToLuau_API __declspec(dllexport)
#    else
        /* We are using this library */
#      define ToLuau_API __declspec(dllimport)
#    endif
#  endif

#  ifndef TOLUAU_NO_EXPORT
#    define TOLUAU_NO_EXPORT 
#  endif
#endif

#ifndef TOLUAU_DEPRECATED
#  define TOLUAU_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef TOLUAU_DEPRECATED_EXPORT
#  define TOLUAU_DEPRECATED_EXPORT ToLuau_API TOLUAU_DEPRECATED
#endif

#ifndef TOLUAU_DEPRECATED_NO_EXPORT
#  define TOLUAU_DEPRECATED_NO_EXPORT TOLUAU_NO_EXPORT TOLUAU_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef TOLUAU_NO_DEPRECATED
#    define TOLUAU_NO_DEPRECATED
#  endif
#endif

#endif /* ToLuau_API_H */
