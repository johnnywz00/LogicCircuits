//
//  CircuitInput.hpp
//  LogicCircuits
//
//  Created by John Ziegler on 7/13/25.
//  Copyright © 2025 John Ziegler. All rights reserved.
//

#ifndef CircuitInput_hpp
#define CircuitInput_hpp

#include "state.hpp"

class CircuitTerminus;
using TerminusPtr = shared_ptr<CircuitTerminus>;


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
