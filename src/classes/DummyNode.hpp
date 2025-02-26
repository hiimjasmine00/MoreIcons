#include <cocos2d.h>

// Adapted from Alphalaneous https://github.com/Alphalaneous/FineOutline/blob/1.0.8/src/CCSpriteBatchNode.h#L24
class DummyNode : public cocos2d::CCSpriteBatchNode {
public:
    static DummyNode* createWithTexture(cocos2d::CCTexture2D* texture, unsigned int capacity) {
        auto ret = new DummyNode();
        ret->initWithTexture(texture, capacity);
        ret->autorelease();
        return ret;
    }

    void draw() override {
        CCNode::draw();
    }

    void reorderChild(CCNode* child, int zOrder) override {
        CCNode::reorderChild(child, zOrder);
    }

    void removeChild(CCNode* child, bool cleanup) override {
        CCNode::removeChild(child, cleanup);
    }

    void removeAllChildrenWithCleanup(bool cleanup) override {
        CCNode::removeAllChildrenWithCleanup(cleanup);
    }

    void addChild(CCNode* child, int zOrder, int tag) override {
        if (geode::cast::typeinfo_cast<CCBlendProtocol*>(child)) recursiveBlend(child, getBlendFunc());

        CCNode::addChild(child, zOrder, tag);
    }

    void sortAllChildren() override {
        CCNode::sortAllChildren();
    }

    void visit() override {
        CCNode::visit();
    }

    void setBlendFunc(cocos2d::ccBlendFunc blendFunc) override;

    void recursiveBlend(CCNode* node, cocos2d::ccBlendFunc blendFunc);
};
