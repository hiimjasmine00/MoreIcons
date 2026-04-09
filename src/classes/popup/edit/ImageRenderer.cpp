#include "ImageRenderer.hpp"
#include "../../../utils/Get.hpp"
#include <Geode/utils/file.hpp>

using namespace geode::prelude;

texpack::Image ImageRenderer::getImage(CCNode* node) {
    auto size = node->getScaledContentSize() * Get::director->getContentScaleFactor();
    auto floatWidth = size.width;
    auto floatHeight = size.height;
    uint32_t width = floatWidth;
    uint32_t height = floatHeight;

    auto texture = 0u;
    glPixelStorei(GL_PACK_ALIGNMENT, 8);
    glGenTextures(1, &texture);
    ccGLBindTexture2D(texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    auto oldFBO = 0;
    auto fbo = 0u;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    kmGLMatrixMode(KM_GL_PROJECTION);
    kmGLPushMatrix();
    kmGLMatrixMode(KM_GL_MODELVIEW);
    kmGLPushMatrix();

    glViewport(0, 0, width, height);

    auto winSize = Get::director->getWinSizeInPixels();

    kmMat4 orthoMatrix;
    kmMat4OrthographicProjection(&orthoMatrix, -floatWidth / winSize.width, floatWidth / winSize.width,
        -floatHeight / winSize.height, floatHeight / winSize.height, -1.0f, 1.0f);
    kmGLMultMatrix(&orthoMatrix);

    std::array clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor.data());
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

    node->visit();

    std::vector<uint8_t> data(width * height * 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
    Get::director->setViewport();
    kmGLMatrixMode(KM_GL_PROJECTION);
    kmGLPopMatrix();
    kmGLMatrixMode(KM_GL_MODELVIEW);
    kmGLPopMatrix();
    ccGLDeleteTexture(texture);
    glDeleteFramebuffers(1, &fbo);

    for (uint32_t y = 0; y < height / 2; y++) {
        std::swap_ranges(data.begin() + y * width * 4, data.begin() + (y + 1) * width * 4, data.end() - (y + 1) * width * 4);
    }

    return { std::move(data), width, height };
}

Result<std::vector<uint8_t>> ImageRenderer::getImage(CCTexture2D* texture) {
    auto width = texture->getPixelsWide();
    auto height = texture->getPixelsHigh();

    std::vector<uint8_t> data(width * height * 4);

    auto oldFBO = 0;
    auto fbo = 0u;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->getName(), 0);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
    glDeleteFramebuffers(1, &fbo);

    return texpack::toPNG(data, width, height);
}

texpack::Image ImageRenderer::getImage(CCSpriteFrame* frame) {
    auto texture = frame->getTexture();

    auto& frameRect = frame->m_obRectInPixels;
    uint32_t w = frameRect.size.width;
    uint32_t h = frameRect.size.height;

    auto rotated = frame->m_bRotated;

    std::vector<uint8_t> frameData(w * h * 4);

    auto oldFBO = 0;
    auto fbo = 0u;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->getName(), 0);
    glReadPixels(frameRect.origin.x, frameRect.origin.y, rotated ? h : w, rotated ? w : h, GL_RGBA, GL_UNSIGNED_BYTE, frameData.data());
    glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
    glDeleteFramebuffers(1, &fbo);

    auto& frameSize = frame->m_obOriginalSizeInPixels;
    uint32_t width = frameSize.width;
    uint32_t height = frameSize.height;

    if (width == w && height == h) {
        return { std::move(frameData), width, height };
    }

    auto& frameOffset = frame->m_obOffsetInPixels;
    int offsetX = frameOffset.x;
    int offsetY = frameOffset.y;

    std::vector<uint8_t> data(width * height * 4);
    auto top = height - h;
    if (top % 2 != 0) top++;
    top /= 2;
    top -= offsetY;
    auto left = width - w;
    if (left % 2 != 0) left++;
    left /= 2;
    left += offsetX;

    if (rotated) {
        for (uint32_t y = 0; y < w; y++) {
            for (uint32_t x = 0; x < h; x++) {
                auto src = (y * h + x) * 4;
                auto dst = ((top + h - x - 1) * width + left + y) * 4;
                data[dst] = frameData[src];
                data[dst + 1] = frameData[src + 1];
                data[dst + 2] = frameData[src + 2];
                data[dst + 3] = frameData[src + 3];
            }
        }
    }
    else {
        for (uint32_t y = 0; y < h; y++) {
            for (uint32_t x = 0; x < w; x++) {
                auto src = (y * w + x) * 4;
                auto dst = ((top + y) * width + left + x) * 4;
                data[dst] = frameData[src];
                data[dst + 1] = frameData[src + 1];
                data[dst + 2] = frameData[src + 2];
                data[dst + 3] = frameData[src + 3];
            }
        }
    }

    return { std::move(data), width, height };
}

Result<> ImageRenderer::save(texpack::Packer& packer, const std::filesystem::path& png, const std::filesystem::path& plist, std::string_view name) {
    GEODE_UNWRAP(packer.pack().mapErr([](std::string err) {
        return fmt::format("Failed to pack image: {}", err);
    }));
    GEODE_UNWRAP_INTO(auto pngData, packer.png().mapErr([](std::string err) {
        return fmt::format("Failed to encode image: {}", err);
    }));
    auto plistData = packer.plist(name, "    ");
    GEODE_UNWRAP(file::writeBinary(png, pngData).mapErr([](std::string err) {
        return fmt::format("Failed to save image: {}", err);
    }));
    return file::writeString(plist, plistData).mapErr([](std::string err) {
        return fmt::format("Failed to save plist: {}", err);
    });
}
