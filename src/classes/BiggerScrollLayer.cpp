#include "BiggerScrollLayer.hpp"
#include "BiggerContentLayer.hpp"

using namespace geode::prelude;

BiggerScrollLayer::BiggerScrollLayer(float width, float height, float sizeOffset)
    : CCScrollLayerExt({ 0.0f, 0.0f, width, height }), m_sizeOffset(sizeOffset) {
    m_contentLayer->removeFromParent();
    m_contentLayer = BiggerContentLayer::create(width, height, sizeOffset);
    m_contentLayer->setAnchorPoint({ 0.0f, 0.0f });
    m_contentLayer->setID("content-layer");
    addChild(m_contentLayer);

    m_disableVertical = false;
    m_disableHorizontal = true;
    m_cutContent = true;

    setMouseEnabled(true);
}

BiggerScrollLayer* BiggerScrollLayer::create(float width, float height, float sizeOffset) {
    auto ret = new BiggerScrollLayer(width, height, sizeOffset);
    ret->autorelease();
    return ret;
}

bool BiggerScrollLayer::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    return m_contentLayer->ccTouchBegan(touch, event);
}

void BiggerScrollLayer::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    m_contentLayer->ccTouchMoved(touch, event);
}

void BiggerScrollLayer::ccTouchEnded(CCTouch* touch, CCEvent* event) {
    m_contentLayer->ccTouchEnded(touch, event);
}

void BiggerScrollLayer::ccTouchCancelled(CCTouch* touch, CCEvent* event) {
    m_contentLayer->ccTouchCancelled(touch, event);
}

void BiggerScrollLayer::scrollToTop() {
    m_contentLayer->setPositionY(getContentHeight() - m_contentLayer->getContentHeight());
}

void BiggerScrollLayer::scrollWheel(float y, float) {
    scrollLayer(y);
}

void BiggerScrollLayer::visit() {
    if (m_cutContent && isVisible()) {
        glEnable(GL_SCISSOR_TEST);
        if (getParent()) {
            auto bottomLeft = convertToWorldSpace({ 0.0f, -m_sizeOffset });
            auto size = convertToWorldSpace(getContentSize() + CCSize { 0.0f, m_sizeOffset }) - bottomLeft;
            CCEGLView::get()->setScissorInPoints(bottomLeft.x, bottomLeft.y, size.x, size.y);
        }
    }

    CCNode::visit();

    if (m_cutContent && isVisible()) glDisable(GL_SCISSOR_TEST);
}
