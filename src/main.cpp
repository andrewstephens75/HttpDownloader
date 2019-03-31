// A very simple example of how HTTPDownloader is supposed to be used

#include <iostream>
#include <sstream>

#include "HttpDownloader.hpp"

int main(int argc, const char * argv[]) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    HTTPDownloader downloader;
    std::stringstream contents;
    
    if (argc < 2) {
        std::cout << "Pass the URI on the command line" << std::endl;
        return 1;
    }
    
    try
    {
        downloader.download(contents, std::string{argv[1]}, "text/html", 300000);
    }
    catch (const HTTPDownloaderException& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }
    
    std::cout << contents.str() << std::endl;
    
    curl_global_cleanup();
    
    return 0;
}
