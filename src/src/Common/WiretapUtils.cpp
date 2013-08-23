#include "WiretapUtils.h"
#include <sstream>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <sys/time.h>
#endif

using namespace Wiretap;

/*
** Profiler utils
*/

std::string Wiretap::GetIndent(unsigned int indentationLevel)
{
	std::string tabs = "";
	for (unsigned int i = 0; i < indentationLevel; i++)
		tabs += "  ";
	return tabs;
}

void Wiretap::DumpEvents(const std::vector<ProfileEvent>& events, std::ostringstream* outStr /* = NULL */, StringArray* eventNames /* = NULL */, UnorderedStringSet* expandedEventNames /* = NULL */)
{
	unsigned int eventIndex = 0;
	unsigned int eventDepth = 0;

	bool isEventExpanded = true;
	std::string collapsedEventName;

	while (eventIndex < events.size())
	{
		const ProfileEvent& profileEvent = events[eventIndex];
		eventIndex++;

		if (profileEvent.GetType() == ProfileEvent_Start)
		{
			if (isEventExpanded)
			{
				const std::string tabs = GetIndent(eventDepth);
				eventDepth += 1;
				if (outStr)
					(*outStr) << tabs.c_str() << profileEvent.GetName() << " - " << profileEvent.GetDuration() << "s\n";
				else
					printf("%s%s - %fs\n", tabs.c_str(), profileEvent.GetName(), profileEvent.GetDuration());

				if (eventNames)
					eventNames->push_back(profileEvent.GetName());
			}
			
			if (isEventExpanded && expandedEventNames && expandedEventNames->find(profileEvent.GetName()) == expandedEventNames->end())
			{
				isEventExpanded = false;
				collapsedEventName = profileEvent.GetName();
			}
		}
		else
		{
			if (isEventExpanded)
			{
				eventDepth -= 1;
			}
			else if (collapsedEventName == profileEvent.GetName())
			{
				eventDepth -= 1;
				isEventExpanded = true;
			}
		}
	}
}

double Wiretap::ParseEvents(std::vector<ProfileEvent>& events)
{
	static unsigned int eventIndex = 0;
	static unsigned int eventDepth = 0;

	while (eventIndex < events.size())
	{
		ProfileEvent& profileEvent = events[eventIndex];
		eventIndex++;

		if (profileEvent.GetType() == ProfileEvent_Start)
		{
			eventDepth += 1;
			const double stopTime = ParseEvents(events);
			const double elapsedTime = stopTime - profileEvent.GetTime();
			profileEvent.SetDuration(elapsedTime);
		}
		else
		{
			eventDepth -= 1;
			return profileEvent.GetTime();
		}
	}

	eventIndex = 0;
	eventDepth = 0;

	return 0.0f;
}


/*
** High precision timer
*/

#ifndef _WIN32
typedef union _LARGE_INTEGER
{
	struct
	{
		unsigned int LowPart;
		int HighPart;
	} u;
	long long QuadPart;
} LARGE_INTEGER;

bool QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
	struct timeval t;

	gettimeofday(&t, NULL);
	lpPerformanceCount->QuadPart = t.tv_usec + t.tv_sec * 1000000;
	return true;
}

bool QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency)
{
	lpFrequency->QuadPart = 1000000;
	return true;
}
#endif

double Wiretap::GetHiResTime()
{
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	LARGE_INTEGER f;
	QueryPerformanceFrequency(&f);
	return (double)(t.QuadPart * 1000.0 / f.QuadPart) / 1000.0;
}
