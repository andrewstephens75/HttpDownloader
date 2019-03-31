
// HTTPDownloader
//
// A simple libcurl wrapper that properly handles many cases that the
// various curl example programs do not. This only covers the case where 
// you want to download some data from a URI. See the main.cpp for a simple
// example
//
// License is explicitly given to use or adapt this code in any way you see fit
// within the bounds of appliciable law.
//
// Author: Andrew Stephens (https://sheep.horse/)
// 

#ifndef HttpDownloader_hpp
#define HttpDownloader_hpp

#include <iosfwd>
#include <sstream>

#include <curl/curl.h>

// I like exceptions
class HTTPDownloaderException : public std::runtime_error
{
public:
    HTTPDownloaderException(const std::string& what):std::runtime_error(what) {}
};

class HTTPDownloader
{
public:
    HTTPDownloader():_curl(curl_easy_init(), &curl_easy_cleanup){}
    
    // Start a synchronous(blocking) download, writing the data to the given stream.
    // output : any valid ostream. This is not closed on return.
    // uri : string containing the full URI of the request, eg: https://example.com/somepage.html
    // mimetype : expected mimetype, leave blank for anything (not recomended)
    // maxSize : expected maxiumum size. The download will fail as soon as this many bytes have been returned
    // timeoutSeconds : total duration after which the download will fail.
    void download(std::ostream& output, std::string uri, std::string mimetype = "", size_t maxSize = 0, long timeoutSeconds = 0);
private:
    using CurlDeleter = std::function<void(CURL*)>;
    std::unique_ptr<CURL, CurlDeleter> _curl;
    
    // Structure that contains information about the transfer as it is in progress
    struct TransferData {
        std::ostream& outputStream;
        size_t maxBytes;
        size_t totalBytes;
        std::string mimeType;
        std::exception_ptr exception;
    };
    
    static size_t writeFunction(char *ptr, size_t size, size_t nmemb, void *userdata);
    static size_t headerFunction(char *buffer, size_t size, size_t nitems, void *userdata);
};
#endif /* HttpDownloader_hpp */
