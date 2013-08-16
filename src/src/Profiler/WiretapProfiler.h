#pragma once
#ifndef __WIRETAP_PROFILER_H__
#define __WIRETAP_PROFILER_H__

#include <vector>
#include "WiretapUtils.h"

namespace Wiretap
{
	class ProfilePoint
	{
	public:
		ProfilePoint(char* name);
		~ProfilePoint();

	protected:
		double		m_TimeStart;
		std::string	m_Name;
	};

	class Profiler
	{
	public:
		~Profiler();
		static Profiler& GetInstance();
		void AddProfileEvent(const std::string& name, double time, EProfileEventType eventType);
		
		static void SendData(const char* viewerIPAddress = "127.0.0.1",  unsigned short viewerPort = 13001);
		static void DumpEvents();

	protected:
		Profiler();

		void _SendData(const char* viewerIPAddress = "127.0.0.1",  unsigned short viewerPort = 13001);
		void _DumpEvents();

	protected:
		std::vector<ProfileEvent>	m_Events;
		static Profiler*			ms_Instance;
	};
}

#define PROFILE_SCOPE(name) Wiretap::ProfilePoint cp##name(#name);

#endif //! __WIRETAP_PROFILER_H__