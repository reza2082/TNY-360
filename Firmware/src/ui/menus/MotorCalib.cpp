#include "ui/menus/MotorCalib.hpp"
#include "ui/Icons.hpp"
#include "common/Log.hpp"
#include "locomotion/Joint.hpp"

MenuMotorCalib::MenuMotorCalib(Menu* parent)
    : Menu("Motors", parent, Icons::MotorMenu)
{
}

bool MenuMotorCalib::onBack()
{
    return false;
}

bool MenuMotorCalib::onSelect()
{
    return false;
}

bool MenuMotorCalib::onNext()
{
    return false;
}

bool MenuMotorCalib::onPrev()
{
    return false;
}


void MenuMotorCalib::onShow()
{
    Joint* joint = Joint::GetJoint(Joint::Id::FrontRightKneePitch);
    if (joint) joint->getMotorController().startCalibration();
    else LOG_ERROR("MenuMotorCalib", "Joint not found");
}

void MenuMotorCalib::onHide()
{
    Joint* joint = Joint::GetJoint(Joint::Id::FrontRightKneePitch);
    if (joint && joint->getMotorController().getCalibrationState() == MotorController::CalibrationState::CALIBRATING) 
    {
        joint->getMotorController().stopCalibration();
    }
}

void MenuMotorCalib::onRender()
{
    renderHeader();
}

void MenuMotorCalib::onUpdate()
{
}
