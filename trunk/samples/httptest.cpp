// ***************************************************************** -*- C++ -*-
/*
 httptest.cpp
 This application is to test http.cpp. It provides the function to GET|HEAD|PUT the file via http protocol.
 */

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <stdlib.h>

static int testSyntax(const char* arg)
{
	if ( !arg ) {
		std::cout << "insufficient input" << std::endl;
		exit(0);
	}
	return 0;
}

int main(int argc,const char** argv)
{
    if ( argc < 2 ) {
        std::cout << "usage  : " << argv[0] << " [key value]+" << std::endl;
        std::cout << "example: " << argv[0] << " [[-url] url | -server clanmills.com -page /LargsPanorama.jpg] -header \"Range: bytes=0-200\"" << std::endl;
		std::cout << "or     : " << argv[0] << " http://clanmills.com/LargsPanorama.jpg"                                     << std::endl;
        std::cout << "useful  keys: -verb {GET|HEAD|PUT}  -page str -server str -port number -version [-header something]+ " << std::endl;
        std::cout << "default keys: -verb GET -server clanmills.com -page robin.shtml -port 80 -version 1.0"                 << std::endl;
        std::cout << "export http_proxy=url eg export http_proxy=http://64.62.247.244:80"                                    << std::endl;
        return 0;
    }

    Exiv2::Dictionary response;
    Exiv2::Dictionary request;
    std::string       errors;

    // convert the command-line arguments into the request dictionary
    for ( int i = 1 ; i < argc ; i +=2 ) {
        const char* arg = argv[i];
        // skip past the -'s on the key
        while ( arg[0] == '-' ) arg++;

        if ( std::string(arg) == "header" ) {
			testSyntax(argv[i+1]);
            std::string header = argv[i+1];
            if ( ! strchr(argv[i+1],'\n') ) {
                header += "\r\n";
            }
            request[arg] += header;
        } else if ( std::string(arg) == "uri" || std::string(arg) == "url" ) {
			testSyntax(argv[i+1]);
        	Exiv2::Uri uri = Exiv2::Uri::Parse(argv[i+1]);
        	if ( uri.Protocol == "http" ) {
        	    request["server"] = uri.Host;
        	    request["page"]   = uri.Path;
        	    request["port"]   = uri.Port;
        	}
		} else if ( std::string(arg).substr(0,7) == "http://" ) {
        	Exiv2::Uri uri = Exiv2::Uri::Parse(argv[i--]);
        	if ( uri.Protocol == "http" ) {
        	    request["server"] = uri.Host;
        	    request["page"]   = uri.Path;
        	    request["port"]   = uri.Port;
        	}
        } else {
			testSyntax(argv[i+1]);
            request[arg]=argv[i+1];
        }
    }
    if ( !request.count("page"  ) ) request["page"  ]   = "robin.shtml";
    if ( !request.count("server") ) request["server"]   = "clanmills.com";

    int result = Exiv2::http(request,response,errors);
    std::cout << "result = " << result << std::endl;
    std::cout << "errors = " << errors << std::endl;
    std::cout << std::endl;

    for ( Exiv2::Dictionary_i it = response.begin() ; it != response.end() ; it++ ) {
        // don't show request header
        if (it->first ==  "requestheaders") continue;

        std::cout << it->first << " -> ";

        if ( it->first ==  "body") {
        	std::string value(it->second);
        	std::cout << "# " << value.length();
        	if ( value.length() < 1000 ) std::cout << " = " << value ;
        } else {
        	std::cout << it->second;
        }

        std::cout << std::endl;
    }

    return 0;
}

// That's all Folks!
////
