HttpDownloader
==============

Some advice on making HTTP requests from your C++ code and a simple wrapper around libcurl that includes these best practices.

Why?
====

You are probably here because you want to make HTTP requests from your C++ program and you are looking for some source code examples you can look at. You are in the right place - I wrote this because I was dissatisfied with the state of the popular examples on other sites.

This code is based on [libcurl](https://curl.haxx.se/libcurl/). libcurl is an amazing project - one of the best long-running open source products around. It is so fully-featured and easy to integrate that it is easy to forget you should be very careful when including any kind of web access.

Do you need it?
===============

Probably not. But you should at least take a look to see if there is anything you can steal. It isn't the simplest libcurl example available but it is some of the most robust and safe.

However...

If you are writing a client-side, user focused application you may be better of eschewing libcurl altogether. libcurl is great for cross platform code but lacks the advantages of tight integration with desktop and mobile OSes. 

In particular, libcurl does not know about the users HTTP proxy settings which makes it harder to use behind corporate and school proxies. I am not saying not to use libcurl on the client, just that you might be better off just using the local APIs.

You might also be already using Qt or something similar which will include perfectly good HTTP libraries.

Compiling
=========

HttpDownloader comes with a very simple example program to get you started. I've provided a simple CMakeList.txt file to build it but you probably don't need it.

````
mkdir build
cd build
cmake ..
make

````

Usage
=====

A typical usage might be something like this:

````
    HTTPDownloader downloader;
    std::stringstream contents;
    
    try
    {
        downloader.download(contents, "https://example.com/some/rest/endpoint", "application/json", 100 * 1024, 60);
    }
    catch (const HTTPDownloaderException& e)
    {
        // AARRGGHH! Something went wrong - tell the user or try again or something
    }
    
    JSON.parse(content.str());
````

See `main.cpp` for a more complete example. 

Rationale
=========

As written, this class is only designed to perform simple GETs in the safest way possible. Extending it would not be hard but I decided to cover the most common case comprehensively rather than provide a massive kitchen sink solution.

Exceptions
----------
This class throws exceptions. Maybe you don't like that but I think it is appropriate for what amounts to a network request to a server you don't control. Anything can go wrong. 

libcurl itself is NOT exception safe. HttpDownloader smuggles exceptions out of the libcurl callbacks and throws them safely. 

Http Errors
-----------
There is no way to get the HTTP status code of the response. An exception will be thrown on 400 errors. I think this is probably what most people want. If you need to get the status code then you will have to modify the class not to set the `CURLOPT_FAILONERROR` option.

Mimetype
--------
You can ignore the mimetype but you shouldn't. You may be expecting a nicely formatted json file but sooner or later the server will barf up an error or some middle box will give you something you don't expect. Best to detect this up front.

Maximum Size
------------
A common use case is to pass a `stringstream` into the `download()` method. This is very convenient but you don't want to stream in 2Gig of random junk if the server decides to go crazy. My advice is to set the maximum size to something reasonable.

Timeout
-------
Again, this is optional but you probably don't want to hang around for hours if the server decides it doesn't want to respond.






