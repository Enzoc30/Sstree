//
// Created by enzoc on 25/10/23.
//

#ifndef EDA_CORTEXAPI_H
#define EDA_CORTEXAPI_H


#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <curl/curl.h>

#include "params.h"

class CortexAPI {
private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

public:
    CortexAPI();
    ~CortexAPI();

    std::vector<NType> postImage(const std::string& imagePath);
};


#endif //EDA_CORTEXAPI_H
