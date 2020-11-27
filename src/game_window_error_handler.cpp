#include <game_window_error_handler.h>

bool GameWindowErrorHandler::onError(std::string title, std::string errormsg) {
    printf("[%s]: %s\n", title.c_str(), errormsg.c_str());
    return true;
}