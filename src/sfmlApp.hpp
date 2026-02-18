
#ifndef sfmlApp_hpp
#define sfmlApp_hpp

#include "state.hpp"


class FullscreenOnlyApp
{
public:
    FullscreenOnlyApp ();
       
	void run ();
	
	void update ();
	
	void close ()
	{
		isDone = true;
		window.close();
	}
	
	void setRedrawColor (const Color& c) { redrawColor = c; }

	RenderWindow            window;
	TimedEventManager		timedMgr;
	State            		state;
	Clock            		clock;
	Time             		elapsed;
	Image                   icon;
	Color                   redrawColor { Color::White };
	bool					isDone = false;
};

#endif /* sfmlApp_hpp */
