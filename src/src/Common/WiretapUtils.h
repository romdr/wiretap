#pragma once
#ifndef __WIRETAP_UTILS_H__
#define __WIRETAP_UTILS_H__

#include <vector>

namespace Wiretap
{
	enum EProfileEventType
	{
		ProfileEvent_Start,
		ProfileEvent_Stop
	};

	class ostringstream;

	class ProfileEvent
	{
	public:
		ProfileEvent(const std::string& name, double time, EProfileEventType eventType)
			: m_Name(name), m_Time(time), m_Type(eventType), m_Duration(0.0)
		{}
		const char* GetName() const { return m_Name.c_str(); }
		double GetTime() const { return m_Time; }
		EProfileEventType GetType() const { return m_Type; }

		double GetDuration() const { return m_Duration; }
		void SetDuration(double duration) { m_Duration = duration; }

	protected: // Logged
		double				m_Time;
		std::string			m_Name;
		EProfileEventType	m_Type;

	protected: // Generated
		double				m_Duration;
	};

	std::string GetIndent(unsigned int indentationLevel);
	double ParseEvents(std::vector<ProfileEvent>& events);
	void DumpEvents(const std::vector<ProfileEvent>& events, std::ostringstream* outStr = NULL);
	double GetHiResTime();
}

#endif //! __WIRETAP_UTILS_H__