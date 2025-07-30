#include "BiggerScrollLayer.hpp"
#include "BiggerContentLayer.hpp"
#include "../../api/MoreIconsAPI.hpp"

using namespace geode::prelude;

int stencilBits = -1;

BiggerScrollLayer::BiggerScrollLayer(float width, float height, float cutOffset, float sizeOffset) : CCScrollLayerExt({
    0.0f, 0.0f, width, height
}) {
    auto background = CCScale9Sprite::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    background->setPosition({ width / 2.0f, height / 2.0f });
    background->setContentSize({ width, height + cutOffset * 2.0f });
    background->setOpacity(105);
    background->setID("background");
    addChild(background);

    m_stencil = CCScale9Sprite::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    m_stencil->setPosition(background->getPosition());
    m_stencil->setContentSize(background->getContentSize());

    static auto _ = [] {
        glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
        return true;
    }();

    m_contentLayer->removeFromParent();
    m_contentLayer = BiggerContentLayer::create(width, height, sizeOffset);
    m_contentLayer->setAnchorPoint({ 0.0f, 0.0f });
    m_contentLayer->setID("content-layer");
    addChild(m_contentLayer);

    m_disableVertical = false;
    m_disableHorizontal = true;
    m_cutContent = true;

    ignoreAnchorPointForPosition(false);
    setMouseEnabled(true);
}

BiggerScrollLayer* BiggerScrollLayer::create(float width, float height, float stencilOffset, float sizeOffset) {
    auto ret = new BiggerScrollLayer(width, height, stencilOffset, sizeOffset);
    ret->autorelease();
    return ret;
}

bool BiggerScrollLayer::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    return m_contentLayer->isTouchEnabled() ? m_contentLayer->ccTouchBegan(touch, event) : CCScrollLayerExt::ccTouchBegan(touch, event);
}

void BiggerScrollLayer::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    return m_contentLayer->isTouchEnabled() ? m_contentLayer->ccTouchMoved(touch, event) : CCScrollLayerExt::ccTouchMoved(touch, event);
}

void BiggerScrollLayer::ccTouchEnded(CCTouch* touch, CCEvent* event) {
    return m_contentLayer->isTouchEnabled() ? m_contentLayer->ccTouchEnded(touch, event) : CCScrollLayerExt::ccTouchEnded(touch, event);
}

void BiggerScrollLayer::ccTouchCancelled(CCTouch* touch, CCEvent* event) {
    return m_contentLayer->isTouchEnabled() ? m_contentLayer->ccTouchCancelled(touch, event) : CCScrollLayerExt::ccTouchCancelled(touch, event);
}

void BiggerScrollLayer::onEnter() {
    CCLayer::onEnter();
    m_stencil->onEnter();
}

void BiggerScrollLayer::onEnterTransitionDidFinish() {
    CCLayer::onEnterTransitionDidFinish();
    m_stencil->onEnterTransitionDidFinish();
}

void BiggerScrollLayer::onExit() {
    m_stencil->onExit();
    CCLayer::onExit();
}

void BiggerScrollLayer::onExitTransitionDidStart() {
    m_stencil->onExitTransitionDidStart();
    CCNode::onExitTransitionDidStart();
}

void BiggerScrollLayer::scrollToTop() {
    m_contentLayer->setPositionY(getContentHeight() - m_contentLayer->getContentHeight());
}

void BiggerScrollLayer::scrollWheel(float y, float) {
    scrollLayer(y);
}

void setProgram(CCNode* node, CCGLProgram* program) {
    if (node->getShaderProgram() != program) {
        node->setShaderProgram(program);
        for (auto child : CCArrayExt<CCNode*>(node->getChildren())) {
            setProgram(child, program);
        }
    }
}

void BiggerScrollLayer::visit() {
    if (stencilBits < 1 || !m_stencil || !m_stencil->isVisible() || !m_cutContent) return CCNode::visit();

    auto currentStencilEnabled = glIsEnabled(GL_STENCIL_TEST);

    auto currentStencilWriteMask = -1;
    glGetIntegerv(GL_STENCIL_WRITEMASK, &currentStencilWriteMask);

    auto currentStencilFunc = GL_ALWAYS;
    glGetIntegerv(GL_STENCIL_FUNC, &currentStencilFunc);

    auto currentStencilRef = 0;
    glGetIntegerv(GL_STENCIL_REF, &currentStencilRef);

    auto currentStencilValueMask = -1;
    glGetIntegerv(GL_STENCIL_VALUE_MASK, &currentStencilValueMask);

    auto currentStencilFail = GL_KEEP;
    glGetIntegerv(GL_STENCIL_FAIL, &currentStencilFail);

    auto currentStencilPassDepthFail = GL_KEEP;
    glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &currentStencilPassDepthFail);

    auto currentStencilPassDepthPass = GL_KEEP;
    glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &currentStencilPassDepthPass);

    glEnable(GL_STENCIL_TEST);
    glStencilMask(1);
    glClear(GL_STENCIL_BUFFER_BIT);

    uint8_t currentDepthWriteMask = GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &currentDepthWriteMask);
    glDepthMask(GL_FALSE);

    glStencilFunc(GL_NEVER, 1, 1);
    glStencilOp(GL_ZERO, GL_KEEP, GL_KEEP);
    ccDrawSolidRect({ 0.0f, 0.0f }, MoreIconsAPI::get<CCDirector>()->getWinSize(), { 1.0f, 1.0f, 1.0f, 1.0f });
    glStencilFunc(GL_NEVER, 1, 1);
    glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

    #ifdef GEODE_IS_DESKTOP
    auto currentAlphaTestEnabled = glIsEnabled(GL_ALPHA_TEST);

    auto currentAlphaTestFunc = GL_ALWAYS;
    glGetIntegerv(GL_ALPHA_TEST_FUNC, &currentAlphaTestFunc);

    auto currentAlphaTestRef = 1.0f;
    glGetFloatv(GL_ALPHA_TEST_REF, &currentAlphaTestRef);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.05f);
    #else
    auto program = MoreIconsAPI::get<CCShaderCache>()->programForKey("ShaderPositionTextureColorAlphaTest");
    auto alphaValueLocation = glGetUniformLocation(program->getProgram(), "CC_alpha_value");
    program->use();
    program->setUniformLocationWith1f(alphaValueLocation, 0.05f);
    setProgram(m_stencil, program);
    #endif

    kmGLPushMatrix();
    transform();
    m_stencil->visit();
    kmGLPopMatrix();

    #ifdef GEODE_IS_DESKTOP
    glAlphaFunc(currentAlphaTestFunc, currentAlphaTestRef);
    if (!currentAlphaTestEnabled) glDisable(GL_ALPHA_TEST);
    #endif

    glDepthMask(currentDepthWriteMask);

    glStencilFunc(GL_EQUAL, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    CCNode::visit();

    glStencilFunc(currentStencilFunc, currentStencilRef, currentStencilValueMask);
    glStencilOp(currentStencilFail, currentStencilPassDepthFail, currentStencilPassDepthPass);
    glStencilMask(currentStencilWriteMask);
    if (!currentStencilEnabled) glDisable(GL_STENCIL_TEST);
}
