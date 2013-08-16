**How are the profile points captured?**

By declaring a profile point (PROFILE_SCOPE()) it creates a ProfileEvent object on the stack and saves the current time. When leaving the scope (function or unnamed scope, matching closing curly bracket "}"), the destructor of ProfileEvent is called and the current time measured. The difference with the start time gives the execution time. Each start and stop events are pushed into a vector. All the events are sent by calling Wiretap::Profiler::SendData();. The frame time is the difference between the first event's start and the last event's stop. SendData is (and should always be) outside any PROFILE_SCOPE so the timed spent sending the data is not counted and it's not biasing the timings.

**How much CPU and memory does the viewer uses?**

It depends on the amount of data that is streamed. I've tested sending dozens of events 30 times per second and the viewer was taking 2% of the CPU. Memory also depends on the number of profile points and the number of frames per second. A rough estimate is 1.5MB/sec for 1000 PROFILE_SCOPEs at 30FPS. About 5.4GB for 1h for intense capture. This memory cost will most likely get optimized later.

**Why are you using SFML?**

Wiretap is meant to be portable and requires a number of systems that SFML already provides for several OS, such as threading, window, window and keyboard events, networking, 2D rendering. These are pretty standard stuff, no need to reinvent the wheel. That brings the focus on the profiler/viewer themselves.

**My OS is not supported, what can I do?**

SFML is open source and could probably be ported anywhere without too much trouble. Then if some of Wiretap's code is not compatible, enter an issue, send me an email or fork and fix it.

**Why is there a lot of includes and libs in the external folder?**

I'm not sure what the best way to do this is. SFML doesn't seem to have a github account so I can't reference it. But mainly I want to provide something you can clone/download and compile without having to follow a complicated setup. So the includes/libs are provided for Windows/Linux, 32bit, 64bit, VS08, VS11.

**Do I or any user of Wiretap need the SFML dlls?**

Right now SFML is linked statically so a single executable can be distributed easily. It might get a dll/import libs config later if requested.

**Could the profiler work for x language?**

Python and C# are planned, most languages that let you get accurate time and do socket networking could work for the profiler part.

**How can I contribute?**

You're free to fork it and modify wiretap the way you want. If you want your work to be integrated into the main/official version and depot, take a look at the [TODO list](TODO.txt) and send me an email if you want to work on something there. Or just do it and do a pull request, but the email first would be better. Right now is a little early for contributions, I put it on github early to keep track of my own changes. I'm definitely interested in comments and suggestions.
