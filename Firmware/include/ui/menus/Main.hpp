#pragma once
#include "ui/Menus.hpp"
#include "ui/menus/List.hpp"
#include "ui/menus/Network.hpp"
#include "ui/menus/Tests.hpp"
#include "ui/menus/Calibration.hpp"
#include "ui/menus/System.hpp"

class MenuMain : public MenuList
{
public:
    MenuMain() = default;
    MenuMain(Menu* parent);
    virtual ~MenuMain() = default;

private:
    MenuNetwork menuNetwork;
    MenuTests menuTests;
    MenuCalibration menuCalibration;
    MenuSystem menuSystem;
};