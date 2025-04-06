#include <Foundation/Foundation.h>

const char* iosResourcePath(const char* path) {
    auto resourcePath = [[NSBundle mainBundle] resourcePath];
    return resourcePath ? [[resourcePath stringByAppendingPathComponent:[NSString stringWithUTF8String:path]] UTF8String] : path;
}
