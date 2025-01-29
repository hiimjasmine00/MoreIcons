#include <cocos2d.h>
#include <Geode/Enums.hpp>
#include <Geode/GeneratedPredeclare.hpp>

// https://github.com/Alphalaneous/FineOutline/blob/1.0.8/src/CCSpriteBatchNode.h#L24 probably
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

class MoreIconsAPI {
public:
    static inline std::vector<std::string> ICONS;
    static inline std::vector<std::string> SHIPS;
    static inline std::vector<std::string> BALLS;
    static inline std::vector<std::string> UFOS;
    static inline std::vector<std::string> WAVES;
    static inline std::vector<std::string> ROBOTS;
    static inline std::vector<std::string> SPIDERS;
    static inline std::vector<std::string> SWINGS;
    static inline std::vector<std::string> JETPACKS;
    static inline std::vector<std::string> TRAILS;

    static bool doesExist(cocos2d::CCSpriteFrame* frame) {
        return frame && frame->getTag() != 105871529;
    }

    static void setUserObject(cocos2d::CCNode* node, const std::string& value);
    static void removeUserObject(cocos2d::CCNode* node);
    static std::vector<std::string>& vectorForType(IconType type);
    static std::string_view savedForType(IconType type, bool dual);
    static std::string activeForType(IconType type, bool dual);
    static void setIcon(const std::string& icon, IconType type, bool dual);
    static bool hasIcon(const std::string& icon, IconType type);
    static void updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type);
    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon);
    static void updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type);
    static IconType getIconType(PlayerObject* object);
    static void updatePlayerObject(PlayerObject* object, const std::string& icon);
    static void updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type);
};
