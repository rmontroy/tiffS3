#pragma once
#include <tiffio.h>

#if __GNUC__ >= 4 && defined(TIFFS3_EXPORTS)
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
    #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
    #define DLL_PUBLIC
    #define DLL_LOCAL
#endif
  
struct Context;

#ifdef __cplusplus
extern "C"
{
#endif
    DLL_PUBLIC void* tiff_s3_init(void);
    DLL_PUBLIC void tiff_s3_shutdown(void*);
    DLL_PUBLIC thandle_t tiff_s3_open(const char*);
    DLL_PUBLIC tsize_t tiff_s3_read(thandle_t, tdata_t, tsize_t);
    DLL_PUBLIC toff_t tiff_s3_seek(thandle_t, toff_t, int);
    DLL_PUBLIC int tiff_s3_close(thandle_t);
    DLL_PUBLIC toff_t tiff_s3_size(thandle_t);
#ifdef __cplusplus
}
#endif