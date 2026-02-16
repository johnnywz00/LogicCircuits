//
//  CircuitInput.cpp
//  LogicCircuits
//
//  Created by John Ziegler on 7/13/25.
//  Copyright © 2025 John Ziegler. All rights reserved.
//

#include "CircuitInput.hpp"

CircuitTerminus::CircuitTerminus()
{
	txt = Text("0", getFont(), 20);
	txt.setFillColor(Color::Black);
	txt.setOutlineColor(Color::White);
	txt.setOutlineThickness(2);
	centerOrigin(txt);
	
	txMapKey = "inout";
}

Font& CircuitTerminus::getFont()
{
	static bool isLoaded = false;
	static Font font;
	if (!isLoaded) {
		if (!font.loadFromFile("resources/Abadi MT Condensed Extra Bold"))
			cerr << "Failed to load font. "<< endl;
		isLoaded = true;
	}
	return font;
}

void CircuitTerminus::draw(RenderTarget& win, RenderStates st) const
{
	InterconnectNode::draw(win, st);
	if (State::getSelf()->mode == "simulate")
		win.draw(txt);
}
