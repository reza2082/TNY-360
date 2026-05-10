#pragma once
#include "ui/Menus.hpp"
#include "locomotion/Body.hpp"

class MenuBootCalibration : public Menus::Menu
{
public:
    constexpr static const char* TAG = "MenuBootCalibration";

    MenuBootCalibration() = default;
    virtual ~MenuBootCalibration() = default;

    enum class Page: uint8_t
    {
        Intro,
        CalibMotor,
        CalibIMU,
        Done,
    };

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
    Page page = Page::Intro;
    Joint::Id jointId = (Joint::Id) 0;
    float progress = 0;
    MotorController::CalibrationState motorCalibState = MotorController::CalibrationState::UNCALIBRATED;
};