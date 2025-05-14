#include <cocos2d.h>

class LogCell : public cocos2d::CCLayer {
protected:
    bool init(const std::string&, const std::string&, int, int);
public:
    static LogCell* create(const std::string&, const std::string&, int, int);

    void draw() override;
};
