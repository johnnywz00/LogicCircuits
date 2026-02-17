
#include "sfmlApp.hpp"


FullscreenOnlyApp::FullscreenOnlyApp ()
{
	VideoMode mode = VideoMode::getDesktopMode();
	auto style = Style::Default;
	auto fsmodes = VideoMode::getFullscreenModes();
	if (fsmodes.size()) {
		mode = fsmodes[0];
		style = Style::Fullscreen;
	}
	window.create(mode, "", style);
	
	window.setFramerateLimit(60);
	
	if (!icon.loadFromFile((Resources::executingDir() / "resources" / "images" / "icon.png").string()))
		cerr << "Couldn't load icon. " << endl;
	else
		window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	
    srand(unsigned(time(nullptr)));
		
    state.rwin = &window;
	state.app = this;
	state.timedMgr = &timedMgr;

    state.onCreate();
	
    clock.restart();
}

void FullscreenOnlyApp::run ()
{
	cout << "Entering app.run()\n";
	while (!isDone) {
		update();
		
		window.clear(redrawColor);
		state.draw();
		window.display();
		
		elapsed += clock.restart();
	}
	cout << "Leaving app.run()\n";
}

void FullscreenOnlyApp::update ()
{
	state.oldMouse = state.mouseVec;
    Event event;
    while (window.pollEvent(event)) {
		vecF adj = window.mapPixelToCoords(vecI(event.mouseButton.x,
												event.mouseButton.y));
		if (!state.handleTextEvent(event)) {
			switch(event.type) {
					
				case Event::KeyPressed:
					switch(event.key.code) {
						default:
							state.onKeyPress(event.key.code);
							break;
					}
					break;
					
				case Event::KeyReleased:
					state.onKeyRelease(event.key.code);
					break;
					
				case Event::MouseButtonPressed:
					state.onMouseDown(int(adj.x), int(adj.y));
					break;
					
				case Event::MouseButtonReleased:
					state.onMouseUp(int(adj.x), int(adj.y));
					break;
					
				case Event::MouseMoved:
					adj = window.mapPixelToCoords(vecI(event.mouseMove.x,
													   event.mouseMove.y));
					state.mouseVec = toVecI(adj);
					break;
					
				case Event::Closed:
					close();
					break;
					
				default:
					break;
			}
		}
    }
    state.update(elapsed);
}


int main (int argc, char* argv[])
{
/* XCode folly: two instances of the program are launched if
 * we customize the working directory. We can cause the extraneous
 * instance to silently quit immediately by giving it a relative
 * path that it can't find.
 */
#ifdef DEBUG
    Image img;
	if (!img.loadFromFile("resources/images/icon.png")) {
		cerr << "DEBUG preprocessor symbol is defined, but icon.png can't be found through relative path.\n";
		return EXIT_FAILURE;
	}
#endif
	
	cout << "Initializing Resources\n";
	Resources::initialize(argc, argv);

	FullscreenOnlyApp app;
	
	cout << "Preparing to launch game loop for LogicCircuits\n";
	try {
		app.run();
		cout << "Exited LogicCircuits loop\n";
	}
	catch (std::runtime_error& e) {
		cerr << "Caught runtime exception: " << e.what() << endl;
	}
	catch (std::exception& e) {
		cerr << "Caught exception: " << e.what() << endl;
	}
	catch (...) {
		cerr << "Caught unknown exception\n";
	}
}
