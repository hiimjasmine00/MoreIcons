#include "DummyNode.hpp"
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

void DummyNode::setBlendFunc(ccBlendFunc blendFunc) {
    CCSpriteBatchNode::setBlendFunc(blendFunc);

    for (auto child : CCArrayExt<CCNode*>(getChildren())) {
        if (typeinfo_cast<CCBlendProtocol*>(child)) recursiveBlend(child, blendFunc);
    }
}

void DummyNode::recursiveBlend(CCNode* node, ccBlendFunc blendFunc) {
    if (!node) return;

    if (auto blendNode = typeinfo_cast<CCBlendProtocol*>(node)) blendNode->setBlendFunc(blendFunc);

    for (auto child : CCArrayExt<CCNode*>(node->getChildren())) {
        if (typeinfo_cast<CCBlendProtocol*>(child)) recursiveBlend(child, blendFunc);
    }
}
