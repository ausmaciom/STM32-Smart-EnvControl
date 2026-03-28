#include "Loop.hpp"

Loop::Loop() :
	state_(SysState::MONITORING), shtStatus_(HAL_OK), iotCommand_(0),
	fan_(nullptr), humidifier_(nullptr), hotSHT_(nullptr), coldSHT_(nullptr),
	coldHumidMin_(600), hotHumidMin_(550), globalHumidMax_(750), hotTempMax_(310),
	hotTempMin_(265)
{
}

void Loop::initialize(Fan* fan, Humidifier* humidifier, SHTC3* hotSHT, SHTC3* coldSHT)
{
	fan_ = fan;
	humidifier_ = humidifier;
	hotSHT_ = hotSHT;
	coldSHT_ = coldSHT;
}

HAL_StatusTypeDef Loop::updateSHTPacket()
{
	HAL_StatusTypeDef res;

	res = coldSHT_->readTempHumidity();
	if (res != HAL_OK) return res;

	res = hotSHT_->readTempHumidity();
	if (res != HAL_OK) return res;

	shtPacket_.coldHumid = coldSHT_->getHumidity();
	shtPacket_.coldTemp = coldSHT_->getTemperature();
	shtPacket_.hotHumid = hotSHT_->getHumidity();
	shtPacket_.hotTemp = hotSHT_->getTemperature();
	return HAL_OK;
}

void Loop::decideNextAction()
{
	bool needsHumidify = (shtPacket_.coldHumid <= coldHumidMin_) ||
		(shtPacket_.hotHumid <= hotHumidMin_) ||
		(iotCommand_ == 1);
	
	if (needsHumidify && !humidifier_->isCooldown())
	{
		humidifier_->turnON();
		state_ = SysState::HUMIDIFY;
		iotCommand_ = 0;
		return;
	}

	// 3 priorities
	bool autoRequest = (shtPacket_.coldHumid > globalHumidMax_) ||
		(shtPacket_.hotHumid > globalHumidMax_) ||
		(shtPacket_.hotTemp > hotTempMax_);
	bool isSafeToVent = shtPacket_.hotTemp >= hotTempMin_;
	bool needsVent = (iotCommand_ == 2) || (autoRequest && isSafeToVent);

	if (needsVent)
	{
		fan_->turnON();
		state_ = SysState::VENT;
		iotCommand_ = 0;
	}
}

void Loop::isVentFinish()
{
	shtStatus_ = updateSHTPacket();
	if (shtStatus_ != HAL_OK)
	{
		state_ = SysState::SYS_ERROR;
		iotCommand_ = 0;
		fan_->turnOFF();
		return;
	}
	
    // autoFinish : threshold temp: 1.5°C humid: 5.0%
    bool autoFinish = (shtPacket_.coldHumid < (globalHumidMax_ - 50)) &&
		(shtPacket_.hotHumid < (globalHumidMax_ - 50)) &&
		(shtPacket_.hotTemp < (hotTempMax_ - 15));
	bool timeout = fan_->isTimeout();
	bool stopVent = (iotCommand_ == 3) || autoFinish || timeout;

	if (stopVent)
	{
		fan_->turnOFF();
		state_ = SysState::MONITORING;
		iotCommand_ = 0;
	}
}

void Loop::fsmUpdate()
{
	switch (state_)
	{
	case SysState::MONITORING:
		shtStatus_ = updateSHTPacket();
		if (shtStatus_ != HAL_OK)
		{
			state_ = SysState::SYS_ERROR;
		}
		else
		{
			state_ = SysState::DECIDE;
		}
		break;
	case SysState::DECIDE:
		decideNextAction();
		break;
	case SysState::VENT:
		isVentFinish();
		break;
	case SysState::HUMIDIFY:
		if (!humidifier_->humidify())
		{
			humidifier_->turnOFF();
			state_ = SysState::MONITORING;
		}
		break;
	case SysState::SYS_ERROR:
		if (shtStatus_ == HAL_ERROR)
			Error_Handler();
		else
			state_ = SysState::MONITORING;
		break;
	default:
		state_ = SysState::MONITORING;
		break;
	}
}