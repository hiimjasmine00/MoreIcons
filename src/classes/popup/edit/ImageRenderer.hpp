#include <cocos2d.h>
#include <texpack.hpp>

class ImageRenderer {
public:
    static texpack::Image getImage(cocos2d::CCNode* node);
};
