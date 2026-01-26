#include "SimpleIcon.hpp"
#include "../../utils/Filesystem.hpp"
#include "../../utils/Icons.hpp"
#include "../../utils/Json.hpp"
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

const matjson::Value definitions = [] {
    matjson::Value json;

    auto def = Load::readPlist(dirs::getResourcesDir() / L("objectDefinitions.plist"));
    if (def.isErr()) {
        log::error("Failed to load objectDefinitions.plist: {}", def.unwrapErr());
        return json;
    }

    for (auto& value : def.unwrap()) {
        auto keyOpt = value.getKey();
        if (!keyOpt.has_value()) continue;

        auto key = std::move(keyOpt).value();
        if (key != "Robot" && key != "Spider") continue;

        auto& animDesc = value["animDesc"];
        if (auto filenameRes = animDesc.asString()) {
            auto filename = std::move(filenameRes).unwrap();
            if (auto desc = Load::readPlist(dirs::getResourcesDir() / Filesystem::strWide(filename))) {
                animDesc = std::move(desc).unwrap();
            }
            else {
                log::error("Failed to load {}: {}", filename, desc.unwrapErr());
                animDesc = matjson::Value();
            }
        }

        json.set(key, std::move(value));
    }

    return json;
}();

std::vector<SpriteDefinition> parseDefinition(const matjson::Value& definition) {
    std::vector<SpriteDefinition> definitions;
    for (size_t i = 0; i < definition.size(); i++) {
        auto& value = definition[fmt::format("sprite_{}", i)];
        if (!value.isObject()) continue;

        auto& def = definitions.emplace_back();
        def.position = CCPointFromString(Json::get<std::string>(value, "position").c_str());
        def.scale = CCPointFromString(Json::get<std::string>(value, "scale").c_str());
        def.flipped = CCPointFromString(Json::get<std::string>(value, "flipped").c_str());
        def.rotation = jasmine::convert::getOr<float>(Json::get<std::string>(value, "rotation"));
        def.zValue = jasmine::convert::getOr<int>(Json::get<std::string>(value, "zValue"));
        def.tag = jasmine::convert::getOr<int>(Json::get<std::string>(value, "tag"));
    }
    return definitions;
}

template <typename... Args>
CCSprite* spriteWithFrame(fmt::format_string<Args...> name, Args&&... args) {
    auto frame = Icons::getFrame(name, std::forward<Args>(args)...);
    auto sprite = frame ? CCSprite::createWithSpriteFrame(frame) : CCSprite::create();
    sprite->setVisible(frame != nullptr);
    return sprite;
}

bool SimpleIcon::init(IconType type, std::string_view name) {
    if (!CCNode::init()) return false;

    setAnchorPoint({ 0.5f, 0.5f });

    if (type == IconType::Robot || type == IconType::Spider) {
        createComplexIcon(type, name);
    }
    else {
        createSimpleIcon(type, name);
    }

    return true;
}

void SimpleIcon::createSimpleIcon(IconType type, std::string_view name) {
    auto yOffset = type == IconType::Ufo ? -7.0f : 0.0f;
    auto scale = type == IconType::Ball ? 0.9f : 1.0f;

    auto primarySprite = spriteWithFrame("{}_001.png", name);
    primarySprite->setPositionY(yOffset);
    primarySprite->setScale(scale);
    primarySprite->setID("sprite_001");
    addChild(primarySprite, 0);
    m_targets["_001"].push_back(primarySprite);
    m_mainColorSprites.emplace_back(primarySprite, 1.0f);

    auto secondarySprite = spriteWithFrame("{}_2_001.png", name);
    secondarySprite->setPositionY(yOffset);
    secondarySprite->setScale(scale);
    secondarySprite->setID("sprite_2_001");
    addChild(secondarySprite, -1);
    m_targets["_2_001"].push_back(secondarySprite);
    m_secondaryColorSprites.emplace_back(secondarySprite, 1.0f);

    if (type == IconType::Ufo) {
        auto tertiarySprite = spriteWithFrame("{}_3_001.png", name);
        tertiarySprite->setPositionY(yOffset);
        tertiarySprite->setScale(scale);
        tertiarySprite->setID("sprite_3_001");
        addChild(tertiarySprite, -2);
        m_targets["_3_001"].push_back(tertiarySprite);
    }

    auto glowSprite = spriteWithFrame("{}_glow_001.png", name);
    glowSprite->setPositionY(yOffset);
    glowSprite->setScale(scale);
    glowSprite->setID("sprite_glow_001");
    addChild(glowSprite, -3);
    m_targets["_glow_001"].push_back(glowSprite);
    m_glowColorSprites.push_back(glowSprite);

    auto extraSprite = spriteWithFrame("{}_extra_001.png", name);
    extraSprite->setPositionY(yOffset);
    extraSprite->setScale(scale);
    extraSprite->setID("sprite_extra_001");
    addChild(extraSprite, 1);
    m_targets["_extra_001"].push_back(extraSprite);
}

void SimpleIcon::createComplexIcon(IconType type, std::string_view name) {
    auto spider = type == IconType::Spider;

    std::string_view anim = "idle";
    std::string_view key = spider ? "Spider" : "Robot";

    auto& definition = definitions[key];
    if (!definition.isObject()) return;

    auto& animations = definition["animations"];
    if (!animations.isObject()) return;

    auto& animation = animations[anim];
    if (!animation.isObject()) return;

    auto& animDesc = definition["animDesc"];
    if (!animDesc.isObject()) return;

    auto& container = animDesc["animationContainer"];
    if (!container.isObject()) return;

    auto& usedTextures = animDesc["usedTextures"];
    if (!usedTextures.isObject()) return;

    auto glowNode = CCNode::create();
    glowNode->setAnchorPoint({ 0.5f, 0.5f });
    glowNode->setID("glow-node");
    addChild(glowNode, -1);

    auto minSize = spider ? 13 : 12;
    for (size_t i = 0; i < usedTextures.size(); i++) {
        auto& usedTexture = usedTextures[fmt::format("texture_{}", i)];
        if (!usedTexture.isObject()) continue;

        auto texture = Json::get<std::string>(usedTexture, "texture");
        if (texture.size() < minSize) continue;

        texture.erase(0, minSize - 4).erase(texture.size() - 4);

        auto customID = Json::get<std::string>(usedTexture, "customID");
        auto factor = customID == "back01" || customID == "back02" || customID == "back03" ? (spider ? 0.5f : 0.7f) : 1.0f;
        ccColor3B spriteColor = { (uint8_t)(factor * 255.0f), (uint8_t)(factor * 255.0f), (uint8_t)(factor * 255.0f) };

        auto partNode = new CCSpritePlus();
        partNode->init();
        partNode->autorelease();
        partNode->m_propagateScaleChanges = true;
        partNode->m_propagateFlipChanges = true;
        partNode->setID(fmt::format("sprite_{}", i + 1));
        addChild(partNode);
        m_spriteParts.push_back(partNode);

        auto prefix = partNode->getID();

        auto primarySprite = spriteWithFrame("{}{}.png", name, texture);
        primarySprite->setColor(spriteColor);
        primarySprite->setID(fmt::format("{}{}", prefix, texture));
        partNode->addChild(primarySprite, 0);
        m_targets[texture].push_back(primarySprite);
        m_mainColorSprites.emplace_back(primarySprite, factor);

        texture.replace(0, 0, "_2", 2);

        auto secondarySprite = spriteWithFrame("{}{}.png", name, texture);
        secondarySprite->setColor(spriteColor);
        secondarySprite->setID(fmt::format("{}{}", prefix, texture));
        partNode->addChild(secondarySprite, -1);
        m_targets[texture].push_back(secondarySprite);
        m_secondaryColorSprites.emplace_back(secondarySprite, factor);

        texture.replace(0, 2, "_glow", 5);

        auto glowSprite = spriteWithFrame("{}{}.png", name, texture);
        glowSprite->setID(fmt::format("{}{}", prefix, texture));
        glowNode->addChild(glowSprite, -1);
        partNode->addFollower(glowSprite);
        m_targets[texture].push_back(glowSprite);
        m_glowColorSprites.push_back(glowSprite);

        if (texture.starts_with("_01")) {
            texture.replace(0, 5, "_extra", 6);

            auto extraSprite = spriteWithFrame("{}{}.png", name, texture);
            extraSprite->setID(fmt::format("{}{}", prefix, texture));
            partNode->addChild(extraSprite, 1);
            m_targets[texture].push_back(extraSprite);
        }
    }

    auto singleFrame = Json::get<std::string>(animation, "singleFrame");
    if (!singleFrame.empty()) {
        updateComplexSprite(parseDefinition(container[singleFrame]));
    }
    else {
        auto frames = jasmine::convert::getOr<int>(Json::get<std::string>(animation, "frames"));
        m_divisor = std::max(0.01f, jasmine::convert::getOr<float>(Json::get<std::string>(animation, "delay")) * frames);
        auto prefix = fmt::format("{}_{}_", key, anim);
        m_definitions.reserve(frames);
        for (int i = 1; i <= frames; i++) {
            m_definitions.push_back(parseDefinition(container[fmt::format("{}{:03}.png", prefix, i)]));
        }
        auto looped = Json::get<std::string>(animation, "looped");
        m_looped = !looped.empty() && looped != std::string_view("0", 1) && looped != std::string_view("false", 5);
        m_elapsed = 0.0f;
        scheduleUpdate();
    }
}

void SimpleIcon::updateComplexSprite(std::span<SpriteDefinition const> definitions) {
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

std::span<CCSprite*> SimpleIcon::getTargets(std::string_view suffix) {
    auto it = m_targets.find(suffix);
    return it != m_targets.end() ? std::span<CCSprite*>(it->second) : std::span<CCSprite*>();
}

void SimpleIcon::setMainColor(const ccColor3B& color) {
    for (auto [sprite, factor] : m_mainColorSprites) {
        sprite->setColor({ (uint8_t)(color.r * factor), (uint8_t)(color.g * factor), (uint8_t)(color.b * factor) });
    }
}

void SimpleIcon::setSecondaryColor(const ccColor3B& color) {
    for (auto [sprite, factor] : m_secondaryColorSprites) {
        sprite->setColor({ (uint8_t)(color.r * factor), (uint8_t)(color.g * factor), (uint8_t)(color.b * factor) });
    }
}

void SimpleIcon::setGlowColor(const ccColor3B& color) {
    for (auto sprite : m_glowColorSprites) {
        sprite->setColor(color);
    }
}

void SimpleIcon::setGlow(bool glow) {
    for (auto sprite : m_glowColorSprites) {
        sprite->setOpacity(glow ? 255 : 0);
    }
}
