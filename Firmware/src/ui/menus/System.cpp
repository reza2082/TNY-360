#include "ui/menus/System.hpp"
#include "ui/Icons.hpp"

MenuSystem::MenuSystem(Menu* parent)
    : MenuList("System", parent, Icons::SystemMenu),
    menuInfo(MenuInfo(this)), menuLogs(MenuLogs(this)), menuUpdate(MenuUpdate(this)), menuReboot(MenuReboot(this)), menuReset(MenuReset(this))
{
    setItems({
        &menuInfo,
        &menuLogs,
        &menuUpdate,
        &menuReboot,
        &menuReset
    });
}
