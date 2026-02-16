
#include "sfmlApp.hpp"


SFGameWindow::SFGameWindow ()
{
	setup(defaultTitle, vecU(defaultWidth, defaultHeight));
}

SFGameWindow::SFGameWindow (const string& title, const vecU& size)
{
	setup(title, size);
}
	
void SFGameWindow::setup (const string& title, const vecU& size)
{
	windowTitle = title;
	windowSize = size;
	_isFullscreen = true;
	_isDone = false;
	_isFocused = true;
	create();
#ifdef EMBEDDED_HPP
	if (!loadByMethod(icon, iconPath))
#else
	if (!icon.loadFromFile(iconPath))
#endif
		cerr << "Couldn\'t load icon! \n";
	else window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
}

void SFGameWindow::create ()
{
    VideoMode mode = VideoMode::getDesktopMode();
	auto style = Style::Default;

	if (_isFullscreen) {
		auto fsmodes = VideoMode::getFullscreenModes();
		if (fsmodes.size()) {
			mode = fsmodes[0];
			style = Style::Fullscreen;
		}
		else _isFullscreen = false;
	}
	window.create(mode, windowTitle, style);
	window.setFramerateLimit(60);
}
	
void SFGameWindow::toggleFullscreen ()
{
	_isFullscreen = !_isFullscreen;
	destroy();
	create();
}


Game::Game ()
	: window()
{
    srand(unsigned(time(nullptr)));
    state.w = window.getRenderWindow();
	state.gw = &window;
	state.timedMgr = &timedMgr;
    state.onCreate();
    clock.restart();
}

void Game::update ()
{
	state.mxOld = state.mx;
	state.myOld = state.my;
    Event event;
    while (window.window.pollEvent(event)) {
		vecf adj = window.window.mapPixelToCoords(vecI(event.mouseButton.x,
													   event.mouseButton.y));
		if (state.activeTbox
				&& (event.type == Event::TextEntered
					|| event.type == Event::KeyPressed
					|| event.type == Event::KeyReleased)) {
			if (event.type == Event::TextEntered) {
				if (event.text.unicode == 8)
					if (iKP(LShift))
						state.activeTbox->clear();
					else state.activeTbox->deleteLastChar();
				else if (event.text.unicode == 9) ; // Don't write the \t
				else state.activeTbox->appendText(event.text.unicode);
			}
			if (event.type == Event::KeyPressed && (event.key.code == Keyboard::Escape || event.key.code == Keyboard::Enter)) {
				state.activeTbox->setActive(false);
				state.activeTbox = nullptr;
			}
			
		}
		else {
			switch(event.type) {
	
				case Event::KeyPressed:
					switch(event.key.code) {
						case Keyboard::F5:
							window.toggleFullscreen();
							break;
						default:
							state.onKeyPress(event.key.code);
							break;
					}
					break;
					
				case Event::KeyReleased:
					switch(event.key.code) {
						default:
							state.onKeyRelease(event.key.code);
							break;
					}
					break;
					
				case Event::MouseButtonPressed:
					state.onMouseDown(int(adj.x), int(adj.y));
					break;
					
				case Event::MouseButtonReleased:
					state.onMouseUp(int(adj.x), int(adj.y));
					break;
					
				case Event::MouseMoved:
					adj = window.window.mapPixelToCoords(vecI(event.mouseMove.x,
															  event.mouseMove.y));
					state.mx = int(adj.x);
					state.my = int(adj.y);
					break;
					
				case Event::Closed:
					window.close();
					break;
					
				case Event::LostFocus:
					window._isFocused = false;
					break;
					
				case Event::GainedFocus:
					window._isFocused = true;
					break;
					
				default:
					break;
			}
		}
    }
    state.update(elapsed);
}



int main ()
{
    Image img;
	if (!img.loadFromFile(iconPath))
        return EXIT_FAILURE;
    

    Game game;
    while (!game.getWindow()->isDone()) {
        game.update();
        game.render();
        game.lateUpdate();
    }
}
