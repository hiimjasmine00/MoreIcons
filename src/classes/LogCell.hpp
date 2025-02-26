#include "../MoreIcons.hpp"

class LogCell : public cocos2d::CCLayer {
protected:
    int m_index = 0;
    int m_total = 0;

    bool init(const LogData& data, int index, int total, bool dark);
public:
    static LogCell* create(const LogData& data, int index, int total, bool dark);
    void draw() override;
};
