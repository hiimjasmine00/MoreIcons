#include "EditIconPopup.hpp"
#include "IconPresetPopup.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/SpriteDescription.hpp>
#include <Geode/binding/Slider.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/string.hpp>
#include <texpack.hpp>

using namespace geode::prelude;

template <typename... T>
void notify(NotificationIcon icon, fmt::format_string<T...> message, T&&... args) {
    Notification::create(fmt::format(message, std::forward<T>(args)...), icon)->show();
}

EditIconPopup* EditIconPopup::create(MoreIconsPopup* popup, IconType type) {
    auto ret = new EditIconPopup();
    if (ret->initAnchored(450.0f, 280.0f, popup, type, "geode.loader/GE_square03.png")) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

CCArray* arrayWithObject(CCObject* obj) {
    auto arr = CCArray::create();
    if (obj) arr->addObject(obj);
    return arr;
}

template <class... T>
CCArray* arrayWithObjects(CCArray* parent, T... indices) {
    auto arr = CCArray::create();
    for (auto index : { indices... }) {
        if (auto obj = parent->objectAtIndex(index)) arr->addObject(obj);
    }
    return arr;
}

bool EditIconPopup::setup(MoreIconsPopup* popup, IconType type) {
    setID("EditIconPopup");
    setTitle(fmt::format("{} Editor", MoreIconsAPI::uppercase[(int)type]));
    m_title->setID("edit-icon-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_parentPopup = popup;
    m_pieceArrays = CCArray::create();
    m_pieceDefinitions = CCDictionary::create();
    m_pieces = CCDictionary::create();
    m_frames = CCDictionary::create();
    m_iconType = type;

    auto isRobot = type == IconType::Robot || type == IconType::Spider;

    auto iconBackground = CCScale9Sprite::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    iconBackground->setPosition({ 55.0f, 205.0f });
    iconBackground->setContentSize({ 80.0f, 80.0f });
    iconBackground->setOpacity(105);
    iconBackground->setID("icon-background");
    m_mainLayer->addChild(iconBackground);

    auto piecesBackground = CCScale9Sprite::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    piecesBackground->setPosition({ 270.0f, 222.0f });
    piecesBackground->setContentSize({ isRobot ? 260.0f : 330.0f, 45.0f });
    piecesBackground->setOpacity(105);
    piecesBackground->setID("pieces-background");
    m_mainLayer->addChild(piecesBackground);

    m_pieceMenu = CCMenu::create();
    m_pieceMenu->setPosition({ 270.0f, 222.0f });
    m_pieceMenu->setContentSize({ isRobot ? 260.0f : 330.0f, 45.0f });
    m_pieceMenu->ignoreAnchorPointForPosition(false);
    m_pieceMenu->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Even));
    m_pieceMenu->setID("piece-menu");
    m_mainLayer->addChild(m_pieceMenu);

    m_player = SimplePlayer::create(1);
    m_player->updatePlayerFrame(1, type);
    m_player->m_hasGlowOutline = true;
    m_player->updateColors();

    auto previewNode = CCNode::create();
    previewNode->setPosition({ 55.0f, 205.0f });
    previewNode->setScale(2.0f);
    previewNode->setAnchorPoint({ 0.5f, 0.5f });
    transferPlayerToNode(previewNode, m_player);
    previewNode->setID("preview-node");
    m_mainLayer->addChild(previewNode);

    if (isRobot) {
        m_suffix = "_01_001";

        auto prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        prevSprite->setScale(0.8f);
        auto prevButton = CCMenuItemExt::createSpriteExtra(prevSprite, [this](auto) {
            goToPage(m_page - 1);
        });
        prevButton->setPosition({ 120.0f, 222.0f });
        prevButton->setID("prev-button");
        m_buttonMenu->addChild(prevButton);

        auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        nextSprite->setScale(0.8f);
        nextSprite->setFlipX(true);
        auto nextButton = CCMenuItemExt::createSpriteExtra(nextSprite, [this](auto) {
            goToPage(m_page + 1);
        });
        nextButton->setPosition({ 420.0f, 222.0f });
        nextButton->setID("next-button");
        m_buttonMenu->addChild(nextButton);

        auto isSpider = type == IconType::Spider;
        auto robotSprite = isSpider ? m_player->m_spiderSprite : m_player->m_robotSprite;
        auto firstArray = robotSprite->m_paSprite->m_spriteParts;
        auto secondArray = robotSprite->m_secondArray;
        auto glowArray = robotSprite->m_glowSprite->getChildren();

        addPieceButton("_01_001", 0, arrayWithObjects(firstArray, 3));
        addPieceButton("_01_2_001", 0, arrayWithObjects(secondArray, 3));
        addPieceButton("_01_glow_001", 0, arrayWithObjects(glowArray, 3));
        addPieceButton("_01_extra_001", 0, arrayWithObject(robotSprite->m_extraSprite));
        if (isSpider) {
            addPieceButton("_02_001", 1, arrayWithObjects(firstArray, 0, 1, 5));
            addPieceButton("_02_2_001", 1, arrayWithObjects(secondArray, 0, 1, 5));
            addPieceButton("_02_glow_001", 1, arrayWithObjects(glowArray, 0, 1, 5));
            addPieceButton("_03_001", 2, arrayWithObjects(firstArray, 4));
            addPieceButton("_03_2_001", 2, arrayWithObjects(secondArray, 4));
            addPieceButton("_03_glow_001", 2, arrayWithObjects(glowArray, 4));
            addPieceButton("_04_001", 3, arrayWithObjects(firstArray, 2));
            addPieceButton("_04_2_001", 3, arrayWithObjects(secondArray, 2));
            addPieceButton("_04_glow_001", 3, arrayWithObjects(glowArray, 2));
        }
        else {
            addPieceButton("_02_001", 1, arrayWithObjects(firstArray, 1, 5));
            addPieceButton("_02_2_001", 1, arrayWithObjects(secondArray, 1, 5));
            addPieceButton("_02_glow_001", 1, arrayWithObjects(glowArray, 1, 5));
            addPieceButton("_03_001", 2, arrayWithObjects(firstArray, 0, 4));
            addPieceButton("_03_2_001", 2, arrayWithObjects(secondArray, 0, 4));
            addPieceButton("_03_glow_001", 2, arrayWithObjects(glowArray, 0, 4));
            addPieceButton("_04_001", 3, arrayWithObjects(firstArray, 2, 6));
            addPieceButton("_04_2_001", 3, arrayWithObjects(secondArray, 2, 6));
            addPieceButton("_04_glow_001", 3, arrayWithObjects(glowArray, 2, 6));
        }
    }
    else {
        m_suffix = "_001";

        addPieceButton("_001", 0, arrayWithObject(m_player->m_firstLayer));
        addPieceButton("_2_001", 0, arrayWithObject(m_player->m_secondLayer));
        if (type == IconType::Ufo) addPieceButton("_3_001", 0, arrayWithObject(m_player->m_birdDome));
        addPieceButton("_glow_001", 0, arrayWithObject(m_player->m_outlineSprite));
        addPieceButton("_extra_001", 0, arrayWithObject(m_player->m_detailSprite));
    }

    m_pieceMenu->updateLayout();

    m_selectSprite = CCSprite::createWithSpriteFrameName("GJ_select_001.png");
    m_selectSprite->setPosition(m_mainLayer->convertToNodeSpace(m_pieceMenu->convertToWorldSpace(m_pieceMenu->getChildByIndex(0)->getPosition())));
    m_selectSprite->setTag(0);
    m_selectSprite->setID("select-sprite");
    m_mainLayer->addChild(m_selectSprite);

    m_offsetXSlider = Slider::create(nullptr, nullptr, 0.75f);
    m_offsetXSlider->setPosition({ 185.0f, 165.0f });
    m_offsetXSlider->setValue((m_offsetX + 20.0f) / 40.0f);
    m_offsetXSlider->setID("offset-x-slider");
    m_mainLayer->addChild(m_offsetXSlider);

    m_offsetXLabel = CCLabelBMFont::create(fmt::format("Offset X: {:.1f}", m_offsetX).c_str(), "goldFont.fnt");
    m_offsetXLabel->setPosition({ 185.0f, 185.0f });
    m_offsetXLabel->setScale(0.6f);
    m_offsetXLabel->setID("offset-x-label");
    m_mainLayer->addChild(m_offsetXLabel);

    CCMenuItemExt::assignCallback<SliderThumb>(m_offsetXSlider->m_touchLogic->m_thumb, [this](SliderThumb* sender) {
        m_offsetX = roundf(sender->getValue() * 400.0f - 200.0f) / 10.0f;
        static_cast<CCFloat*>(m_pieceDefinitions->objectForKey(fmt::format("offsetX{}", m_suffix)))->m_fValue = m_offsetX;
        updateTargets();
        m_offsetXLabel->setString(fmt::format("Offset X: {:.1f}", m_offsetX).c_str());
        m_hasChanged = true;
    });

    m_offsetYSlider = Slider::create(nullptr, nullptr, 0.75f);
    m_offsetYSlider->setPosition({ 355.0f, 165.0f });
    m_offsetYSlider->setValue((m_offsetY + 20.0f) / 40.0f);
    m_offsetYSlider->setID("offset-y-slider");
    m_mainLayer->addChild(m_offsetYSlider);

    m_offsetYLabel = CCLabelBMFont::create(fmt::format("Offset Y: {:.1f}", m_offsetY).c_str(), "goldFont.fnt");
    m_offsetYLabel->setPosition({ 355.0f, 185.0f });
    m_offsetYLabel->setScale(0.6f);
    m_offsetYLabel->setID("offset-y-label");
    m_mainLayer->addChild(m_offsetYLabel);

    CCMenuItemExt::assignCallback<SliderThumb>(m_offsetYSlider->m_touchLogic->m_thumb, [this](SliderThumb* sender) {
        m_offsetY = roundf(sender->getValue() * 400.0f - 200.0f) / 10.0f;
        static_cast<CCFloat*>(m_pieceDefinitions->objectForKey(fmt::format("offsetY{}", m_suffix)))->m_fValue = m_offsetY;
        updateTargets();
        m_offsetYLabel->setString(fmt::format("Offset Y: {:.1f}", m_offsetY).c_str());
        m_hasChanged = true;
    });

    m_rotationXSlider = Slider::create(nullptr, nullptr, 0.75f);
    m_rotationXSlider->setPosition({ 185.0f, 125.0f });
    m_rotationXSlider->setValue(m_rotationX / 360.0f);
    m_rotationXSlider->setID("rotation-x-slider");
    m_mainLayer->addChild(m_rotationXSlider);

    m_rotationXLabel = CCLabelBMFont::create(fmt::format("Rotation X: {:.0f}", m_rotationX).c_str(), "goldFont.fnt");
    m_rotationXLabel->setPosition({ 185.0f, 145.0f });
    m_rotationXLabel->setScale(0.6f);
    m_rotationXLabel->setID("rotation-x-label");
    m_mainLayer->addChild(m_rotationXLabel);

    CCMenuItemExt::assignCallback<SliderThumb>(m_rotationXSlider->m_touchLogic->m_thumb, [this](SliderThumb* sender) {
        m_rotationX = roundf(sender->getValue() * 360.0f);
        static_cast<CCFloat*>(m_pieceDefinitions->objectForKey(fmt::format("rotationX{}", m_suffix)))->m_fValue = m_rotationX;
        updateTargets();
        m_rotationXLabel->setString(fmt::format("Rotation X: {:.0f}", m_rotationX).c_str());
        m_hasChanged = true;
    });

    m_rotationYSlider = Slider::create(nullptr, nullptr, 0.75f);
    m_rotationYSlider->setPosition({ 355.0f, 125.0f });
    m_rotationYSlider->setValue(m_rotationY / 360.0f);
    m_rotationYSlider->setID("rotation-y-slider");
    m_mainLayer->addChild(m_rotationYSlider);

    m_rotationYLabel = CCLabelBMFont::create(fmt::format("Rotation Y: {:.0f}", m_rotationY).c_str(), "goldFont.fnt");
    m_rotationYLabel->setPosition({ 355.0f, 145.0f });
    m_rotationYLabel->setScale(0.6f);
    m_rotationYLabel->setID("rotation-y-label");
    m_mainLayer->addChild(m_rotationYLabel);

    CCMenuItemExt::assignCallback<SliderThumb>(m_rotationYSlider->m_touchLogic->m_thumb, [this](SliderThumb* sender) {
        m_rotationY = roundf(sender->getValue() * 360.0f);
        static_cast<CCFloat*>(m_pieceDefinitions->objectForKey(fmt::format("rotationY{}", m_suffix)))->m_fValue = m_rotationY;
        updateTargets();
        m_rotationYLabel->setString(fmt::format("Rotation Y: {:.0f}", m_rotationY).c_str());
        m_hasChanged = true;
    });

    m_scaleXSlider = Slider::create(nullptr, nullptr, 0.75f);
    m_scaleXSlider->setPosition({ 185.0f, 85.0f });
    m_scaleXSlider->setValue((m_scaleX + 10.0f) / 20.0f);
    m_scaleXSlider->setID("scale-x-slider");
    m_mainLayer->addChild(m_scaleXSlider);

    m_scaleXLabel = CCLabelBMFont::create(fmt::format("Scale X: {:.1f}", m_scaleX).c_str(), "goldFont.fnt");
    m_scaleXLabel->setPosition({ 185.0f, 105.0f });
    m_scaleXLabel->setScale(0.6f);
    m_scaleXLabel->setID("scale-x-label");
    m_mainLayer->addChild(m_scaleXLabel);

    CCMenuItemExt::assignCallback<SliderThumb>(m_scaleXSlider->m_touchLogic->m_thumb, [this](SliderThumb* sender) {
        m_scaleX = roundf(sender->getValue() * 200.0f - 100.0f) / 10.0f;
        static_cast<CCFloat*>(m_pieceDefinitions->objectForKey(fmt::format("scaleX{}", m_suffix)))->m_fValue = m_scaleX;
        updateTargets();
        m_scaleXLabel->setString(fmt::format("Scale X: {:.1f}", m_scaleX).c_str());
        m_hasChanged = true;
    });

    m_scaleYSlider = Slider::create(nullptr, nullptr, 0.75f);
    m_scaleYSlider->setPosition({ 355.0f, 85.0f });
    m_scaleYSlider->setValue((m_scaleY + 10.0f) / 20.0f);
    m_scaleYSlider->setID("scale-y-slider");
    m_mainLayer->addChild(m_scaleYSlider);

    m_scaleYLabel = CCLabelBMFont::create(fmt::format("Scale Y: {:.1f}", m_scaleY).c_str(), "goldFont.fnt");
    m_scaleYLabel->setPosition({ 355.0f, 105.0f });
    m_scaleYLabel->setScale(0.6f);
    m_scaleYLabel->setID("scale-y-label");
    m_mainLayer->addChild(m_scaleYLabel);

    CCMenuItemExt::assignCallback<SliderThumb>(m_scaleYSlider->m_touchLogic->m_thumb, [this](SliderThumb* sender) {
        m_scaleY = roundf(sender->getValue() * 200.0f - 100.0f) / 10.0f;
        static_cast<CCFloat*>(m_pieceDefinitions->objectForKey(fmt::format("scaleY{}", m_suffix)))->m_fValue = m_scaleY;
        updateTargets();
        m_scaleYLabel->setString(fmt::format("Scale Y: {:.1f}", m_scaleY).c_str());
        m_hasChanged = true;
    });

    auto pieceButtonMenu = CCMenu::create();
    pieceButtonMenu->setPosition({ 270.0f, 60.0f });
    pieceButtonMenu->setContentSize({ 330.0f, 20.0f });
    pieceButtonMenu->ignoreAnchorPointForPosition(false);
    pieceButtonMenu->setLayout(RowLayout::create()->setGap(30.0f));
    pieceButtonMenu->setID("piece-button-menu");
    m_mainLayer->addChild(pieceButtonMenu);

    auto pieceImportSprite = ButtonSprite::create("Import", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    pieceImportSprite->setScale(0.6f);
    auto pieceImportButton = CCMenuItemExt::createSpriteExtra(pieceImportSprite, [this](auto) {
        m_listener.bind([this](Task<geode::Result<std::filesystem::path>>::Event* event) {
            auto res = event->getValue();
            if (res && res->isErr()) return notify(NotificationIcon::Error, "Failed to import PNG file: {}", res->unwrapErr());
            if (!res || !res->isOk()) return;

            auto imageRes = texpack::fromPNG(res->unwrap());
            if (imageRes.isErr()) return notify(NotificationIcon::Error, "Failed to load image: {}", imageRes.unwrapErr());

            auto image = std::move(imageRes).unwrap();
            Autorelease texture = new CCTexture2D();
            texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
                (float)image.width,
                (float)image.height
            });

            m_frames->setObject(CCSpriteFrame::createWithTexture(texture, { { 0.0f, 0.0f }, texture->getContentSize() }),
                fmt::format("{}.png", m_suffix));
            updatePieces();
        });

        m_listener.setFilter(file::pick(file::PickMode::OpenFile, {
            .filters = {{
                .description = "PNG Files",
                .files = { "*.png" }
            }}
        }));
    });
    pieceImportButton->setID("piece-import-button");
    pieceButtonMenu->addChild(pieceImportButton);

    auto piecePresetSprite = ButtonSprite::create("Preset", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    piecePresetSprite->setScale(0.6f);
    auto piecePresetButton = CCMenuItemExt::createSpriteExtra(piecePresetSprite, [this](auto) {
        auto key = fmt::format("{}.png", m_suffix);
        IconPresetPopup::create(m_iconType, key, [this, key](int id, IconInfo* info) {
            auto frame = MoreIconsAPI::getFrame(info
                ? fmt::format("{}{}", info->name, key)
                : fmt::format("{}{:02}{}", MoreIconsAPI::prefixes[(int)m_iconType], id, key));
            if (frame) m_frames->setObject(frame, key);
            else m_frames->removeObjectForKey(key);
            updatePieces();
        })->show();
    });
    piecePresetButton->setID("piece-preset-button");
    pieceButtonMenu->addChild(piecePresetButton);

    auto pieceClearSprite = ButtonSprite::create("Clear", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    pieceClearSprite->setScale(0.6f);
    auto pieceClearButton = CCMenuItemExt::createSpriteExtra(pieceClearSprite, [this](auto) {
        m_frames->setObject(MoreIconsAPI::getFrame("emptyFrame.png"), fmt::format("{}.png", m_suffix));
        updatePieces();
    });
    pieceClearButton->setID("piece-clear-button");
    pieceButtonMenu->addChild(pieceClearButton);

    pieceButtonMenu->updateLayout();

    auto iconButtonMenu = CCMenu::create();
    iconButtonMenu->setPosition({ 225.0f, 30.0f });
    iconButtonMenu->setContentSize({ 450.0f, 30.0f });
    iconButtonMenu->ignoreAnchorPointForPosition(false);
    iconButtonMenu->setLayout(RowLayout::create()->setGap(30.0f));
    iconButtonMenu->setID("icon-button-menu");
    m_mainLayer->addChild(iconButtonMenu);

    auto pngSprite = ButtonSprite::create("PNG", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    auto pngButton = CCMenuItemExt::createSpriteExtra(pngSprite, [this](auto) {
        m_listener.bind([this](Task<geode::Result<std::filesystem::path>>::Event* event) {
            auto res = event->getValue();
            if (res && res->isErr()) return notify(NotificationIcon::Error, "Failed to import PNG file: {}", res->unwrapErr());
            if (!res || !res->isOk()) return;

            m_selectedPNG = res->unwrap();
            if (!m_selectedPlist.empty()) updateWithSelectedFiles();
        });

        m_listener.setFilter(file::pick(file::PickMode::OpenFile, {
            .filters = {{
                .description = "PNG Files",
                .files = { "*.png" }
            }}
        }));
    });
    pngButton->setID("png-button");
    iconButtonMenu->addChild(pngButton);

    auto plistSprite = ButtonSprite::create("Plist", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    auto plistButton = CCMenuItemExt::createSpriteExtra(plistSprite, [this](auto) {
        m_listener.bind([this](Task<geode::Result<std::filesystem::path>>::Event* event) {
            auto res = event->getValue();
            if (res && res->isErr()) return notify(NotificationIcon::Error, "Failed to import Plist file: {}", res->unwrapErr());
            if (!res || !res->isOk()) return;

            m_selectedPlist = res->unwrap();
            if (!m_selectedPNG.empty()) updateWithSelectedFiles();
        });

        m_listener.setFilter(file::pick(file::PickMode::OpenFile, {
            .filters = {{
                .description = "Plist Files",
                .files = { "*.plist" }
            }}
        }));
    });
    plistButton->setID("plist-button");
    iconButtonMenu->addChild(plistButton);

    auto presetSprite = ButtonSprite::create("Preset", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    auto presetButton = CCMenuItemExt::createSpriteExtra(presetSprite, [this](auto) {
        IconPresetPopup::create(m_iconType, {}, [this](int id, IconInfo* info) {
            for (auto [suffix, _] : CCDictionaryExt<std::string, CCSprite*>(m_pieces)) {
                auto key = fmt::format("{}.png", suffix);
                auto frame = MoreIconsAPI::getFrame(info
                    ? fmt::format("{}{}", info->name, key)
                    : fmt::format("{}{:02}{}", MoreIconsAPI::prefixes[(int)m_iconType], id, key));
                if (frame) m_frames->setObject(frame, key);
                else m_frames->removeObjectForKey(key);
            }
            updatePieces();
        })->show();
    });
    presetButton->setID("preset-button");
    iconButtonMenu->addChild(presetButton);

    auto saveSprite = ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png", 0.8f);
    auto saveButton = CCMenuItemExt::createSpriteExtra(saveSprite, [](auto) {});
    saveButton->setID("save-button");
    iconButtonMenu->addChild(saveButton);

    iconButtonMenu->updateLayout();

    handleTouchPriority(this);

    return true;
}

void EditIconPopup::transferPlayerToNode(CCNode* node, SimplePlayer* player) {
    auto type = m_iconType;
    if (type == IconType::Robot || type == IconType::Spider) {
        auto robotSprite = type == IconType::Spider ? player->m_spiderSprite : player->m_robotSprite;

        auto glowSprite = robotSprite->m_glowSprite;
        glowSprite->removeFromParentAndCleanup(false);
        node->addChild(glowSprite);

        auto headSprite = robotSprite->m_headSprite;
        auto spriteParts = robotSprite->m_paSprite->m_spriteParts;
        auto secondArray = robotSprite->m_secondArray;
        for (int i = 0; i < spriteParts->count(); i++) {
            auto spritePart = static_cast<CCSpritePart*>(spriteParts->objectAtIndex(i));
            auto scaleX = spritePart->getScaleX();
            auto scaleY = spritePart->getScaleY();
            auto rotation = spritePart->getRotation();
            auto partNode = CCNode::create();
            partNode->setPosition(spritePart->getPosition());
            partNode->setAnchorPoint({ 0.5f, 0.5f });
            node->addChild(partNode, spritePart->getZOrder());

            auto secondSprite = static_cast<CCSprite*>(secondArray->objectAtIndex(i));
            secondSprite->removeFromParentAndCleanup(false);
            secondSprite->setPosition({ 0.0f, 0.0f });
            secondSprite->setScaleX(scaleX);
            secondSprite->setScaleY(scaleY);
            secondSprite->setRotation(rotation);
            partNode->addChild(secondSprite);

            if (spritePart == headSprite) {
                if (auto extraSprite = robotSprite->m_extraSprite) {
                    extraSprite->removeFromParentAndCleanup(false);
                    extraSprite->setPosition({ 0.0f, 0.0f });
                    partNode->addChild(extraSprite);
                }
            }

            spritePart->removeFromParentAndCleanup(false);
            spritePart->m_followers->removeAllObjects();
            spritePart->m_hasFollower = false;
            spritePart->setPosition({ 0.0f, 0.0f });
            partNode->addChild(spritePart, 0);
        }
    }
    else {
        Ref firstLayer = player->m_firstLayer;
        auto position = firstLayer->getPosition();
        auto scale = firstLayer->getScale();
        node->setPosition(node->getPosition() + position);
        while (firstLayer->getChildrenCount() > 0) {
            auto child = firstLayer->getChildByIndex(0);
            child->removeFromParentAndCleanup(false);
            child->setPosition(position);
            child->setScale(scale);
            node->addChild(child);
        }
        firstLayer->removeFromParentAndCleanup(false);
        node->addChild(firstLayer);
    }
}

void EditIconPopup::addPieceButton(std::string_view suffix, int page, CCArray* targets) {
    m_pieceDefinitions->setObject(CCFloat::create(0.0f), fmt::format("offsetX{}", suffix));
    m_pieceDefinitions->setObject(CCFloat::create(0.0f), fmt::format("offsetY{}", suffix));
    m_pieceDefinitions->setObject(CCFloat::create(0.0f), fmt::format("rotationX{}", suffix));
    m_pieceDefinitions->setObject(CCFloat::create(0.0f), fmt::format("rotationY{}", suffix));
    m_pieceDefinitions->setObject(CCFloat::create(1.0f), fmt::format("scaleX{}", suffix));
    m_pieceDefinitions->setObject(CCFloat::create(1.0f), fmt::format("scaleY{}", suffix));

    auto pieceFrame = MoreIconsAPI::getFrame(fmt::format("{}01{}.png", MoreIconsAPI::prefixes[(int)m_iconType], suffix));
    if (pieceFrame) m_frames->setObject(pieceFrame, fmt::format("{}.png", suffix));
    else pieceFrame = MoreIconsAPI::getFrame("GJ_deleteIcon_001.png");
    auto pieceSprite = CCSprite::createWithSpriteFrame(pieceFrame);
    auto pieceButton = CCMenuItemExt::createSpriteExtra(pieceSprite, [this, suffix, page](CCMenuItemSpriteExtra* sender) {
        m_suffix = suffix;
        m_offsetX = static_cast<CCFloat*>(m_pieceDefinitions->objectForKey(fmt::format("offsetX{}", suffix)))->getValue();
        m_offsetXSlider->setValue((m_offsetX + 20.0f) / 40.0f);
        m_offsetXLabel->setString(fmt::format("Offset X: {:.1f}", m_offsetX).c_str());
        m_offsetY = static_cast<CCFloat*>(m_pieceDefinitions->objectForKey(fmt::format("offsetY{}", suffix)))->getValue();
        m_offsetYSlider->setValue((m_offsetY + 20.0f) / 40.0f);
        m_offsetYLabel->setString(fmt::format("Offset Y: {:.1f}", m_offsetY).c_str());
        m_rotationX = static_cast<CCFloat*>(m_pieceDefinitions->objectForKey(fmt::format("rotationX{}", suffix)))->getValue();
        m_rotationXSlider->setValue(m_rotationX / 360.0f);
        m_rotationXLabel->setString(fmt::format("Rotation X: {:.0f}", m_rotationX).c_str());
        m_rotationY = static_cast<CCFloat*>(m_pieceDefinitions->objectForKey(fmt::format("rotationY{}", suffix)))->getValue();
        m_rotationYSlider->setValue(m_rotationY / 360.0f);
        m_rotationYLabel->setString(fmt::format("Rotation Y: {:.0f}", m_rotationY).c_str());
        m_scaleX = static_cast<CCFloat*>(m_pieceDefinitions->objectForKey(fmt::format("scaleX{}", suffix)))->getValue();
        m_scaleXSlider->setValue((m_scaleX + 10.0f) / 20.0f);
        m_scaleXLabel->setString(fmt::format("Scale X: {:.1f}", m_scaleX).c_str());
        m_scaleY = static_cast<CCFloat*>(m_pieceDefinitions->objectForKey(fmt::format("scaleY{}", suffix)))->getValue();
        m_scaleYSlider->setValue((m_scaleY + 10.0f) / 20.0f);
        m_scaleYLabel->setString(fmt::format("Scale Y: {:.1f}", m_scaleY).c_str());
        m_targets = static_cast<CCArray*>(sender->getUserObject("piece-targets"));
        m_descriptions = static_cast<CCArray*>(sender->getUserObject("piece-descriptions"));
        m_selectSprite->setTag(page);
        m_selectSprite->setVisible(true);
        m_selectSprite->setPosition(m_mainLayer->convertToNodeSpace(m_pieceMenu->convertToWorldSpace(sender->getPosition())));
    });
    pieceButton->setContentSize({ 30.0f, 30.0f });
    pieceSprite->setPosition({ 15.0f, 15.0f });

    if (targets) {
        pieceButton->setUserObject("piece-targets", targets);
        auto descriptions = CCArray::create();
        for (int i = 0; i < targets->count(); i++) {
            auto target = static_cast<CCSprite*>(targets->objectAtIndex(i));
            auto description = new SpriteDescription();
            description->m_position = target->getPosition();
            description->m_scale.x = target->getScaleX();
            description->m_scale.y = target->getScaleY();
            description->m_rotation = target->getRotation();
            description->autorelease();
            descriptions->addObject(description);
        }
        pieceButton->setUserObject("piece-descriptions", descriptions);
        if (m_suffix == suffix) {
            m_targets = targets;
            m_descriptions = descriptions;
        }
    }
    pieceButton->setID(fmt::format("piece-button{}", suffix).c_str());

    if (page == 0) m_pieceMenu->addChild(pieceButton);

    if (m_pieceArrays->count() <= page) m_pieceArrays->addObject(CCArray::create());
    static_cast<CCArray*>(m_pieceArrays->objectAtIndex(page))->addObject(pieceButton);
    m_pieces->setObject(pieceSprite, gd::string(suffix.data(), suffix.size()));
}

void EditIconPopup::updateWithSelectedFiles() {
    auto imageRes = texpack::fromPNG(m_selectedPNG);
    if (imageRes.isErr()) {
        notify(NotificationIcon::Error, "Failed to load image: {}", imageRes.unwrapErr());
        m_selectedPNG.clear();
        m_selectedPlist.clear();
        return;
    }

    auto image = std::move(imageRes).unwrap();

    Autorelease texture = new CCTexture2D();
    texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
        (float)image.width,
        (float)image.height
    });

    auto framesRes = MoreIconsAPI::createFrames(string::pathToString(m_selectedPlist), texture, "", m_iconType);
    if (framesRes.isErr()) {
        notify(NotificationIcon::Error, "Failed to load frames: {}", framesRes.unwrapErr());
        m_selectedPNG.clear();
        m_selectedPlist.clear();
        return;
    }

    auto frames = std::move(framesRes).unwrap();
    m_frames->removeAllObjects();
    for (auto [key, frame] : CCDictionaryExt<gd::string, CCSpriteFrame*>(frames)) {
        m_frames->setObject(frame, key);
    }
    updatePieces();
}

void EditIconPopup::updatePieces() {
    log::info("Updating pieces");
    auto crossFrame = MoreIconsAPI::getSpriteFrameCache()->spriteFrameByName("GJ_deleteIcon_001.png");
    for (auto [prefix, sprite] : CCDictionaryExt<std::string_view, CCSprite*>(m_pieces)) {
        auto spriteFrame = static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("{}.png", prefix)));
        sprite->setDisplayFrame(spriteFrame ? spriteFrame : crossFrame);
    }

    auto type = m_iconType;
    if (type == IconType::Robot || type == IconType::Spider) {
        auto sprite = type == IconType::Spider ? m_player->m_spiderSprite : m_player->m_robotSprite;
        auto spriteParts = sprite->m_paSprite->m_spriteParts;
        for (int i = 0; i < spriteParts->count(); i++) {
            auto spritePart = static_cast<CCSprite*>(spriteParts->objectAtIndex(i));
            auto tag = spritePart->getTag();

            spritePart->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_001.png", tag))));
            if (auto secondSprite = static_cast<CCSprite*>(sprite->m_secondArray->objectAtIndex(i))) {
                secondSprite->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_2_001.png", tag))));
            }

            if (auto glowChild = sprite->m_glowSprite->getChildByIndex<CCSprite>(i)) {
                glowChild->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_glow_001.png", tag))));
            }

            if (spritePart == sprite->m_headSprite) {
                auto extraFrame = static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_extra_001.png", tag)));
                if (extraFrame) {
                    if (sprite->m_extraSprite) sprite->m_extraSprite->setDisplayFrame(extraFrame);
                    else {
                        sprite->m_extraSprite = CCSprite::createWithSpriteFrame(extraFrame);
                        spritePart->addChild(sprite->m_extraSprite, 2);
                    }
                }
                sprite->m_extraSprite->setVisible(extraFrame != nullptr);
            }
        }
    }
    else {
        m_player->m_firstLayer->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey("_001.png")));
        m_player->m_secondLayer->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey("_2_001.png")));
        if (type == IconType::Ufo) {
            m_player->m_birdDome->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey("_3_001.png")));
        }
        m_player->m_outlineSprite->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey("_glow_001.png")));
        auto extraFrame = static_cast<CCSpriteFrame*>(m_frames->objectForKey("_extra_001.png"));
        m_player->m_detailSprite->setVisible(extraFrame != nullptr);
        if (extraFrame) {
            m_player->m_detailSprite->setDisplayFrame(extraFrame);
        }
    }

    m_hasChanged = true;
}

void EditIconPopup::goToPage(int page) {
    for (auto sprite : CCArrayExt<CCSprite*>(static_cast<CCArray*>(m_pieceArrays->objectAtIndex(m_page)))) {
        m_pieceMenu->removeChild(sprite, false);
    }

    auto count = m_pieceArrays->count();
    m_page = ((page % count) + count) % count;
    m_selectSprite->setVisible(m_selectSprite->getTag() == m_page);

    for (auto sprite : CCArrayExt<CCSprite*>(static_cast<CCArray*>(m_pieceArrays->objectAtIndex(m_page)))) {
        m_pieceMenu->addChild(sprite);
    }

    m_pieceMenu->updateLayout();
}

void EditIconPopup::updateTargets() {
    if (!m_targets || !m_descriptions) return;

    for (int i = 0; i < m_targets->count(); i++) {
        auto target = static_cast<CCSprite*>(m_targets->objectAtIndex(i));
        auto description = static_cast<SpriteDescription*>(m_descriptions->objectAtIndex(i));
        target->setPositionX(description->m_position.x + m_offsetX);
        target->setPositionY(description->m_position.y + m_offsetY);
        target->setRotationX(description->m_rotation + m_rotationX);
        target->setRotationY(description->m_rotation + m_rotationY);
        target->setScaleX(description->m_scale.x * m_scaleX);
        target->setScaleY(description->m_scale.y * m_scaleY);
    }
}

void EditIconPopup::onClose(CCObject* sender) {
    if (!m_hasChanged) return Popup::onClose(sender);

    auto type = (int)m_iconType;
    createQuickPopup(
        fmt::format("Exit {} Editor", MoreIconsAPI::uppercase[type]).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} editor</c>?\n<cr>All unsaved changes will be lost!</c>",
            MoreIconsAPI::lowercase[type]),
        "No",
        "Yes",
        [this](auto, bool btn2) {
            if (btn2) Popup::onClose(m_closeBtn);
        }
    );
}
