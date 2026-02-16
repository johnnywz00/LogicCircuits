//
//  CircuitInput.hpp
//  LogicCircuits
//
//  Created by John Ziegler on 7/13/25.
//  Copyright © 2025 John Ziegler. All rights reserved.
//

#ifndef CircuitInput_hpp
#define CircuitInput_hpp

//#include "InterconnectNode.hpp"
#include "state.hpp"

class State;

class CircuitTerminus : public InterconnectNode
{
public:
	CircuitTerminus();
	
	void draw(RenderTarget& win, RenderStates st) const override;

	void propagateOutput() override
	{
		InterconnectNode::propagateOutput();
		setString();
	}
	
	void setString()
	{
		if (getInput(0)->status == 1)
			txt.setString("1");
		else txt.setString("0");
		centerOrigin(txt);
	}
	
	static Font& getFont();
	Text		txt;
};

class CircuitInput : public CircuitTerminus
{
public:
	string getInputLocs() override { return "0"; }
	string getOutputLocs() override { return string({xformedStr[2]}); }
	
};



class CircuitOutput : public CircuitTerminus
{
public:
	string getInputLocs() override { return string({xformedStr[2]}); }
	string getOutputLocs() override { return "0"; }
	
	void propagateOutput() override
	{
		auto inp = getInput(0);
//		if (inp->status == -1)
//			return;
//		else
		if (inp->status == 1) {
			spr.setColor(circOnColor);
		}
		else {
			spr.setColor(circOffColor);
		}
		setString();
	}
};

#endif /* CircuitInput_hpp */
