#pragma once
#include "ui/Menus.hpp"

class MenuReboot : public Menus::Menu
{
public:
    MenuReboot() = default;
    MenuReboot(Menu* parent);
    virtual ~MenuReboot() = default;

protected:
    virtual bool onBack() override;
    virtual bool onSelect() override;
    virtual bool onNext() override;
    virtual bool onPrev() override;

    virtual void onShow() override;
    virtual void onHide() override;
    virtual void onRender() override;
    virtual void onUpdate() override;

private:
    bool isRebooting = false;
};