#pragma once
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <tiffio.h>
#include "cache.h"

class Context {
    public:
        Context():
        _request_offset(0) {}

        static Context* Open(std::string);
        tsize_t Read(tdata_t buffer, tsize_t size);
        toff_t Seek(toff_t offset, int whence);
        toff_t Size() { return _object_size; }

    private:
        std::unique_ptr<Aws::S3::S3Client> _s3_client;
        std::string _bucket;
        std::string _object_key;
        tsize_t _object_size;
        tsize_t _request_offset;
        toff_t _last_fetched_offset = std::numeric_limits<uint64_t>::max();
        tsize_t _num_ranges_to_fetch = 1;

        const tsize_t _range_size = 16384;
        const int _max_regions = 1000;
        using RangeCache = Cache<toff_t,std::shared_ptr<std::string>>;
        std::unique_ptr<RangeCache> _cache = 
            std::unique_ptr<RangeCache>(new RangeCache(_range_size * 1000));
        std::shared_ptr<std::string> GetCachedRange(toff_t);
        void PutCachedRange(toff_t, tsize_t, const char*);
        std::string FetchRange(toff_t range_offset, int num_ranges);
};
