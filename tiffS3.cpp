#include "tiffS3.h"
#include "context.h"

const char R_OK = 0;
const char R_FAIL = -1;

extern "C" {
    void* tiff_s3_init()
    {
        auto options = new Aws::SDKOptions;
        options->loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Debug;
        options->httpOptions.installSigPipeHandler = true;
        Aws::InitAPI(*options);
        return static_cast<void*>(options);
    }

    void tiff_s3_shutdown(void *options_in)
    {
        auto options = *static_cast<Aws::SDKOptions*>(options_in);
        Aws::ShutdownAPI(options);
    }

    thandle_t tiff_s3_open(const char * filename)
    {
        Context *ctx = Context::Open(filename);
        return static_cast<void*>(ctx);
    }
    tsize_t tiff_s3_read(thandle_t userdata, tdata_t buffer, tsize_t size)
    {
        if (userdata == NULL) return R_FAIL;
        return static_cast<Context*>(userdata)->Read(buffer, size);
    }
    tsize_t tiff_s3_write(thandle_t, tdata_t, tsize_t)
    { return 0; }
    toff_t tiff_s3_seek(thandle_t userdata, toff_t offset, int whence)
    {
        if (userdata == NULL) return R_FAIL;
        return static_cast<Context*>(userdata)->Seek(offset, whence);
    }
    toff_t tiff_s3_size(thandle_t userdata)
    {
        if (userdata == NULL) return R_FAIL;
        return static_cast<Context*>(userdata)->Size();
    }
    int tiff_s3_close(thandle_t userdata)
    {
        if (userdata == NULL) return R_FAIL;
        Context *ctx = static_cast<Context*>(userdata);
        delete ctx;
        return R_OK;
    }
}
