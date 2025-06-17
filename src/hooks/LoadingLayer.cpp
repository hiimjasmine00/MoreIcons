#include "../MoreIcons.hpp"
#include <Geode/modify/LoadingLayer.hpp>

using namespace geode::prelude;

class $modify(MILoadingLayer, LoadingLayer) {
    struct Fields {
        int m_iconLoadStep = 0;
        bool m_finishedLoading = false;
    };

    static void onModify(ModifyBase<ModifyDerive<MILoadingLayer, LoadingLayer>>& self) {
        (void)self.setHookPriorityAfterPre("LoadingLayer::loadAssets", "geode.loader");
    }

    void loadAssets() {
        auto f = m_fields.self();
        if (f->m_finishedLoading) {
            MoreIcons::packs.clear();
            return LoadingLayer::loadAssets();
        }

        switch (f->m_iconLoadStep) {
            case 0:
                MoreIcons::loadPacks();
                break;
            case 1:
                MoreIcons::loadIcons(IconType::Cube);
                break;
            case 2:
                MoreIcons::loadIcons(IconType::Ship);
                break;
            case 3:
                MoreIcons::loadIcons(IconType::Ball);
                break;
            case 4:
                MoreIcons::loadIcons(IconType::Ufo);
                break;
            case 5:
                MoreIcons::loadIcons(IconType::Wave);
                break;
            case 6:
                MoreIcons::loadIcons(IconType::Robot);
                break;
            case 7:
                MoreIcons::loadIcons(IconType::Spider);
                break;
            case 8:
                MoreIcons::loadIcons(IconType::Swing);
                break;
            case 9:
                MoreIcons::loadIcons(IconType::Jetpack);
                break;
            case 10:
                MoreIcons::loadIcons(IconType::Special);
                break;
            default:
                return LoadingLayer::loadAssets();
        }

        constexpr std::array labels = {
            "More Icons: Loading Icons", "More Icons: Loading Ships",
            "More Icons: Loading Balls", "More Icons: Loading UFOs",
            "More Icons: Loading Waves", "More Icons: Loading Robots",
            "More Icons: Loading Spiders", "More Icons: Loading Swings",
            "More Icons: Loading Jetpacks", "More Icons: Loading Trails"
        };

        if (auto smallLabel2 = static_cast<CCLabelBMFont*>(getChildByID("geode-small-label-2")))
            smallLabel2->setString(f->m_iconLoadStep < labels.size() ? labels[f->m_iconLoadStep] : "");

        f->m_iconLoadStep++;
        f->m_finishedLoading = f->m_iconLoadStep > labels.size();
        queueInMainThread([this] { loadAssets(); });
    }
};
