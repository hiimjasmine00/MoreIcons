#import <CoreFoundation/CoreFoundation.h>
#include <fmt/format.h>
#include <Geode/Result.hpp>
#include <span>

using namespace geode;

Result<std::vector<uint8_t>> readBinaryPlist(std::span<const uint8_t> data) {
    CFDataRef cfData = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, data.data(), data.size(), kCFAllocatorNull);
    if (!cfData) return Err("Failed to create data");

    CFErrorRef error = nullptr;
    CFPropertyListRef plist = CFPropertyListCreateWithData(kCFAllocatorDefault, cfData, kCFPropertyListImmutable, nullptr, &error);
    CFRelease(cfData);

    if (!plist) {
        if (error) {
            std::string message = fmt::format("Failed to parse plist: {}",
                CFStringGetCStringPtr(CFErrorCopyDescription(error), kCFStringEncodingUTF8));
            CFRelease(error);
            return Err(std::move(message));
        }
        else return Err("Failed to parse plist");
    }

    CFDataRef outData = CFPropertyListCreateData(kCFAllocatorDefault, plist, kCFPropertyListXMLFormat_v1_0, 0, &error);
    CFRelease(plist);

    if (!outData) {
        if (error) {
            std::string message = fmt::format("Failed to convert to XML: {}",
                CFStringGetCStringPtr(CFErrorCopyDescription(error), kCFStringEncodingUTF8));
            CFRelease(error);
            return Err(std::move(message));
        }
        else return Err("Failed to convert to XML");
    }

    std::vector<uint8_t> result(CFDataGetLength(outData));
    ::memcpy(result.data(), CFDataGetBytePtr(outData), result.size());
    CFRelease(outData);
    return Ok(std::move(result));
}
