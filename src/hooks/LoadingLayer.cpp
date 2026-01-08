#include "../MoreIcons.hpp"
#include "../utils/Icons.hpp"
#include <Geode/modify/LoadingLayer.hpp>

using namespace geode::prelude;

class $modify(MILoadingLayer, LoadingLayer) {
    struct Fields {
        int m_iconLoadStep = -1;
    };

    static void onModify(ModifyBase<ModifyDerive<MILoadingLayer, LoadingLayer>>& self) {
        (void)self.setHookPriorityAfterPre("LoadingLayer::loadAssets", "geode.loader");
    }

    void loadAssets() {
        auto f = m_fields.self();
        if (f->m_iconLoadStep > 12) return LoadingLayer::loadAssets();

        auto smallLabel2 = static_cast<CCLabelBMFont*>(getChildByID("geode-small-label-2"));
        switch (f->m_iconLoadStep) {
            case -1:
                if (smallLabel2) smallLabel2->setString("More Icons: Loading Packs");
                break;
            case 0:
                Icons::loadPacks();
                if (smallLabel2) smallLabel2->setString("More Icons: Loading Icons");
                break;
            case 1:
                Icons::loadIcons(IconType::Cube);
                MoreIcons::loadFromSave(IconType::Cube);
                if (smallLabel2) smallLabel2->setString("More Icons: Loading Ships");
                break;
            case 2:
                Icons::loadIcons(IconType::Ship);
                MoreIcons::loadFromSave(IconType::Ship);
                if (smallLabel2) smallLabel2->setString("More Icons: Loading Balls");
                break;
            case 3:
                Icons::loadIcons(IconType::Ball);
                MoreIcons::loadFromSave(IconType::Ball);
                if (smallLabel2) smallLabel2->setString("More Icons: Loading UFOs");
                break;
            case 4:
                Icons::loadIcons(IconType::Ufo);
                MoreIcons::loadFromSave(IconType::Ufo);
                if (smallLabel2) smallLabel2->setString("More Icons: Loading Waves");
                break;
            case 5:
                Icons::loadIcons(IconType::Wave);
                MoreIcons::loadFromSave(IconType::Wave);
                if (smallLabel2) smallLabel2->setString("More Icons: Loading Robots");
                break;
            case 6:
                Icons::loadIcons(IconType::Robot);
                MoreIcons::loadFromSave(IconType::Robot);
                if (smallLabel2) smallLabel2->setString("More Icons: Loading Spiders");
                break;
            case 7:
                Icons::loadIcons(IconType::Spider);
                MoreIcons::loadFromSave(IconType::Spider);
                if (smallLabel2) smallLabel2->setString("More Icons: Loading Swings");
                break;
            case 8:
                Icons::loadIcons(IconType::Swing);
                MoreIcons::loadFromSave(IconType::Swing);
                if (smallLabel2) smallLabel2->setString("More Icons: Loading Jetpacks");
                break;
            case 9:
                Icons::loadIcons(IconType::Jetpack);
                MoreIcons::loadFromSave(IconType::Jetpack);
                if (smallLabel2) smallLabel2->setString("More Icons: Loading Trails");
                break;
            case 10:
                Icons::loadIcons(IconType::Special);
                MoreIcons::loadFromSave(IconType::Special);
                if (smallLabel2) smallLabel2->setString("More Icons: Loading Death Effects");
                break;
            case 11:
                Icons::loadIcons(IconType::DeathEffect);
                MoreIcons::loadFromSave(IconType::DeathEffect);
                if (smallLabel2) smallLabel2->setString("More Icons: Loading Ship Fires");
                break;
            case 12:
                Icons::loadIcons(IconType::ShipFire);
                MoreIcons::loadFromSave(IconType::ShipFire);
                Icons::finishLoading();
                if (smallLabel2) smallLabel2->setString("");
                break;
            default:
                return LoadingLayer::loadAssets();
        }

        f->m_iconLoadStep++;
        queueInMainThread([this] {
            loadAssets();
        });
    }
};
