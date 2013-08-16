#include <stdio.h>
#include <SFML/Network.hpp>
#include "WiretapProfiler.h"
#include "WiretapUtils.h"

using namespace Wiretap;

/*
** Profile point used to profile scopes
*/

ProfilePoint::ProfilePoint(char* name)
: m_Name(name)
, m_TimeStart(GetHiResTime())
{
	Profiler::GetInstance().AddProfileEvent(m_Name, m_TimeStart, ProfileEvent_Start);
}

ProfilePoint::~ProfilePoint()
{
	Profiler::GetInstance().AddProfileEvent(m_Name, GetHiResTime(), ProfileEvent_Stop);
}


/*
** Profiler recording events coming from profile points and dumping results
*/

Profiler* Profiler::ms_Instance = NULL;

Profiler::Profiler()
{
}

Profiler::~Profiler()
{
}

Profiler& Profiler::GetInstance()
{
	if (!ms_Instance)
		ms_Instance = new Profiler();
	return *ms_Instance;
}

void Profiler::AddProfileEvent(const std::string& name, double time, EProfileEventType eventType)
{
	m_Events.push_back(ProfileEvent(name, time, eventType));
}

// Sends this frame's events and clear them
void Profiler::SendData(const char* viewerIPAddress /* = "127.0.0.1" */,  unsigned short viewerPort /* = 13001 */)
{
	GetInstance()._SendData(viewerIPAddress, viewerPort);
}

void Profiler::_SendData(const char* viewerIPAddress /* = "127.0.0.1" */,  unsigned short viewerPort /* = 13001 */)
{
	static sf::TcpSocket socket;
	static sf::Socket::Status socketStatus = sf::Socket::Error;

	if (socketStatus != sf::Socket::Done)
	{
		socketStatus = socket.connect(viewerIPAddress, viewerPort);
	}
	if (socketStatus == sf::Socket::Done)
	{
		for (unsigned int i = 0; i < m_Events.size(); i++)
		{
			const ProfileEvent& profileEvent = m_Events[i];
			sf::Packet packet;
			packet << profileEvent.GetTime() << profileEvent.GetName() << (unsigned int)profileEvent.GetType();
			socket.send(packet);
		}

		m_Events.clear();
	}
}

// Parses and prints the events (make sure to call this before SendData which clears them all)
void Profiler::DumpEvents()
{
	GetInstance()._DumpEvents();
}

void Profiler::_DumpEvents()
{
	::ParseEvents(m_Events);
	::DumpEvents(m_Events);
}