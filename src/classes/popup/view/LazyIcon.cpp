#include "LazyIcon.hpp"
#include "../EditIconPopup.hpp"
#include "../../misc/ThreadPool.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/CCAnimateFrameCache.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SpriteDescription.hpp>
#include <Geode/utils/file.hpp>
#include <texpack.hpp>

using namespace geode::prelude;

LazyIcon* LazyIcon::create(IconType type, int id, IconInfo* info) {
    auto ret = new LazyIcon();
    if (ret->init(type, id, info)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LazyIcon::init(IconType type, int id, IconInfo* info) {
    auto normalImage = CCNode::create();
    normalImage->setContentSize(CCSpriteFrameCache::get()->spriteFrameByName("playerSquare_001.png")->getOriginalSize());
    if (!CCMenuItemSpriteExtra::init(normalImage, nullptr, nullptr, nullptr)) return false;

    m_type = type;
    m_id = id;
    m_info = info;

    if (type == IconType::Special && !info) {
        m_visited = true;
        auto playerSpecial = CCSprite::createWithSpriteFrameName(fmt::format("player_special_{:02}_001.png", id).c_str());
        playerSpecial->setPosition(normalImage->getContentSize() / 2.0f);
        playerSpecial->setID("player-special");
        normalImage->addChild(playerSpecial);
        return true;
    }

    if (type <= IconType::Jetpack || (type == IconType::Special && info)) {
        auto textureName = getTextureNames().first;
        if (CCTextureCache::get()->textureForKey(textureName.c_str())) {
            m_visited = true;
            createIcon("", "", {});
        }
        else {
            setEnabled(false);
            m_loadingSprite = CCSprite::create("loadingCircle.png");
            m_loadingSprite->setPosition(normalImage->getContentSize() / 2.0f);
            m_loadingSprite->setScale(0.4f);
            m_loadingSprite->setBlendFunc({ GL_ONE, GL_ONE });
            m_loadingSprite->runAction(CCRepeatForever::create(CCRotateBy::create(1.0f, 360.0f)));
            normalImage->addChild(m_loadingSprite);
        }
    }

    return true;
}

std::pair<std::string, std::string> LazyIcon::getTextureNames() {
    if (m_info) return { m_info->folderName.empty() ? m_info->textures[0] : m_info->folderName, m_info->sheetName };
    std::string fullName = GameManager::get()->sheetNameForIcon(m_id, (int)m_type);
    auto fileUtils = CCFileUtils::get();
    return {
        fileUtils->fullPathForFilename(fmt::format("{}.png", fullName).c_str(), false),
        fileUtils->fullPathForFilename(fmt::format("{}.plist", fullName).c_str(), false)
    };
}

std::string LazyIcon::getIconName() {
    return m_info ? m_info->name : fmt::format("{}{:02}", MoreIcons::prefixes[(int)m_type], m_id);
}

void LazyIcon::createSimpleIcon() {
    auto iconName = fmt::format("{}{}", m_info ? GEODE_MOD_ID "/" : "", getIconName());
    auto spriteFrameCache = CCSpriteFrameCache::get();

    auto primaryFrame = spriteFrameCache->spriteFrameByName(fmt::format("{}_001.png", iconName).c_str());
    if (primaryFrame && primaryFrame->getTag() == 105871529) primaryFrame = nullptr;

    auto secondaryFrame = spriteFrameCache->spriteFrameByName(fmt::format("{}_2_001.png", iconName).c_str());
    if (secondaryFrame && secondaryFrame->getTag() == 105871529) secondaryFrame = nullptr;

    auto tertiaryFrame = m_type == IconType::Ufo ? spriteFrameCache->spriteFrameByName(fmt::format("{}_3_001.png", iconName).c_str()) : nullptr;
    if (tertiaryFrame && tertiaryFrame->getTag() == 105871529) tertiaryFrame = nullptr;

    auto glowFrame = spriteFrameCache->spriteFrameByName(fmt::format("{}_glow_001.png", iconName).c_str());
    if (glowFrame && glowFrame->getTag() == 105871529) glowFrame = nullptr;

    auto extraFrame = spriteFrameCache->spriteFrameByName(fmt::format("{}_extra_001.png", iconName).c_str());
    if (extraFrame && extraFrame->getTag() == 105871529) extraFrame = nullptr;

    auto normalImage = getNormalImage();
    auto center = normalImage->getContentSize() / 2.0f - CCPoint { 0.0f, (m_type == IconType::Ufo) * 7.0f };

    if (primaryFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(primaryFrame);
        sprite->setPosition(center);
        sprite->setID("primary-sprite");
        normalImage->addChild(sprite, 0);
    }

    if (secondaryFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(secondaryFrame);
        sprite->setPosition(center);
        sprite->setID("secondary-sprite");
        normalImage->addChild(sprite, -1);
    }

    if (tertiaryFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(tertiaryFrame);
        sprite->setPosition(center);
        sprite->setID("tertiary-sprite");
        normalImage->addChild(sprite, -2);
    }

    if (glowFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(glowFrame);
        sprite->setPosition(center);
        sprite->setID("glow-sprite");
        normalImage->addChild(sprite, -3);
    }

    if (extraFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(extraFrame);
        sprite->setPosition(center);
        sprite->setID("extra-sprite");
        normalImage->addChild(sprite, 1);
    }
}

void LazyIcon::createComplexIcon() {
    auto spider = m_type == IconType::Spider;
    auto afc = CCAnimateFrameCache::get();
    auto type = spider ? "Spider" : "Robot";
    auto idleFrames = afc->spriteFrameByName(fmt::format("{}_idle_001.png", type).c_str());
    auto iconName = fmt::format("{}{}", m_info ? GEODE_MOD_ID "/" : "", getIconName());
    auto sfc = CCSpriteFrameCache::get();
    auto normalImage = getNormalImage();
    auto center = normalImage->getContentSize() / 2.0f;

    auto glowNode = CCNode::create();
    glowNode->setPosition(center);
    glowNode->setAnchorPoint({ 0.5f, 0.5f });
    glowNode->setID("glow-node");

    auto usedTextures = static_cast<CCDictionary*>(afc->addSpriteFramesWithFile(
        fmt::format("{}_AnimDesc.plist", type).c_str())->objectForKey("usedTextures"));
    for (int i = 0; i < usedTextures->count(); i++) {
        auto usedTexture = static_cast<CCDictionary*>(usedTextures->objectForKey(fmt::format("texture_{}", i)));
        if (!usedTexture) continue;

        auto textureString = usedTexture->valueForKey("texture");
        auto texture = textureString ? textureString->m_sString : "";
        auto index = texture.size() >= spider + 11 ? numFromString<int>(std::string(texture, spider + 9, 2)).unwrapOr(0) : 0;

        if (index <= 0) continue;

        auto primaryFrame = sfc->spriteFrameByName(fmt::format("{}_{:02}_001.png", iconName, index).c_str());
        if (primaryFrame && primaryFrame->getTag() == 105871529) primaryFrame = nullptr;

        auto secondaryFrame = sfc->spriteFrameByName(fmt::format("{}_{:02}_2_001.png", iconName, index).c_str());
        if (secondaryFrame && secondaryFrame->getTag() == 105871529) secondaryFrame = nullptr;

        auto glowFrame = sfc->spriteFrameByName(fmt::format("{}_{:02}_glow_001.png", iconName, index).c_str());
        if (glowFrame && glowFrame->getTag() == 105871529) glowFrame = nullptr;

        auto extraFrame = index == 1 ? sfc->spriteFrameByName(fmt::format("{}_{:02}_extra_001.png", iconName, index).c_str()) : nullptr;
        if (extraFrame && extraFrame->getTag() == 105871529) extraFrame = nullptr;

        auto customIDString = usedTexture->valueForKey("customID");
        auto customID = customIDString ? customIDString->m_sString : "";
        uint8_t spriteColor = customID == "back01" || customID == "back02" || customID == "back03" ? 178 - (spider * 51) : 255;
        ccColor3B spriteColor3B = { spriteColor, spriteColor, spriteColor };

        auto description = static_cast<SpriteDescription*>(idleFrames->objectAtIndex(i));

        auto partNode = CCNode::create();
        partNode->setPosition(description->m_position + center);
        partNode->setRotation(description->m_rotation);
        partNode->setScaleX(description->m_scale.x);
        partNode->setScaleY(description->m_scale.y);
        partNode->setAnchorPoint({ 0.5f, 0.5f });
        partNode->setID(fmt::format("part-node-{}", i + 1));

        if (primaryFrame) {
            auto sprite = CCSprite::createWithSpriteFrame(primaryFrame);
            sprite->setFlipX(description->m_flipped.x != 0.0f);
            sprite->setFlipY(description->m_flipped.y != 0.0f);
            sprite->setColor(spriteColor3B);
            sprite->setID(fmt::format("primary-sprite-{}", i + 1));
            partNode->addChild(sprite, 0);
        }

        if (secondaryFrame) {
            auto sprite = CCSprite::createWithSpriteFrame(secondaryFrame);
            sprite->setFlipX(description->m_flipped.x != 0.0f);
            sprite->setFlipY(description->m_flipped.y != 0.0f);
            sprite->setColor(spriteColor3B);
            sprite->setID(fmt::format("secondary-sprite-{}", i + 1));
            partNode->addChild(sprite, -1);
        }

        if (glowFrame) {
            auto sprite = CCSprite::createWithSpriteFrame(glowFrame);
            sprite->setPosition(description->m_position);
            sprite->setRotation(description->m_rotation);
            sprite->setScaleX(description->m_scale.x);
            sprite->setScaleY(description->m_scale.y);
            sprite->setFlipX(description->m_flipped.x != 0.0f);
            sprite->setFlipY(description->m_flipped.y != 0.0f);
            sprite->setID(fmt::format("glow-sprite-{}", i + 1));
            glowNode->addChild(sprite, -1);
        }

        if (extraFrame) {
            auto sprite = CCSprite::createWithSpriteFrame(extraFrame);
            sprite->setFlipX(description->m_flipped.x != 0.0f);
            sprite->setFlipY(description->m_flipped.y != 0.0f);
            sprite->setID(fmt::format("extra-sprite-{}", i + 1));
            partNode->addChild(sprite, 1);
        }

        normalImage->addChild(partNode, description->m_zValue);
    }

    normalImage->addChild(glowNode, -1);
}

void LazyIcon::createIcon(const std::string& err, const std::string& texture, const std::vector<std::string>& frames) {
    if (err.empty()) {
        setEnabled(true);
        m_texture = texture;
        m_frames = frames;
        if (m_loadingSprite) {
            m_loadingSprite->removeFromParent();
            m_loadingSprite = nullptr;
        }
        if (m_type == IconType::Robot || m_type == IconType::Spider) createComplexIcon();
        else if (m_type <= IconType::Jetpack) createSimpleIcon();
        else if (m_info && m_type == IconType::Special) {
            auto normalImage = getNormalImage();

            auto square = CCSprite::createWithSpriteFrameName("playerSquare_001.png");
            square->setPosition(normalImage->getContentSize() / 2.0f);
            square->setColor({ 150, 150, 150 });
            square->setID("player-square");
            normalImage->addChild(square, 0);

            auto streak = CCSprite::create(m_info->textures[0].c_str());
            limitNodeHeight(streak, 27.0f, 999.0f, 0.001f);
            streak->setRotation(-90.0f);
            streak->setPosition(normalImage->getContentSize() / 2);
            streak->setID("player-streak");
            normalImage->addChild(streak, 1);
        }
    }
    else {
        log::error("{}: {}", getIconName(), err);
        m_loadingSprite->stopAllActions();
        m_loadingSprite->setDisplayFrame(CCSpriteFrameCache::get()->spriteFrameByName("GJ_deleteIcon_001.png"));
        m_loadingSprite->setScale(1.1f);
        m_loadingSprite->setRotation(0.0f);
    }
}

void LazyIcon::activate() {
    if (!m_bEnabled) return;
    CCMenuItemSpriteExtra::activate();
    EditIconPopup::create(m_type, m_id, m_info ? m_info->name : "", true)->show();
}

typedef std::function<void(std::string, std::string, std::vector<std::string>)> Callback;

void loadSheet(const std::string& png, const std::string& plist, const std::string& name, IconType type, Callback callback) {
    ThreadPool::get().pushTask([png, plist, name, type, callback = std::move(callback)] {
        auto data = MoreIconsAPI::getFileData(png);
        if (data.empty()) return queueInMainThread([callback = std::move(callback)] {
            callback("Failed to read image", "", {});
        });

        auto image = texpack::fromPNG(data.data(), data.size());
        if (image.isErr()) return queueInMainThread([err = image.unwrapErr(), callback = std::move(callback)] {
            callback(err, "", {});
        });

        CCDictionary* frames = nullptr;
        if (!plist.empty()) {
            auto sheet = MoreIconsAPI::createDictionary(plist, true);
            if (!sheet) return queueInMainThread([callback = std::move(callback)] {
                callback("Failed to load sheet", "", {});
            });

            auto originalFrames = static_cast<CCDictionary*>(sheet->objectForKey("frames"));
            if (!originalFrames) return sheet->release(), queueInMainThread([callback = std::move(callback)] {
                callback("Failed to load frames", "", {});
            });

            auto metadata = static_cast<CCDictionary*>(sheet->objectForKey("metadata"));
            auto formatStr = metadata ? metadata->valueForKey("format") : nullptr;
            auto format = formatStr ? numFromString<int>(formatStr->m_sString).unwrapOr(0) : 0;

            frames = new CCDictionary();
            for (auto [frame, dict] : CCDictionaryExt<std::string, CCDictionary*>(originalFrames)) {
                if (auto spriteFrame = MoreIconsAPI::createSpriteFrame(dict, nullptr, format)) {
                    frames->setObject(spriteFrame, name.empty() ? frame : MoreIconsAPI::getFrameName(frame, name, type));
                    spriteFrame->release();
                }
            }

            sheet->release();
        }

        queueInMainThread([png, image = image.unwrap(), frames = Ref(frames), callback = std::move(callback)] {
            auto texture = new CCTexture2D();
            texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
                (float)image.width,
                (float)image.height
            });
            CCTextureCache::get()->m_pTextures->setObject(texture, png);
            texture->release();

            std::vector<std::string> frameNames;
            if (frames) {
                auto spriteFrameCache = CCSpriteFrameCache::get();
                for (auto [frameName, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(frames)) {
                    frame->setTexture(texture);
                    spriteFrameCache->addSpriteFrame(frame, frameName.c_str());
                    frameNames.push_back(frameName);
                }
            }

            callback("", png, frameNames);
        });

        CC_SAFE_RELEASE(frames);
    });
}

void loadImages(IconInfo* info, Callback callback) {
    ThreadPool::get().pushTask([info, callback = std::move(callback)] {
        texpack::Packer packer;
        for (int i = 0; i < info->textures.size(); i++) {
            auto& frameName = info->frameNames[i];
            auto res = packer.frame(frameName, info->textures[i]).mapErr([&frameName](const std::string& err) {
                return fmt::format("{}: {}", frameName, err);
            });
            if (res.isErr()) return queueInMainThread([err = res.unwrapErr(), callback = std::move(callback)] {
                callback(err, "", {});
            });
        }

        auto packRes = packer.pack();
        if (packRes.isErr()) return queueInMainThread([err = packRes.unwrapErr(), callback = std::move(callback)] {
            callback(err, "", {});
        });

        auto frames = new CCDictionary();
        for (auto& frame : packer.frames()) {
            auto spriteFrame = new CCSpriteFrame();
            spriteFrame->initWithTexture(
                nullptr,
                { (float)frame.rect.origin.x, (float)frame.rect.origin.y, (float)frame.rect.size.width, (float)frame.rect.size.height },
                frame.rotated,
                { (float)frame.offset.x, (float)frame.offset.y },
                { (float)frame.size.width, (float)frame.size.height }
            );
            frames->setObject(spriteFrame, frame.name);
            spriteFrame->release();
        }

        queueInMainThread([info, image = packer.image(), frames = Ref(frames), callback = std::move(callback)] {
            auto texture = new CCTexture2D();
            texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
                (float)image.width,
                (float)image.height
            });
            CCTextureCache::get()->m_pTextures->setObject(texture, info->folderName);
            texture->release();

            std::vector<std::string> frameNames;
            auto spriteFrameCache = CCSpriteFrameCache::get();
            for (auto [frameName, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(frames)) {
                frame->setTexture(texture);
                spriteFrameCache->addSpriteFrame(frame, frameName.c_str());
                frameNames.push_back(frameName);
            }

            callback("", info->folderName, frameNames);
        });

        frames->release();
    });
}

void LazyIcon::visit() {
    CCNode::visit();

    if (!m_bVisible || m_visited) return;

    m_visited = true;

    if (!m_info || !m_info->sheetName.empty() || m_type == IconType::Special) {
        auto textureNames = getTextureNames();
        loadSheet(
            textureNames.first,
            textureNames.second,
            m_info ? m_info->name : "",
            m_type,
            [self = Ref(this)](const std::string& err, const std::string& texture, const std::vector<std::string>& frames) {
                self->createIcon(err, texture, frames);
            }
        );
    }
    else if (!m_info->folderName.empty()) loadImages(
        m_info,
        [self = Ref(this)](const std::string& err, const std::string& texture, const std::vector<std::string>& frames) {
            self->createIcon(err, texture, frames);
        }
    );
}

LazyIcon::~LazyIcon() {
    auto spriteFrameCache = CCSpriteFrameCache::get();
    for (auto& frame : m_frames) {
        spriteFrameCache->removeSpriteFrameByName(frame.c_str());
    }

    if (!m_texture.empty()) CCTextureCache::get()->removeTextureForKey(m_texture.c_str());
}
