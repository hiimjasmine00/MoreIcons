#include "SimpleIcon.hpp"
#include "../../utils/Filesystem.hpp"
#include "../../utils/Icons.hpp"
#include "../../utils/Load.hpp"
#include <Geode/binding/CCSpritePlus.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Log.hpp>
#include <jasmine/convert.hpp>

using namespace geode::prelude;

SimpleIcon* SimpleIcon::create(IconType type, std::string_view name) {
    auto ret = new SimpleIcon();
    if (ret->init(type, name)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

matjson::Value getJSON(Filesystem::PathView filename, std::string_view filenameNarrow) {
    if (auto def = Load::readPlist(dirs::getResourcesDir() / filename)) {
        return std::move(def).unwrap();
    }
    else {
        log::error("Failed to load {}: {}", filenameNarrow, def.unwrapErr());
        return {};
    }
}

const matjson::Value definitions = [] {
    auto json = getJSON(L("objectDefinitions.plist"), "objectDefinitions.plist");
    for (auto it = json.begin(); it != json.end();) {
        auto& value = *it;
        if (auto keyOpt = value.getKey()) {
            auto key = std::move(keyOpt).value();
            if (key == "Robot" || key == "Spider") {
                auto& animDesc = value["animDesc"];
                if (auto filenameRes = animDesc.asString()) {
                    auto filename = std::move(filenameRes).unwrap();
                    animDesc = getJSON(Filesystem::strWide(filename), filename);
                }
                ++it;
            }
            else json.erase(key);
        }
    }
    return json;
}();

std::vector<SpriteDefinition> parseDefinition(const matjson::Value& definition) {
    std::vector<SpriteDefinition> definitions;
    for (size_t i = 0; i < definition.size(); i++) {
        auto& value = definition[fmt::format("sprite_{}", i)];
        if (!value.isObject()) continue;

        auto& def = definitions.emplace_back();
        def.position = CCPointFromString(value.get<std::string>("position").unwrapOrDefault().c_str());
        def.scale = CCPointFromString(value.get<std::string>("scale").unwrapOrDefault().c_str());
        def.flipped = CCPointFromString(value.get<std::string>("flipped").unwrapOrDefault().c_str());
        def.rotation = jasmine::convert::getOr<float>(value.get<std::string>("rotation").unwrapOrDefault());
        def.zValue = jasmine::convert::getOr<int>(value.get<std::string>("zValue").unwrapOrDefault());
        def.tag = jasmine::convert::getOr<int>(value.get<std::string>("tag").unwrapOrDefault());
    }
    return definitions;
}

CCSprite* spriteWithFrame(CCSpriteFrame* frame) {
    auto sprite = frame ? CCSprite::createWithSpriteFrame(frame) : CCSprite::create();
    sprite->setVisible(frame != nullptr);
    return sprite;
}

bool SimpleIcon::init(IconType type, std::string_view name) {
    if (!CCLayer::init()) return false;

    setContentSize({ 0.0f, 0.0f });
    ignoreAnchorPointForPosition(false);

    if (type != IconType::Robot && type != IconType::Spider) {
        auto yOffset = type == IconType::Ufo ? -7.0f : 0.0f;
        auto scale = type == IconType::Ball ? 0.9f : 1.0f;

        auto primarySprite = spriteWithFrame(Icons::getFrame("{}_001.png", name));
        primarySprite->setPositionY(yOffset);
        primarySprite->setScale(scale);
        primarySprite->setID("sprite_001");
        addChild(primarySprite, 0);
        m_targets["_001"].push_back(primarySprite);
        m_mainColorSprites.emplace_back(primarySprite, 1.0f);

        auto secondarySprite = spriteWithFrame(Icons::getFrame("{}_2_001.png", name));
        secondarySprite->setPositionY(yOffset);
        secondarySprite->setScale(scale);
        secondarySprite->setID("sprite_2_001");
        addChild(secondarySprite, -1);
        m_targets["_2_001"].push_back(secondarySprite);
        m_secondaryColorSprites.emplace_back(secondarySprite, 1.0f);

        if (type == IconType::Ufo) {
            auto tertiarySprite = spriteWithFrame(Icons::getFrame("{}_3_001.png", name));
            tertiarySprite->setPositionY(yOffset);
            tertiarySprite->setScale(scale);
            tertiarySprite->setID("sprite_3_001");
            addChild(tertiarySprite, -2);
            m_targets["_3_001"].push_back(tertiarySprite);
        }

        auto glowSprite = spriteWithFrame(Icons::getFrame("{}_glow_001.png", name));
        glowSprite->setPositionY(yOffset);
        glowSprite->setScale(scale);
        glowSprite->setID("sprite_glow_001");
        addChild(glowSprite, -3);
        m_targets["_glow_001"].push_back(glowSprite);
        m_glowColorSprites.push_back(glowSprite);

        auto extraSprite = spriteWithFrame(Icons::getFrame("{}_extra_001.png", name));
        extraSprite->setPositionY(yOffset);
        extraSprite->setScale(scale);
        extraSprite->setID("sprite_extra_001");
        addChild(extraSprite, 1);
        m_targets["_extra_001"].push_back(extraSprite);

        return true;
    }

    auto spider = type == IconType::Spider;

    std::string_view anim = "idle";
    std::string_view key = spider ? "Spider" : "Robot";

    auto& definition = definitions[key];
    if (!definition.isObject()) return true;

    auto& animations = definition["animations"];
    if (!animations.isObject()) return true;

    auto& animation = animations[anim];
    if (!animation.isObject()) return true;

    auto& animDesc = definition["animDesc"];
    if (!animDesc.isObject()) return true;

    auto& container = animDesc["animationContainer"];
    if (!container.isObject()) return true;

    auto& usedTextures = animDesc["usedTextures"];
    if (!usedTextures.isObject()) return true;

    auto glowNode = CCNode::create();
    glowNode->setAnchorPoint({ 0.5f, 0.5f });
    glowNode->setID("glow-node");

    for (size_t i = 0; i < usedTextures.size(); i++) {
        auto& usedTexture = usedTextures[fmt::format("texture_{}", i)];
        if (!usedTexture.isObject()) continue;

        auto texture = usedTexture.get<std::string>("texture").unwrapOrDefault();
        if ((spider && texture.size() < 12) || (!spider && texture.size() < 11)) continue;

        auto index = jasmine::convert::getOr<int>(std::string_view(texture.data() + (spider ? 10 : 9), 2));
        if (index <= 0) continue;

        auto customID = usedTexture.get<std::string>("customID").unwrapOrDefault();
        auto factor = customID == "back01" || customID == "back02" || customID == "back03" ? (spider ? 0.5f : 0.7f) : 1.0f;
        ccColor3B spriteColor = { (uint8_t)(factor * 255.0f), (uint8_t)(factor * 255.0f), (uint8_t)(factor * 255.0f) };

        auto partNode = new CCSpritePlus();
        partNode->init();
        partNode->autorelease();
        partNode->m_propagateScaleChanges = true;
        partNode->m_propagateFlipChanges = true;

        auto prefix = fmt::format("sprite_{}", i + 1);

        auto primarySprite = spriteWithFrame(Icons::getFrame("{}_{:02}_001.png", name, index));
        primarySprite->setColor(spriteColor);
        primarySprite->setID(fmt::format("{}_001", prefix));
        partNode->addChild(primarySprite, 0);
        m_targets[fmt::format("_{:02}_001", index)].push_back(primarySprite);
        m_mainColorSprites.emplace_back(primarySprite, factor);

        auto secondarySprite = spriteWithFrame(Icons::getFrame("{}_{:02}_2_001.png", name, index));
        secondarySprite->setColor(spriteColor);
        secondarySprite->setID(fmt::format("{}_2_001", prefix));
        partNode->addChild(secondarySprite, -1);
        m_targets[fmt::format("_{:02}_2_001", index)].push_back(secondarySprite);
        m_secondaryColorSprites.emplace_back(secondarySprite, factor);

        auto glowSprite = spriteWithFrame(Icons::getFrame("{}_{:02}_glow_001.png", name, index));
        glowSprite->setID(fmt::format("{}_glow_001", prefix));
        glowNode->addChild(glowSprite, -1);
        partNode->addFollower(glowSprite);
        m_targets[fmt::format("_{:02}_glow_001", index)].push_back(glowSprite);
        m_glowColorSprites.push_back(glowSprite);

        if (index == 1) {
            auto extraSprite = spriteWithFrame(Icons::getFrame("{}_01_extra_001.png", name));
            extraSprite->setID(fmt::format("{}_extra_001", prefix));
            partNode->addChild(extraSprite, 1);
            m_targets["_01_extra_001"].push_back(extraSprite);
        }

        partNode->setID(std::move(prefix));
        m_spriteParts.push_back(partNode);
        addChild(partNode);
    }

    addChild(glowNode, -1);

    if (auto singleFrame = animation.get("singleFrame")) {
        updateComplexSprite(parseDefinition(container[singleFrame.unwrap().asString().unwrapOrDefault()]));
    }
    else {
        auto frames = jasmine::convert::getOr<int>(animation.get<std::string>("frames").unwrapOrDefault());
        m_divisor = std::max(0.01f, jasmine::convert::getOr<float>(animation.get<std::string>("delay").unwrapOrDefault()) * frames);
        auto prefix = fmt::format("{}_{}_", key, anim);
        m_definitions.reserve(frames);
        for (int i = 1; i <= frames; i++) {
            m_definitions.push_back(parseDefinition(container[fmt::format("{}{:03}.png", prefix, i)]));
        }
        auto looped = animation.get<std::string>("looped").unwrapOrDefault();
        m_looped = !looped.empty() && looped != "0" && looped != "false";
        m_elapsed = 0.0f;
        scheduleUpdate();
    }

    return true;
}

void SimpleIcon::updateComplexSprite(const std::vector<SpriteDefinition>& definitions) {
    for (auto spritePart : m_spriteParts) {
        spritePart->setVisible(false);
    }

    for (auto& definition : definitions) {
        auto spritePart = m_spriteParts[definition.tag];
        spritePart->setPosition(definition.position);
        spritePart->setScaleX(definition.scale.x);
        spritePart->setScaleY(definition.scale.y);
        spritePart->setRotation(definition.rotation);
        spritePart->setFlipX(definition.flipped.x != 0.0f);
        spritePart->setFlipY(definition.flipped.y != 0.0f);
        if (spritePart->getZOrder() != definition.zValue) {
            spritePart->getParent()->reorderChild(spritePart, definition.zValue);
        }
        spritePart->setVisible(true);
    }
}

void SimpleIcon::update(float dt) {
    m_elapsed += dt;
    auto interval = m_elapsed / m_divisor;
    if (!m_looped && interval >= 1.0f) {
        m_elapsed = 0.0f;
        return unscheduleUpdate();
    }

    updateComplexSprite(m_definitions[fmodf(interval, 1.0f) * m_definitions.size()]);
}

const std::vector<CCSprite*>& SimpleIcon::getTargets(const std::string& suffix) {
    return m_targets[suffix];
}

void SimpleIcon::setColors(const ccColor3B& primary, const ccColor3B& secondary, const ccColor3B& glow) {
    for (auto [sprite, factor] : m_mainColorSprites) {
        sprite->setColor({ (uint8_t)(primary.r * factor), (uint8_t)(primary.g * factor), (uint8_t)(primary.b * factor) });
    }
    for (auto [sprite, factor] : m_secondaryColorSprites) {
        sprite->setColor({ (uint8_t)(secondary.r * factor), (uint8_t)(secondary.g * factor), (uint8_t)(secondary.b * factor) });
    }
    for (auto sprite : m_glowColorSprites) {
        sprite->setColor(glow);
    }
}
