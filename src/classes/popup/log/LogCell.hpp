#include <cocos2d.h>
#include <Geode/loader/Types.hpp>

class LogCell : public cocos2d::CCLayer {
protected:
    bool init(std::string_view name, std::string_view message, geode::Severity severity, bool light);
public:
    static LogCell* create(std::string_view name, std::string_view message, geode::Severity severity, bool light);

    void draw() override;
};
