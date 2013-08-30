#pragma once
#ifndef __WIRETAP_VIEWER_H__
#define __WIRETAP_VIEWER_H__

#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include "SFML/Network.hpp"

#include "WiretapUtils.h"

namespace Wiretap
{
	typedef std::vector<ProfileEvent> ProfileEventList;

	class ProfileFrame
	{
	public:
		ProfileFrame()
			: m_FrameLength(0.0f)
		{
		}

		ProfileFrame(float frameLength, ProfileEventList events)
			: m_FrameLength(frameLength)
			, m_Events(events)
		{
		}

		ProfileEventList& GetEvents() { return m_Events; }
		const ProfileEventList& GetEvents() const { return m_Events; }

		void Invalidate() { m_Events.clear(); m_FrameLength = 0.0f; }
		bool IsValid() const { return m_Events.size() > 0; }
		float GetLength() const { return m_FrameLength; }

	private:
		ProfileEventList m_Events;
		float m_FrameLength;
	};

	typedef std::vector<ProfileFrame> ProfileFrameList;

	class ProfileViewer
	{
	public:
		ProfileViewer(unsigned int windowWidth, unsigned int windowHeight, const char* windowTitle, unsigned short port);

		void Start();
		void Stop();
		void Update();
		bool IsOpen() const { return m_Window.isOpen(); }
		sf::WindowHandle GetWindowHandle() const { return m_Window.getSystemHandle(); }

	private:
		void HandleEvents();
		void HandleNetworkData(sf::Packet& packet);
		void Render();
	
		void UpdateMinMax();
		void UpdateAverage();
		void DrawLastFrames();
		void DrawOverlay();

		static void ServerThread(void* data);
		void ServerThreadRun(void* data);

		void SetTextDefault(sf::Text& text, float x, float y, const sf::Color& color, unsigned int size);

	private:
		static ProfileViewer* m_Instance;

		// Network
		sf::Thread			m_ServerThread;
		sf::Mutex			m_ServerMutex;
		ProfileEventList 	m_EventsFromNetwork;
		ProfileFrameList	m_FramesFromNetwork;
		unsigned short		m_Port;

		// Render thread
		ProfileFrameList 	m_FramesSinceBeginning;
		ProfileFrame		m_MinFrame;
		ProfileFrame		m_MaxFrame;
		bool				m_ServerThreadIsRunning;

		// Window
		sf::RenderWindow	m_Window;
		sf::String 			m_WindowTitle;
		unsigned int 		m_WindowWidth;
		unsigned int 		m_WindowHeight;

		// Viewer
		unsigned int 		m_EventDepth;
		float 				m_NbFramesToDisplay;
		float 				m_HalfScreenHeightTimeMs;
		float				m_FrameRectHeightMultiplier;
		float				m_LineTextVerticalOffset;

		// Viewer pause/resume/step
		ProfileFrame		m_CurrentFrame;
		unsigned int		m_FrameRangeStart;
		unsigned int		m_FrameRangeStop;
		unsigned int		m_SelectedFrameOffset;
		bool				m_UpdatePaused;

		// Viewer expand/collapse events
		UnorderedStringSet	m_ExpandedEventNames;
		StringArray			m_SelectedFrameEventNames;
		unsigned int		m_SelectedEventIndex;
		bool				m_OverlayEventsUpdated;

		// Viewer - Overlay
		sf::RectangleShape	m_SelectedEventRect;
		sf::Vertex			m_FrameLimitVertices[2];
		sf::Vertex			m_FrameMinVertices[2];
		sf::Vertex			m_FrameMaxVertices[2];
		sf::Vertex			m_FrameAverageVertices[2];
		sf::Text			m_FrameLimitText;
		sf::Text			m_FrameMinText;
		sf::Text			m_FrameMaxText;
		sf::Text			m_FrameAverageText;
		sf::Text			m_FrameInfoText;
		sf::Font			m_Font;
		float				m_FrameAverageLength;

		// Viewer - Overlay - Menu
		sf::Text			m_MenuControlsText;

		// Viewer - Timed scopes
		sf::Text			m_FrameEventsText;
	};
}

#endif //! __WIRETAP_VIEWER_H__
