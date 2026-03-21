#pragma once
#include "main.h"
#include "fan.hpp"
#include "humidifier.hpp"
#include "SHTC3.hpp"
// #include "uart.h"

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
	SysState state_;
	HAL_StatusTypeDef shtStatus_;
	const uint16_t coldHumidMin_;
	const uint16_t hotHumidMin_;
	const uint16_t globalHumidMax_;
	const uint16_t hotTempMax_;
	const uint16_t hotTempMin_;
	uint8_t iotCommand_;

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