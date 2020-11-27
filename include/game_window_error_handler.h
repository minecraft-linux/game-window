#pragma once
#include <string>

class GameWindowErrorHandler {

public:
    virtual bool onError(std::string title, std::string errormsg);
};