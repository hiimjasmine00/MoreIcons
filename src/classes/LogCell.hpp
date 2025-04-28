#include <cocos2d.h>

class LogCell : public cocos2d::CCLayer {
protected:
    int m_index = 0;
    int m_total = 0;

    bool init(const std::string& message, int severity, int index, int total, bool dark);
public:
    inline static constexpr std::array frames = { "GJ_infoIcon_001.png", "geode.loader/info-warning.png", "geode.loader/info-alert.png" };

    static LogCell* create(const std::string& message, int severity, int index, int total, bool dark);

    void draw() override;
};
