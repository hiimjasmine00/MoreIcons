#include <cocos2d.h>
#include <Geode/loader/Types.hpp>

class LogCell : public cocos2d::CCLayer {
protected:
    int m_index = 0;
    int m_total = 0;

    bool init(const std::string& message, geode::Severity severity, int index, int total, bool dark);
public:
    static LogCell* create(const std::string& message, geode::Severity severity, int index, int total, bool dark);
    void draw() override;
};
