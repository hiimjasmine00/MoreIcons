#include <cocos2d.h>

class LogCell : public cocos2d::CCLayer {
protected:
    bool init(const char*, std::string_view, int, int);
public:
    static LogCell* create(const char*, std::string_view, int, int);

    void draw() override;
};
