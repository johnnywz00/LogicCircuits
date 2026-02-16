
#ifndef sfmlApp_hpp
#define sfmlApp_hpp


#include "state.hpp"
#include "timedeventmanager.hpp"

inline const string defaultTitle { "NEWGAMETITLE" };
inline const string iconPath { "resources/icon.png" };
inline Color baseScreenColor = Color::White;


class SFGameWindow
{
public:
	friend class Game;
    
    SFGameWindow ();

    SFGameWindow (const string& title, const vecU& size);
    
	~SFGameWindow () { destroy(); }
    
	void draw (Drawable& d) { window.draw(d); }
    
	void beginDraw () { window.clear(redrawColor); }
    
	void endDraw () { window.display(); }
    
	bool isDone () { return _isDone; }
    
	bool isFullscreen () { return _isFullscreen; }
        
	bool isFocused () { return _isFocused; }
    
	vecU getWindowSize () { return windowSize; };
    
	RenderWindow* getRenderWindow () { return &window; };
    
	void close () { _isDone = true; };
    
	void toggleFullscreen ();
	    
    
private:
    
    void destroy () { window.close(); };
    
	void setup (const string& title, const vecU& size);
    
	void create ();
    
    Image                   icon;
    RenderWindow            window;
    vecU                    windowSize;
    static const int        defaultWidth { 1280 };
    static const int        defaultHeight { 720 };
    string                  windowTitle;
	Color					redrawColor = baseScreenColor;

    bool                    _isDone;
    bool                    _isFullscreen;
    bool                    _isFocused;
};




class Game
{
public:
    
    Game ();
       
	void update ();
    
	void render () {
        window.beginDraw();
        state.draw();
        window.endDraw();
	}
    
	void lateUpdate () { restartClock(); }
    
	SFGameWindow* getWindow () { return &window; };
    
	Time getElapsed () { return elapsed; };
    
	void restartClock () { elapsed += clock.restart(); };

private:

    SFGameWindow     window;
	TimedEventManager      timedMgr {25000};
    State            state;
    Clock            clock;
    Time             elapsed;
};




#endif /* sfmlApp_hpp */
