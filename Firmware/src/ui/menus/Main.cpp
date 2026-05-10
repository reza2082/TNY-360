#include "ui/menus/Main.hpp"
#include "ui/Icons.hpp"

MenuMain::MenuMain(Menu* parent)
    : MenuList("Main menu", parent, Icons::MainMenu),
    menuNetwork(MenuNetwork(this)), menuTests(MenuTests(this)), menuCalibration(MenuCalibration(this)), menuSystem(MenuSystem(this))
{
    setItems({
        &menuNetwork,
        &menuTests,
        &menuCalibration,
        &menuSystem
    });
}
