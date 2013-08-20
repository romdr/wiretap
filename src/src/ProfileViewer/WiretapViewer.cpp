#include "WiretapViewer.h"
#include <sstream>

#define max(a, b) (((a) > (b)) ? (a) : (b))

using namespace Wiretap;

ProfileViewer* ProfileViewer::m_Instance = NULL;

ProfileViewer::ProfileViewer(unsigned int windowWidth, unsigned int windowHeight, const char* windowTitle, unsigned short port)
: m_WindowWidth(windowWidth)
, m_WindowHeight(windowHeight)
, m_WindowTitle(windowTitle)
, m_Port(port)
, m_ServerThread(&ProfileViewer::ServerThread, &m_Window)
, m_EventDepth(0)
, m_NbFramesToDisplay(33.0f)
, m_HalfScreenHeightTimeMs(33.0f)
, m_LineTextVerticalOffset(-17.0f)
, m_FrameAverageLength(0.0f)
, m_FrameRangeStart(0)
, m_FrameRangeStop(0)
, m_SelectedFrameOffset(0)
, m_UpdatePaused(false)
, m_ServerThreadIsRunning(false)
{
	m_Instance = this;
	m_Font.loadFromFile("consolas.ttf");

	// Frame limit line - TODO: update when resizing the window
	m_FrameLimitVertices[0] = sf::Vertex(sf::Vector2f(0.0f, (float)m_WindowHeight / 2.0f), sf::Color(255, 77, 0));
	m_FrameLimitVertices[1] = sf::Vertex(sf::Vector2f((float)m_WindowWidth, (float)m_WindowHeight / 2.0f), sf::Color(255, 77, 0));

	m_FrameMinVertices[0] = sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(0, 255, 0));
	m_FrameMinVertices[1] = sf::Vertex(sf::Vector2f((float)m_WindowWidth, 0.0f), sf::Color(0, 255, 0));

	m_FrameMaxVertices[0] = sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(255, 0, 0));
	m_FrameMaxVertices[1] = sf::Vertex(sf::Vector2f((float)m_WindowWidth, 0.0f), sf::Color(255, 0, 0));

	m_FrameAverageVertices[0] = sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(32, 91, 92));
	m_FrameAverageVertices[1] = sf::Vertex(sf::Vector2f((float)m_WindowWidth, 0.0f), sf::Color(32, 91, 92));

	SetTextDefault(m_MenuControlsText, 0.0f, 0.0f, sf::Color::White, 12);
	SetTextDefault(m_FrameInfoText, m_WindowWidth - 175.0f, 0.0f, sf::Color::White, 12);
	SetTextDefault(m_FrameLimitText, 0.0f, m_FrameLimitVertices[0].position.y + m_LineTextVerticalOffset, m_FrameLimitVertices[0].color, 12);
	SetTextDefault(m_FrameMinText, 0.0f, m_FrameMinVertices[0].position.y + m_LineTextVerticalOffset, m_FrameMinVertices[0].color, 12);
	SetTextDefault(m_FrameMaxText, 0.0f, m_FrameMaxVertices[0].position.y + m_LineTextVerticalOffset, m_FrameMaxVertices[0].color, 12);
	SetTextDefault(m_FrameAverageText, 0.0f, m_FrameAverageVertices[0].position.y + m_LineTextVerticalOffset, m_FrameAverageVertices[0].color, 12);
	SetTextDefault(m_FrameEventsText, 50.0f, 24.0f, sf::Color(32, 91, 92), 12);
}

void ProfileViewer::Start()
{
	m_Window.create(sf::VideoMode(m_WindowWidth, m_WindowHeight, 32), m_WindowTitle);
	m_Window.setFramerateLimit(30); // hack, need mutex, see todo
	m_Window.setKeyRepeatEnabled(false);
	m_Window.clear(sf::Color(166, 178, 179));
	m_Window.display();

	m_ServerThread.launch();
}

void ProfileViewer::Stop()
{
	m_ServerThread.wait();
}

void ProfileViewer::Update()
{
	HandleEvents();
	Render();
}

void ProfileViewer::HandleEvents()
{
	sf::Event event;
	while (m_Window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			m_Window.close();

			// The server thread should stop by itself if a client is connected and streaming data
			// but it could also be stuck in an accept(), in that case we terminate it
			sf::sleep(sf::seconds(0.5f));
			if (m_ServerThreadIsRunning)
				m_ServerThread.terminate();
		}
		else if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::R)
			{
				m_MinFrame.Invalidate();
				m_MaxFrame.Invalidate();
			}
			else if (event.key.code == sf::Keyboard::Space)
			{
				m_UpdatePaused = !m_UpdatePaused;
			}
			else if (event.key.code == sf::Keyboard::P)
			{
				if (m_SelectedFrameOffset < m_FrameRangeStop + 1)
					m_SelectedFrameOffset++;
			}
			else if (event.key.code == sf::Keyboard::N)
			{
				if (m_SelectedFrameOffset > 0)
					m_SelectedFrameOffset--;
			}
		}
	}
}

void ProfileViewer::Render()
{
	// Copy the new frame
	ProfileFrame newFrame;

	m_ServerMutex.lock();
	if (m_Frame.IsValid())
	{
		newFrame = m_Frame;
		m_Frame.Invalidate();
	}
	m_ServerMutex.unlock();

	m_FrameRectHeightMultiplier = (m_WindowHeight / 2.0f) / m_HalfScreenHeightTimeMs;

	// Update / render
	if (newFrame.IsValid())
	{
		m_FramesSinceBeginning.push_back(newFrame);

		if (!m_UpdatePaused)
		{
			m_CurrentFrame = newFrame;
			m_FrameRangeStart = (unsigned int)max((int)m_FramesSinceBeginning.size() - (int)m_NbFramesToDisplay, 0);
			m_FrameRangeStop = m_FramesSinceBeginning.size();
			m_SelectedFrameOffset = 0;
		}

		UpdateMinMax();
		UpdateAverage();

		m_Window.clear(sf::Color(166, 178, 179));

		DrawLastFrames();

		if (m_FramesSinceBeginning.size() > 0)
		{
			DrawOverlay();
		}

		m_Window.display();
	}
}

void ProfileViewer::UpdateMinMax()
{
	if (m_CurrentFrame.GetLength() >= m_MaxFrame.GetLength())
	{
		m_MaxFrame = m_CurrentFrame;
		const float maxFrameY = m_WindowHeight - m_FrameRectHeightMultiplier * m_MaxFrame.GetLength() * 1000.0f;
		m_FrameMaxVertices[0].position.y = maxFrameY;
		m_FrameMaxVertices[1].position.y = maxFrameY;

		std::ostringstream outstr;
		outstr << "Max " << m_MaxFrame.GetLength() * 1000.0f << "ms";
		m_FrameMaxText.setString(outstr.str());
		m_FrameMaxText.setPosition(0.0f, maxFrameY + m_LineTextVerticalOffset);
	}
	if (m_CurrentFrame.GetLength() <= m_MinFrame.GetLength() || !m_MinFrame.IsValid())
	{
		m_MinFrame = m_CurrentFrame;
		const float minFrameY = m_WindowHeight - m_FrameRectHeightMultiplier * m_MinFrame.GetLength() * 1000.0f;
		m_FrameMinVertices[0].position.y = minFrameY;
		m_FrameMinVertices[1].position.y = minFrameY;

		std::ostringstream outstr;
		outstr << "Min " << m_MinFrame.GetLength() * 1000.0f << "ms";
		m_FrameMinText.setString(outstr.str());
		m_FrameMinText.setPosition(0.0f, minFrameY + m_LineTextVerticalOffset);
	}
}

void ProfileViewer::UpdateAverage()
{
	// Average is calculated over the visible frames
	m_FrameAverageLength = 0.0f;
	float frameLengthSum = 0.0f;
	unsigned int i = m_FrameRangeStart;
	const unsigned int iStart = i;
	while (i < m_FrameRangeStop)
		frameLengthSum += m_FramesSinceBeginning[i++].GetLength() * 1000.0f;
	if ((i - iStart) > 0)
		m_FrameAverageLength = frameLengthSum / (i - iStart);
	const float averageFrameY = m_WindowHeight - m_FrameRectHeightMultiplier * m_FrameAverageLength;
	m_FrameAverageVertices[0].position.y = averageFrameY;
	m_FrameAverageVertices[1].position.y = averageFrameY;

	std::ostringstream outstr;
	outstr << "Avg " << m_FrameAverageLength << "ms";
	m_FrameAverageText.setString(outstr.str());
	m_FrameAverageText.setPosition(0.0f, averageFrameY + m_LineTextVerticalOffset);
}

void ProfileViewer::DrawLastFrames()
{
	sf::RectangleShape selectedRect;
	for (unsigned int i = m_FrameRangeStart, j = 0; i < m_FrameRangeStop; i++, j++)
	{
		const float frameLengthMs = m_FramesSinceBeginning[i].GetLength() * 1000.0f;
		const sf::Vector2f frameRectSize(m_WindowWidth / m_NbFramesToDisplay, m_FrameRectHeightMultiplier * frameLengthMs);
		const sf::Vector2f posTL(frameRectSize.x * j, m_WindowHeight - frameRectSize.y);
		sf::RectangleShape rect;
		rect.setSize(frameRectSize);
		rect.setPosition(posTL);

		if (frameLengthMs > m_HalfScreenHeightTimeMs)
			rect.setFillColor(sf::Color::Red);
		else
			rect.setFillColor(sf::Color(55, 64, 64));

		if (i == (m_FrameRangeStop - 1 - m_SelectedFrameOffset))
			selectedRect = rect;

		m_Window.draw(rect);
	}

	// Draw the selected rect last so the outline is on top
	selectedRect.setOutlineThickness(3.0f);
	selectedRect.setOutlineColor(sf::Color(80, 97, 97));
	m_Window.draw(selectedRect);
}

void ProfileViewer::DrawOverlay()
{
	// Controls
	std::ostringstream outstrMenu;
	outstrMenu << "[SPACE] Pause/Resume | [R]eset min/max/average | [P]revious / [N]ext frame";
	m_MenuControlsText.setString(outstrMenu.str());

	// Current frame number and duration
	std::ostringstream outstrInfo;
	const unsigned int currentFrameNumber = m_FrameRangeStop - m_SelectedFrameOffset;
	const ProfileFrame& currentFrame = m_SelectedFrameOffset ? m_FramesSinceBeginning[currentFrameNumber - 1] : m_CurrentFrame;
	outstrInfo << "Frame " << currentFrameNumber << " (" << currentFrame.GetLength() * 1000.0f << "ms)";
	m_FrameInfoText.setString(outstrInfo.str());

	// Display the timed scopes
	std::ostringstream strEvents;
	ProfileEventList frameEvents = currentFrame.GetEvents();
	DumpEvents(frameEvents, &strEvents);
	m_FrameEventsText.setString(strEvents.str());

	// Frame limit line (above this line is a 'slow' frame
	std::ostringstream outstrLimit;
	outstrLimit << "Limit " << m_HalfScreenHeightTimeMs << "ms";
	m_FrameLimitText.setString(outstrLimit.str());
	m_FrameLimitText.setPosition(0.0f, m_FrameLimitVertices[0].position.y + m_LineTextVerticalOffset);

	m_Window.draw(m_MenuControlsText);
	m_Window.draw(m_FrameInfoText);
	m_Window.draw(m_FrameEventsText);

	// Min, max, avg, limit frames text and lines
	m_Window.draw(m_FrameMinText);
	m_Window.draw(m_FrameMaxText);
	m_Window.draw(m_FrameAverageText);
	m_Window.draw(m_FrameLimitText);
	m_Window.draw(m_FrameMinVertices, 2, sf::Lines);
	m_Window.draw(m_FrameMaxVertices, 2, sf::Lines);
	m_Window.draw(m_FrameAverageVertices, 2, sf::Lines);
	m_Window.draw(m_FrameLimitVertices, 2, sf::Lines);
}

void ProfileViewer::SetTextDefault(sf::Text& text, float x, float y, const sf::Color& color, unsigned int size)
{
	text.setFont(m_Font);
	text.setCharacterSize(size);
	text.setColor(color);
	text.setPosition(x, y);
}

void ProfileViewer::ServerThread(void* data)
{
	ProfileViewer::m_Instance->ServerThreadRun(data);
}

void ProfileViewer::ServerThreadRun(void* data)
{
	m_ServerThreadIsRunning = true;

	sf::TcpListener listener;
	if (listener.listen(m_Port) != sf::Socket::Done)
	{
		printf("Error: ServerThread: can't listen on port %d\n", m_Port);
		return;
	}

	while (IsOpen())
	{
		// Wait for a new client
		sf::TcpSocket client;
		if (listener.accept(client) != sf::Socket::Done)
		{
			printf("Error: ServerThread: can't accept client\n");
			continue;
		}

		// Receive data from this client until he disconnects
		sf::Packet packet;
		while (client.receive(packet) == sf::Socket::Done && IsOpen())
		{
			HandleNetworkData(packet);
		}

		m_EventDepth = 0;
		m_EventsFromNetwork.clear();

		client.disconnect();
	}

	m_ServerThreadIsRunning = false;
}

void ProfileViewer::HandleNetworkData(sf::Packet& packet)
{
	// De-serialize the packet into a profile event
	double time;
	std::string name;
	unsigned int type;
	packet >> time >> name >> type;
	ProfileEvent profileEvent(name, time, (EProfileEventType)type);
	m_EventsFromNetwork.push_back(profileEvent);

	// Depth at 0 indicates the frame is complete
	m_EventDepth += (profileEvent.GetType() == ProfileEvent_Start ? 1: -1);

	// Should be if new frame
	if (m_EventDepth == 0)
	{
		const float frameLength = (float)(m_EventsFromNetwork[m_EventsFromNetwork.size() - 1].GetTime() - m_EventsFromNetwork[0].GetTime());
		
		// If moving this code, make sure to call ParseEvents before creating the Frame so each Start event has a duration calculated
		ParseEvents(m_EventsFromNetwork);

		m_ServerMutex.lock();
		m_Frame = ProfileFrame(frameLength, m_EventsFromNetwork);
		m_ServerMutex.unlock();

		m_EventsFromNetwork.clear();
	}
}