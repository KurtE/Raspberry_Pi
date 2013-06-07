/*
 * Motor.h
 * Some helper libraries to use
 *
 *  Created on: May 27, 2013
 *      Author: Saad Ahmad ( http://www.saadahmad.ca )
 */

#ifndef MOTOR_H_
#define MOTOR_H_

// #include "Globals.h"
#include <string>
#include "PWM.h"

// This is so that we get smooth transitions between different PWM levels.
// Useful for motors as a sudden voltage change can cause kick back and producing undesirable results
const int SPEED_STEP_VALUE = 100;

// A motor esc controller library
class MotorControl
{
	PWM::Pin m_pin;
	int m_minPWM;
	int m_maxPWM;
	int m_currentPWM;
	int m_targetPWM;
	float m_minValue;
	float m_maxValue;
public:
	PWM::Pin &ModifyPWMPin()
	{
		return m_pin;
	}
	const int &GetMinPWM() const
	{
		return m_minPWM;
	}
	void SetMinPWM(const int & minPWM)
	{
		m_minPWM = minPWM;
	}
	const int &GetMaxPWM() const
	{
		return m_maxPWM;
	}
	void SetMaxPWM(const int & maxPWM)
	{
		m_maxPWM = maxPWM;
	}
	const int &GetTargetPWM() const
	{
		return m_targetPWM;
	}
	void SetTargetPWM(const int & targetPWM)
	{
		m_targetPWM = targetPWM;
	}
	const int &GetCurrentPWM() const
	{
		return m_currentPWM;
	}
	void SetCurrentPWM(const int & currentPWM)
	{
		m_currentPWM = currentPWM;
	}
	const float &GetMinValue() const
	{
		return m_minValue;
	}
	void SetMinValue(const float & minValue)
	{
		m_minValue = minValue;
	}
	const float &GetMaxValue() const
	{
		return m_maxValue;
	}
	void SetMaxValue(const float & maxValue)
	{
		m_maxValue = maxValue;
	}

	MotorControl(const std::string & pinName, const float & currentValue, const float & minValue, const float & maxValue, const int & minPWM = 1000, const int & maxPWM = 2000) :
		m_pin(pinName, 20 * MILLISECONDS_TO_NANOSECONDS, 1 * MILLISECONDS_TO_NANOSECONDS)
	{
		SetMinPWM(minPWM);
		SetMaxPWM(maxPWM);
		SetMinValue(minValue);
		SetMaxValue(maxValue);
		SetOutputValue(currentValue);
		SetCurrentPWM(GetTargetPWM());
	}
	void SetOutputPercent(const float & percent)
	{
		int newPWM = int(GetMinPWM() + percent * (GetMaxPWM() - GetMinPWM()));
#if 0
		SetTargetPWM(Clamp(newPWM, GetMinPWM(), GetMaxPWM()));
#else
		SetTargetPWM(newPWM);
#endif
	}
	virtual void SetOutputValue(const float & value)
	{
		SetOutputPercent((value - GetMinValue()) / (GetMaxValue() - GetMinValue()));
	}
	void UpdatePWMSignal()
	{
		UpdatePWMOutput();
		// Write out to PWM
		ModifyPWMPin().SetDutyUS(GetCurrentPWM());
	}
	void Enable()
	{
		ModifyPWMPin().Enable();
	}
private:
	void UpdatePWMOutput()
	{
		if (m_currentPWM < m_targetPWM && (m_targetPWM - m_currentPWM) >= SPEED_STEP_VALUE)
		{
			m_currentPWM += SPEED_STEP_VALUE;
		}
		else if (m_currentPWM > m_targetPWM && (m_currentPWM - m_targetPWM) >= SPEED_STEP_VALUE)
		{
			m_currentPWM -= SPEED_STEP_VALUE;
		}
		else
		{
			m_currentPWM = m_targetPWM;
		}
	}
};

// A servo controller library
class ServoControl: public MotorControl
{
	float m_centerValue;
public:
	const float &GetCenterValue() const
	{
		return m_centerValue;
	}
	void SetCenterValue(const float & centerValue)
	{
		m_centerValue = centerValue;
	}

	ServoControl(const std::string & pinName, const float & centerValue, const float & minValue, const float & maxValue, const int & minPWM = 1000, const int & maxPWM = 2000) :
		MotorControl(pinName, centerValue, minValue, maxValue, minPWM, maxPWM)
	{
		SetCenterValue(centerValue);
	}
	void SetAngleRelativeToCenter(const float & offset)
	{
		SetOutputValue(GetCenterValue() + offset);
	}
	void SetAngle(const float & angle)
	{
		SetOutputValue(angle);
	}
};

#endif /* MOTOR_H_ */
