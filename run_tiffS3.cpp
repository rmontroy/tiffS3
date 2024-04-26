#include "tiffS3.h"

#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    // Aws::SDKOptions options;
    //Turn on logging.
    // options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info;
    // Override the default log level for AWS common runtime libraries to see multipart upload entries in the log file.
    // options.loggingOptions.crt_logger_create_fn = []() {
    //     return Aws::MakeShared<Aws::Utils::Logging::DefaultCRTLogSystem>(ALLOCATION_TAG, Aws::Utils::Logging::LogLevel::Debug);
    // };

    // Uncomment the following code to override default global client bootstrap for AWS common runtime libraries.
    // options.ioOptions.clientBootstrap_create_fn = []() {
    //     Aws::Crt::Io::EventLoopGroup eventLoopGroup(0 /* cpuGroup */, 18 /* threadCount */);
    //     Aws::Crt::Io::DefaultHostResolver defaultHostResolver(eventLoopGroup, 8 /* maxHosts */, 300 /* maxTTL */);
    //     auto clientBootstrap = Aws::MakeShared<Aws::Crt::Io::ClientBootstrap>(ALLOCATION_TAG, eventLoopGroup, defaultHostResolver);
    //     clientBootstrap->EnableBlockingShutdown();
    //     return clientBootstrap;
    // };

    // Uncomment the following code to override default global TLS connection options for AWS common runtime libraries.
    // options.ioOptions.tlsConnectionOptions_create_fn = []() {
    //     Aws::Crt::Io::TlsContextOptions tlsCtxOptions = Aws::Crt::Io::TlsContextOptions::InitDefaultClient();
    //     tlsCtxOptions.OverrideDefaultTrustStore(<CaPathString>, <CaCertString>);
    //     Aws::Crt::Io::TlsContext tlsContext(tlsCtxOptions, Aws::Crt::Io::TlsMode::CLIENT);
    //     return Aws::MakeShared<Aws::Crt::Io::TlsConnectionOptions>(ALLOCATION_TAG, tlsContext.NewConnectionOptions());
    // };

    auto options = tiff_s3_init();
    {
        thandle_t userdata = tiff_s3_open(argv[1]);
        if (userdata == NULL) {
            std::cout << "tiff_s3_open error" << std::endl << std::endl;
            return 1;
        }
        std::unique_ptr<char[]> buffer(new char [128]);
        int bytes_read = tiff_s3_read(userdata, buffer.get(), 128);
        if (bytes_read > 0) {
            std::cout << "Read " << bytes_read << " bytes" << std::endl << std::endl;
        }
        else {
            std::cout << "tiff_s3_read error" << std::endl << std::endl;
        }
        tiff_s3_close(userdata);
    }
    tiff_s3_shutdown(options);

    return 0;
}