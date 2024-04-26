#include <iostream>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include "context.h"

std::shared_ptr<std::string> Context::GetCachedRange(toff_t offset)
{
    std::shared_ptr<std::string> out;
    return _cache->tryGet(offset, out) ?
        out :
        nullptr;
}
void Context::PutCachedRange(toff_t offset, tsize_t nSize, const char* pData)
{
    std::shared_ptr<std::string> value(new std::string());
    value->assign(pData, nSize);
    _cache->put(offset, value);
}

Context* Context::Open(std::string filename) {
    std::size_t split = filename.find('/', 5);
    auto ctx = new Context; // raw pointer b/c -- ffi
    ctx->_bucket = filename.substr(5, split-5);
    ctx->_object_key = filename.substr(split+1);

    Aws::Client::ClientConfiguration config;
    config.enableHttpClientTrace = true;
    ctx->_s3_client = std::unique_ptr<Aws::S3::S3Client>(new Aws::S3::S3Client(config));

    Aws::S3::Model::HeadObjectRequest request;
    request.SetBucket(ctx->_bucket);
    request.SetKey(ctx->_object_key);
    auto outcome = ctx->_s3_client->HeadObject(request);
    if (!outcome.IsSuccess()) {
        // If HEAD fails, try GET so we have an actual error message
        Aws::S3::Model::GetObjectRequest request;
        request.SetBucket(ctx->_bucket);
        request.SetKey(ctx->_object_key);
        request.SetRange("bytes=1-4");
        auto outcome = ctx->_s3_client->GetObject(request);
        std::cout << outcome.GetError() << std::endl;
        delete ctx;
        return NULL;
    }
    ctx->_object_size = outcome.GetResult().GetContentLength();
    return ctx;
}

tsize_t Context::Read(tdata_t request_buffer, tsize_t buffer_size)
{
    if (buffer_size == 0) return 0;

    void* working_buffer = request_buffer;
    tsize_t remaining_bytes = buffer_size;
    tsize_t iter_offset = _request_offset;
    while (remaining_bytes > 0)
    {
        if(iter_offset >= _object_size)
        {
            break;
        }

        // align fetch offset
        const toff_t fetch_offset = (iter_offset / _range_size) * _range_size;
        std::string range_data;
        std::shared_ptr<std::string> cached_range = GetCachedRange(fetch_offset);
        if (cached_range != nullptr)
        {
            range_data = *cached_range;
        }
        else
        {
            if( fetch_offset == _last_fetched_offset )
            {
                // In case of consecutive reads (of small size), we use a
                // heuristic that we will read the file sequentially, so
                // we double the requested size to decrease the number of
                // client/server roundtrips.
                if( _num_ranges_to_fetch < 100 )
                    _num_ranges_to_fetch *= 2;
            }
            else
            {
                // Random reads. Cancel the above heuristics.
                _num_ranges_to_fetch = 1;
            }

            // Ensure that we will request at least the number of blocks
            // to satisfy the remaining buffer size to read.
            const toff_t end_offset =
                ((iter_offset + remaining_bytes + _range_size - 1) / _range_size) *
                _range_size;
            const int min_ranges_to_fetch =
                static_cast<int>((end_offset - fetch_offset) / _range_size);
            if( _num_ranges_to_fetch < min_ranges_to_fetch )
                _num_ranges_to_fetch = min_ranges_to_fetch;
            
            // Avoid reading already cached data.
            for( int i = 1; i < _num_ranges_to_fetch; i++ )
            {
                if( GetCachedRange(
                        fetch_offset + i * _range_size) != nullptr )
                {
                    _num_ranges_to_fetch = i;
                    break;
                }
            }

            if( _num_ranges_to_fetch > _max_regions )
                _num_ranges_to_fetch = _max_regions;

            range_data = FetchRange(fetch_offset, _num_ranges_to_fetch);
            if(range_data.empty()) return 0;
        }

        toff_t region_offset = iter_offset - fetch_offset;
        if (range_data.size() < region_offset)
        {
            break;
        }

        const int bytes_to_copy = static_cast<int>(
            std::min(static_cast<toff_t>(remaining_bytes),
                     range_data.size() - region_offset));
        memcpy(working_buffer,
               range_data.data() + region_offset,
               bytes_to_copy);
        working_buffer = static_cast<char *>(working_buffer) + bytes_to_copy;
        iter_offset += bytes_to_copy;
        remaining_bytes -= bytes_to_copy;
        if( range_data.size() < static_cast<size_t>(_range_size) &&
            remaining_bytes != 0 )
        {
            break;
        }
    }

    tsize_t bytes_read = iter_offset - _request_offset;
    _request_offset = iter_offset;
    return bytes_read;
}

std::string Context::FetchRange(toff_t start_offset, int num_ranges) {
    Aws::S3::Model::GetObjectRequest request;
    request.SetBucket(_bucket);
    request.SetKey(_object_key);
    toff_t end_offset = start_offset + num_ranges * _range_size - 1;
    if(end_offset >= _object_size)
    {
        end_offset = _object_size - 1;
    }
    std::string range = "bytes=" + std::to_string(start_offset) + "-" + std::to_string(end_offset);
    request.SetRange(range);
    auto outcome = _s3_client->GetObject(request);
    if (!outcome.IsSuccess()) {
        std::cout << outcome.GetError() << std::endl;
        return 0;
    }
    std::stringstream ss;
    ss << outcome.GetResult().GetBody().rdbuf();
    tsize_t fetch_size = outcome.GetResult().GetContentLength();

    _last_fetched_offset = start_offset + num_ranges * _range_size;
    toff_t iter_offset = start_offset;
    std::string range_data = ss.str();
    const char* working_buffer = range_data.c_str();
    while( fetch_size > 0 )
    {
        const size_t range_size = std::min(_range_size, fetch_size);
        PutCachedRange(iter_offset, range_size, working_buffer);
        iter_offset += range_size;
        working_buffer += range_size;
        fetch_size -= range_size;
    }
    return range_data;
}

toff_t Context::Seek(toff_t offset, int whence) {
    switch (whence) {
        case SEEK_SET: {
            _request_offset = offset;
            break;
        }
        case SEEK_CUR: {
            _request_offset += offset;
            break;
        }
        case SEEK_END: {
            _request_offset = _object_size + offset;
            break;
        }
        default:
            return -1;
    }
    return _request_offset;
}
