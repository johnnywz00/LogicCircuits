//
//  CircuitInput.cpp
//  LogicCircuits
//
//  Created by John Ziegler on 7/13/25.
//  Copyright © 2025 John Ziegler. All rights reserved.
//

#include "state.hpp"

CircuitTerminus::CircuitTerminus()
{
	txt = Text("0", gFont("termini"), 20);
	txt.setFillColor(Color::Black);
	txt.setOutlineColor(Color::White);
	txt.setOutlineThickness(2);
	centerOrigin(txt);
	
	txMapKey = "termini";
}

void CircuitTerminus::draw(RenderTarget& win, RenderStates st) const
{
	InterconnectNode::draw(win, st);
	if (State::getSelf()->mode == "simulate")
		win.draw(txt);
}
