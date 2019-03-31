#include "HttpDownloader.hpp"

#include <iostream>

size_t HTTPDownloader::writeFunction(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    HTTPDownloader::TransferData* transfer = reinterpret_cast<HTTPDownloader::TransferData*>(userdata);
    try {
        size_t bytes = size * nmemb;
        if (bytes == 0) {
            // it is valid (highly unusual though) for the server to return 0 bytes
            return 0;
        }
        
        // although curl will check the content-length header for us, this will not be valid if the server
        // doesn't know the length of the file up-front. Even then it can lie. Double check here
        if (transfer->maxBytes != 0) {
            transfer->totalBytes += bytes;
            if (transfer->totalBytes > transfer->maxBytes) {
                throw HTTPDownloaderException("HTTPDownloader - server returned too many bytes");
            }
        }
        
        if (!(transfer->outputStream.write(ptr, bytes))) {
            throw HTTPDownloaderException("HTTPDownloader - error writing to stream");
        }
        
        return bytes;
    } catch (...) {
        // catch all exceptions (include those we haven't thrown ourselves - who knows what outputStream.write does?)
        // Store the exception and return an error. This is to prevent exceptions from being thrown through curl
        transfer->exception = std::current_exception();
        return 0;
    }
}


size_t HTTPDownloader::headerFunction(char *buffer, size_t size, size_t nitems, void *userdata)
{
    static const std::string CONTENT_TYPE_HEADER{"Content-Type: "};
    
    HTTPDownloader::TransferData* transfer = reinterpret_cast<HTTPDownloader::TransferData*>(userdata);
    try {
        size_t bytes = size * nitems;
        if (!transfer->mimeType.empty())
        {
            // examine the Content-Type header to make sure it matches what we expect
            std::string header(buffer, bytes);
            if ((bytes >= CONTENT_TYPE_HEADER.length()) && (header.compare(0, CONTENT_TYPE_HEADER.length(),CONTENT_TYPE_HEADER) == 0)) {
                std::string value{header.substr(CONTENT_TYPE_HEADER.length())};
                
                // trim the whitespace off the end of the value (\r\n)
                while (!value.empty() && isspace(value.back())) {
                    value.erase(value.length() - 1);
                }
                
                if (value != transfer->mimeType) {
                    throw HTTPDownloaderException("HTTPDownloader - server returned an unexpected mimetype of " + value + " instead of " + transfer->mimeType);
                }
            }
        }
        
        return bytes;
    } catch (...) {
        transfer->exception = std::current_exception();
        return 0;
    }
}

void HTTPDownloader::download(std::ostream& output, std::string uri, std::string mimetype, size_t maxSize, long timeoutSeconds)
{
    // The URL to GET - pretty important
    curl_easy_setopt(_curl.get(), CURLOPT_URL, uri.c_str());
    
    // The server might tell us that the resource has moved - follow the trail
    curl_easy_setopt(_curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    
    // This is important for multithreaded programs on unix-like systems
    curl_easy_setopt(_curl.get(), CURLOPT_NOSIGNAL, 1);
    
    // This ensures the we get an error back from curl if the server returns 404 or something similar
    curl_easy_setopt(_curl.get(), CURLOPT_FAILONERROR, 1);
    
    // We don't want to shove more data than we are expecting into the stream
    curl_easy_setopt(_curl.get(), CURLOPT_MAXFILESIZE_LARGE, maxSize);
    
    // We probably have some sense of how long this should take
    curl_easy_setopt(_curl.get(), CURLOPT_TIMEOUT, timeoutSeconds);
    
    // I've seen code that sets this - it is a BAD IDEA.
    // curl_easy_setopt(_curl.get(), CURLOPT_ACCEPT_ENCODING, "deflate");
    
    // Set up our custom handlers to deal with the data. TransferDara
    TransferData transferData{output, maxSize, 0, mimetype};
    curl_easy_setopt(_curl.get(), CURLOPT_WRITEFUNCTION, HTTPDownloader::writeFunction);
    curl_easy_setopt(_curl.get(), CURLOPT_WRITEDATA, &transferData);
    curl_easy_setopt(_curl.get(), CURLOPT_HEADERFUNCTION, HTTPDownloader::headerFunction);
    curl_easy_setopt(_curl.get(), CURLOPT_HEADERDATA, &transferData);
    
    CURLcode res = curl_easy_perform(_curl.get());
    if (res != CURLE_OK) {
        // something went wrong. Either throw the exception we already have, or a new one
        if (transferData.exception) {
            std::rethrow_exception(transferData.exception);
        } else {
            std::string msg = std::string("Download failed: ") + std::string(curl_easy_strerror(res));
            throw HTTPDownloaderException(msg);
        }
    }
}

