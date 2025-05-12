#include "MoreIcons.hpp"
#include <CoreGraphics/CoreGraphics.h>
#include <Geode/loader/Log.hpp>
#ifdef GEODE_IS_MACOS
#define CommentType CommentTypeDummy
#include <CoreServices/CoreServices.h>
#undef CommentType
#include <ImageIO/ImageIO.h>
#else
#include <UIKit/UIKit.h>
#endif

using namespace geode::prelude;

bool MoreIcons::saveToFile(const std::filesystem::path& path, void* data, int width, int height) {
    auto provider = CGDataProviderCreateWithData(nullptr, data, width * height * 4, nullptr);
    auto colorSpace = CGColorSpaceCreateDeviceRGB();
    auto cgImage = CGImageCreate(width, height, 8, 32, width * 4, colorSpace,
        kCGImageAlphaPremultipliedLast, provider, nullptr, false, kCGRenderingIntentDefault);
    auto ret = false;
    #ifdef GEODE_IS_IOS
    if (auto uiImage = [UIImage imageWithCGImage:cgImage]) {
        if (auto pngData = UIImagePNGRepresentation(uiImage)) ret = [pngData writeToFile:[NSString stringWithUTF8String:path.c_str()] atomically:true];
    }
    #else
    // THANK YOU NINNYYYY :DDDDD https://github.com/ninXout/NEW-PRNTSCRN/blob/a179507761e7d0b6dcded15841d21d33bfba4c0d/src/Screenshot.mm#L14
    if (auto file = CFStringCreateWithCString(kCFAllocatorDefault, path.c_str(), kCFStringEncodingUTF8)) {
        if (auto url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, file, kCFURLPOSIXPathStyle, false)) {
            if (auto dest = CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, nullptr)) {
                CGImageDestinationAddImage(dest, cgImage, nullptr);
                ret = CGImageDestinationFinalize(dest);
                CFRelease(dest);
            }
            CFRelease(url);
        }
        CFRelease(file);
    }
    #endif
    CGImageRelease(cgImage);
    CGColorSpaceRelease(colorSpace);
    CGDataProviderRelease(provider);
    return ret;
}
