#pragma once
#include "main.h"
#include "fan.hpp"
#include "humidifier.hpp"
#include "timeController.hpp"
#include "SHTC3.hpp"
#include "uart.h"

class Loop
{
private:
	enum class SysState : uint8_t
	{
		MONITORING,
		DECIDE,
		VENT,
		HUMIDIFY,
		SYS_ERROR
	};
	struct SHTPacket
	{
		uint16_t coldHumid, hotHumid;
		int16_t coldTemp, hotTemp;
	};

	// variables
	SysState state_ = SysState::MONITORING;
	HAL_StatusTypeDef shtStatus_ = HAL_OK;
	const uint16_t coldHumidMin = 600;
	const uint16_t hotHumidMin = 550;
	const uint16_t globalHumidMax = 750;
	const uint16_t hotTempMax = 310;
	const uint16_t hotTempMin = 265;
	uint8_t iotCommand = 0;

	// objects
	Fan* fan_;
	Humidifier* humidifier_;
	SHTC3* hotSHT_,* coldSHT_;
	SHTPacket shtPacket_;

	// functions
	HAL_StatusTypeDef updateSHTPacket();
	void decideNextAction();
	void isVentFinish();
public:
	Loop();
	void initialize(Fan* fan, Humidifier* humidifier, SHTC3* hotSHT, SHTC3* coldSHT);
	void fsmUpdate();
};