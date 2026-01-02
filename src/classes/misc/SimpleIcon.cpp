#include "SimpleIcon.hpp"
#include "../../utils/Filesystem.hpp"
#include "../../utils/Icons.hpp"
#include "../../utils/Load.hpp"
#include <Geode/binding/CCSpritePlus.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Log.hpp>
#include <jasmine/convert.hpp>

using namespace geode::prelude;
using namespace std::string_literals;
using namespace std::string_view_literals;

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
        def.position = CCPointFromString(value["position"].asString().unwrapOrDefault().c_str());
        def.scale = CCPointFromString(value["scale"].asString().unwrapOrDefault().c_str());
        def.flipped = CCPointFromString(value["flipped"].asString().unwrapOrDefault().c_str());
        def.rotation = jasmine::convert::getOr<float>(value["rotation"].asString().unwrapOrDefault());
        def.zValue = jasmine::convert::getOr<int>(value["zValue"].asString().unwrapOrDefault());
        def.tag = jasmine::convert::getOr<int>(value["tag"].asString().unwrapOrDefault());
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

    if (type != IconType::Robot && type != IconType::Spider) {
        auto yOffset = type == IconType::Ufo ? -7.0f : 0.0f;
        auto scale = type == IconType::Ball ? 0.9f : 1.0f;

        auto primarySprite = spriteWithFrame("{}_001.png", name);
        primarySprite->setPositionY(yOffset);
        primarySprite->setScale(scale);
        primarySprite->setID("sprite_001"s);
        addChild(primarySprite, 0);
        m_targets["_001"s].push_back(primarySprite);
        m_mainColorSprites.emplace_back(primarySprite, 1.0f);

        auto secondarySprite = spriteWithFrame("{}_2_001.png", name);
        secondarySprite->setPositionY(yOffset);
        secondarySprite->setScale(scale);
        secondarySprite->setID("sprite_2_001"s);
        addChild(secondarySprite, -1);
        m_targets["_2_001"s].push_back(secondarySprite);
        m_secondaryColorSprites.emplace_back(secondarySprite, 1.0f);

        if (type == IconType::Ufo) {
            auto tertiarySprite = spriteWithFrame("{}_3_001.png", name);
            tertiarySprite->setPositionY(yOffset);
            tertiarySprite->setScale(scale);
            tertiarySprite->setID("sprite_3_001"s);
            addChild(tertiarySprite, -2);
            m_targets["_3_001"s].push_back(tertiarySprite);
        }

        auto glowSprite = spriteWithFrame("{}_glow_001.png", name);
        glowSprite->setPositionY(yOffset);
        glowSprite->setScale(scale);
        glowSprite->setID("sprite_glow_001"s);
        addChild(glowSprite, -3);
        m_targets["_glow_001"s].push_back(glowSprite);
        m_glowColorSprites.push_back(glowSprite);

        auto extraSprite = spriteWithFrame("{}_extra_001.png", name);
        extraSprite->setPositionY(yOffset);
        extraSprite->setScale(scale);
        extraSprite->setID("sprite_extra_001"s);
        addChild(extraSprite, 1);
        m_targets["_extra_001"s].push_back(extraSprite);

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
    glowNode->setID("glow-node"s);

    for (size_t i = 0; i < usedTextures.size(); i++) {
        auto& usedTexture = usedTextures[fmt::format("texture_{}", i)];
        if (!usedTexture.isObject()) continue;

        auto texture = usedTexture["texture"].asString().unwrapOrDefault();
        if ((spider && texture.size() < 12) || (!spider && texture.size() < 11)) continue;

        auto index = jasmine::convert::getOr<int>(std::string_view(texture.data() + (spider ? 10 : 9), 2));
        if (index <= 0) continue;

        auto customID = usedTexture["customID"].asString().unwrapOrDefault();
        auto factor = customID == "back01"sv || customID == "back02"sv || customID == "back03"sv ? (spider ? 0.5f : 0.7f) : 1.0f;
        ccColor3B spriteColor = { (uint8_t)(factor * 255.0f), (uint8_t)(factor * 255.0f), (uint8_t)(factor * 255.0f) };

        auto partNode = new CCSpritePlus();
        partNode->init();
        partNode->autorelease();
        partNode->m_propagateScaleChanges = true;
        partNode->m_propagateFlipChanges = true;

        auto prefix = fmt::format("sprite_{}", i + 1);

        auto primarySprite = spriteWithFrame("{}_{:02}_001.png", name, index);
        primarySprite->setColor(spriteColor);
        primarySprite->setID(fmt::format("{}_001", prefix));
        partNode->addChild(primarySprite, 0);
        m_targets[fmt::format("_{:02}_001", index)].push_back(primarySprite);
        m_mainColorSprites.emplace_back(primarySprite, factor);

        auto secondarySprite = spriteWithFrame("{}_{:02}_2_001.png", name, index);
        secondarySprite->setColor(spriteColor);
        secondarySprite->setID(fmt::format("{}_2_001", prefix));
        partNode->addChild(secondarySprite, -1);
        m_targets[fmt::format("_{:02}_2_001", index)].push_back(secondarySprite);
        m_secondaryColorSprites.emplace_back(secondarySprite, factor);

        auto glowSprite = spriteWithFrame("{}_{:02}_glow_001.png", name, index);
        glowSprite->setID(fmt::format("{}_glow_001", prefix));
        glowNode->addChild(glowSprite, -1);
        partNode->addFollower(glowSprite);
        m_targets[fmt::format("_{:02}_glow_001", index)].push_back(glowSprite);
        m_glowColorSprites.push_back(glowSprite);

        if (index == 1) {
            auto extraSprite = spriteWithFrame("{}_01_extra_001.png", name);
            extraSprite->setID(fmt::format("{}_extra_001", prefix));
            partNode->addChild(extraSprite, 1);
            m_targets["_01_extra_001"s].push_back(extraSprite);
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
        auto frames = jasmine::convert::getOr<int>(animation["frames"].asString().unwrapOrDefault());
        m_divisor = std::max(0.01f, jasmine::convert::getOr<float>(animation["delay"].asString().unwrapOrDefault()) * frames);
        auto prefix = fmt::format("{}_{}_", key, anim);
        m_definitions.reserve(frames);
        for (int i = 1; i <= frames; i++) {
            m_definitions.push_back(parseDefinition(container[fmt::format("{}{:03}.png", prefix, i)]));
        }
        auto looped = animation["looped"].asString().unwrapOrDefault();
        m_looped = !looped.empty() && looped != "0"sv && looped != "false"sv;
        m_elapsed = 0.0f;
        scheduleUpdate();
    }

    return true;
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

std::span<CCSprite* const> SimpleIcon::getTargets(const std::string& suffix) {
    return m_targets[suffix];
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
