#include "../MoreIcons.hpp"
#include <Geode/modify/LoadingLayer.hpp>

using namespace geode::prelude;

class $modify(MILoadingLayer, LoadingLayer) {
    inline static constexpr std::array labels = {
        "More Icons: Loading Icons", "More Icons: Loading Ships",
        "More Icons: Loading Balls", "More Icons: Loading UFOs",
        "More Icons: Loading Waves", "More Icons: Loading Robots",
        "More Icons: Loading Spiders", "More Icons: Loading Swings",
        "More Icons: Loading Jetpacks", "More Icons: Loading Trails"
    };

    struct Fields {
        int m_iconLoadStep;
        std::vector<IconPack> m_iconPacks;
    };

    static void onModify(ModifyBase<ModifyDerive<MILoadingLayer, LoadingLayer>>& self) {
        (void)self.setHookPriorityAfterPre("LoadingLayer::loadAssets", "geode.loader");
    }

    void loadAssets() {
        if (Loader::get()->isModLoaded("dankmeme.blaze") ? m_loadStep < 14 : m_loadStep > 0) return LoadingLayer::loadAssets();

        auto f = m_fields.self();
        switch (f->m_iconLoadStep) {
            case 0:
                f->m_iconPacks = MoreIcons::getTexturePacks();
                break;
            case 1:
                MoreIcons::loadIcons(f->m_iconPacks, "icon", IconType::Cube);
                break;
            case 2:
                MoreIcons::loadIcons(f->m_iconPacks, "ship", IconType::Ship);
                break;
            case 3:
                MoreIcons::loadIcons(f->m_iconPacks, "ball", IconType::Ball);
                break;
            case 4:
                MoreIcons::loadIcons(f->m_iconPacks, "ufo", IconType::Ufo);
                break;
            case 5:
                MoreIcons::loadIcons(f->m_iconPacks, "wave", IconType::Wave);
                break;
            case 6:
                MoreIcons::loadIcons(f->m_iconPacks, "robot", IconType::Robot);
                break;
            case 7:
                MoreIcons::loadIcons(f->m_iconPacks, "spider", IconType::Spider);
                break;
            case 8:
                MoreIcons::loadIcons(f->m_iconPacks, "swing", IconType::Swing);
                break;
            case 9:
                MoreIcons::loadIcons(f->m_iconPacks, "jetpack", IconType::Jetpack);
                break;
            case 10:
                MoreIcons::loadTrails(f->m_iconPacks);
                break;
            default:
                return LoadingLayer::loadAssets();
        }

        if (auto smallLabel2 = static_cast<CCLabelBMFont*>(getChildByID("geode-small-label-2")))
            smallLabel2->setString(f->m_iconLoadStep < labels.size() ? labels[f->m_iconLoadStep] : "");

        f->m_iconLoadStep++;
        queueInMainThread([this] { loadAssets(); });
    }
};
