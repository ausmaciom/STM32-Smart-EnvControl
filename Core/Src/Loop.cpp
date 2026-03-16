#include "Loop.hpp"

Loop::Loop() :
	state_(SysState::MONITORING), shtStatus_(HAL_OK), iotCommand(0)
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
	bool needsHumidify = (shtPacket_.coldHumid <= coldHumidMin) ||
		(shtPacket_.hotHumid <= hotHumidMin) ||
		(iotCommand == 1);
	
	if (needsHumidify && !humidifier_->isCooldown())
	{
		humidifier_->turnON();
		state_ = SysState::HUMIDIFY;
		iotCommand = 0;
		return;
	}

	// 3 priorities
	bool autoRequest = (shtPacket_.coldHumid > globalHumidMax) ||
		(shtPacket_.hotHumid > globalHumidMax) ||
		(shtPacket_.hotTemp > hotTempMax);
	bool isSafeToVent = shtPacket_.hotTemp >= hotTempMin;
	bool needsVent = (iotCommand == 2) || (autoRequest && isSafeToVent);

	if (needsVent)
	{
		fan_->turnON();
		state_ = SysState::VENT;
		iotCommand = 0;
	}
}

void Loop::isVentFinish()
{
	shtStatus_ = updateSHTPacket();
	if (shtStatus_ != HAL_OK)
	{
		state_ = SysState::SYS_ERROR;
		return;
	}
	bool autoFinish = (shtPacket_.coldHumid < (globalHumidMax - 50)) &&
		(shtPacket_.hotHumid < (globalHumidMax - 50)) &&
		(shtPacket_.hotTemp < (hotTempMax - 15));
	bool timeout = fan_->isTimeout();
	bool stopVent = (iotCommand == 3) || autoFinish || timeout;

	if (stopVent)
	{
		fan_->turnOFF();
		state_ = SysState::MONITORING;
		iotCommand = 0;
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
		break;
	}
}