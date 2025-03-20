#include "../MoreIcons.hpp"
#include <Geode/modify/LoadingLayer.hpp>

using namespace geode::prelude;

class $modify(MILoadingLayer, LoadingLayer) {
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

        if (f->m_iconLoadStep < 11) {
            f->m_iconLoadStep++;

            if (auto smallLabel2 = static_cast<CCLabelBMFont*>(getChildByID("geode-small-label-2"))) {
                auto label = "";
                switch (f->m_iconLoadStep) {
                    case 1: label = "More Icons: Loading Icons"; break;
                    case 2: label = "More Icons: Loading Ships"; break;
                    case 3: label = "More Icons: Loading Balls"; break;
                    case 4: label = "More Icons: Loading UFOs"; break;
                    case 5: label = "More Icons: Loading Waves"; break;
                    case 6: label = "More Icons: Loading Robots"; break;
                    case 7: label = "More Icons: Loading Spiders"; break;
                    case 8: label = "More Icons: Loading Swings"; break;
                    case 9: label = "More Icons: Loading Jetpacks"; break;
                    case 10: label = "More Icons: Loading Trails"; break;
                }
                smallLabel2->setString(label);
            }

            queueInMainThread([this] { loadAssets(); });
        }
    }
};
