#include "BiggerContentLayer.hpp"
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

BiggerContentLayer* BiggerContentLayer::create(float width, float height, float sizeOffset) {
    auto ret = new BiggerContentLayer();
    if (ret->init(width, height, sizeOffset)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BiggerContentLayer::init(float width, float height, float sizeOffset) {
    if (!CCLayerColor::initWithColor({ 0, 0, 0, 0 }, width, height)) return false;

    m_sizeOffset = sizeOffset;
    setTouchEnabled(true);

    return true;
}

void BiggerContentLayer::setPosition(const CCPoint& pos) {
    CCNode::setPosition(pos);

    auto minY = -m_sizeOffset;
    auto maxY = m_sizeOffset;
    if (auto parent = getParent()) maxY += parent->getContentHeight();

    for (auto child : CCArrayExt<CCNode*>(m_pChildren)) {
        auto height = child->getScaledContentHeight();
        if (!child->isIgnoreAnchorPointForPosition()) height *= (1.0f - child->getAnchorPoint().y);
        auto childY = getPositionY() + child->getPositionY();
        auto scaleY = child->getScaleY();
        child->setVisible(scaleY * (childY + height) > minY && scaleY * (childY - height) < maxY);
    }
}

CCMenuItem* BiggerContentLayer::itemForTouch(CCTouch* touch) {
    auto touchLocation = touch->getLocation();
    for (auto child : CCArrayExt<CCMenuItem*>(m_pChildren)) {
        if (child->isVisible() && child->isEnabled() &&
            CCRect { { 0.0f, 0.0f }, child->getContentSize() }.containsPoint(child->convertToNodeSpace(touchLocation))) return child;
    }
    return nullptr;
}

bool BiggerContentLayer::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!nodeIsVisible(this) || !isTouchEnabled()) return false;

    m_selectedItem = itemForTouch(touch);
    if (m_selectedItem) {
        m_selectedItem->selected();
        return true;
    }
    return false;
}

void BiggerContentLayer::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    auto item = itemForTouch(touch);
    if (item != m_selectedItem) {
        if (m_selectedItem) m_selectedItem->unselected();
        m_selectedItem = item;
        if (m_selectedItem) m_selectedItem->selected();
    }
}

void BiggerContentLayer::ccTouchEnded(CCTouch* touch, CCEvent* event) {
    if (m_selectedItem) {
        m_selectedItem->unselected();
        m_selectedItem->activate();
        m_selectedItem = nullptr;
    }
}

void BiggerContentLayer::ccTouchCancelled(CCTouch* touch, CCEvent* event) {
    if (m_selectedItem) {
        m_selectedItem->unselected();
        m_selectedItem = nullptr;
    }
}
