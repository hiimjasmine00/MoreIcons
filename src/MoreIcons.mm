#include "MoreIcons.hpp"
#include <CoreGraphics/CoreGraphics.h>
#ifdef GEODE_IS_MACOS
#define CommentType CommentTypeDummy
#include <CoreServices/CoreServices.h>
#undef CommentType
#include <ImageIO/ImageIO.h>
#else
#include <UIKit/UIKit.h>
#endif

using namespace geode::prelude;

bool MoreIcons::imageToFile(CCImage* image, const std::filesystem::path& path) {
    auto width = image->getWidth();
    auto height = image->getHeight();
    auto provider = CGDataProviderCreateWithData(nullptr, image->getData(), width * height * 4, nullptr);
    auto colorSpace = CGColorSpaceCreateDeviceRGB();
    auto cgImage = CGImageCreate(width, height, 8, 32, width * 4, colorSpace,
        kCGImageAlphaPremultipliedLast, provider, nullptr, false, kCGRenderingIntentDefault);
    #ifdef GEODE_IS_IOS
    auto uiImage = [UIImage imageWithCGImage:cgImage];
    auto pngData = UIImagePNGRepresentation(uiImage);
    if (pngData) [pngData writeToFile:[NSString stringWithUTF8String:path.c_str()] atomically:YES];
    auto ret = pngData != nil;
    #else
    // THANK YOU NINNYYYY :DDDDD https://github.com/ninXout/NEW-PRNTSCRN/blob/a179507761e7d0b6dcded15841d21d33bfba4c0d/src/Screenshot.mm#L14
    auto file = CFStringCreateWithCString(kCFAllocatorDefault, path.c_str(), kCFStringEncodingMacRoman);
    auto url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, file, kCFURLPOSIXPathStyle, false);
    auto ret = false;
    if (auto dest = CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, nullptr)) {
        CGImageDestinationAddImage(dest, cgImage, nullptr);
        ret = CGImageDestinationFinalize(dest);
        CFRelease(dest);
    }
    CFRelease(url);
    CFRelease(file);
    #endif
    CGImageRelease(cgImage);
    CGColorSpaceRelease(colorSpace);
    CGDataProviderRelease(provider);
    return ret;
}
