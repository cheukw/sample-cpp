#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

int main()
{ 
    LOG(INFO) << "My First info log using default logger";
    return 0;
}