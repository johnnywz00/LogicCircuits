//
//  Interconnect.hpp
//  LogicCircuits
//
//  Created by John Ziegler on 7/13/25.
//  Copyright © 2025 John Ziegler. All rights reserved.
//

#ifndef InterconnectNode_hpp
#define InterconnectNode_hpp

#include "state.hpp"

class InterconnectNode;
class ICInput;
using ICNodePtr = shared_ptr<InterconnectNode>;
using ICNodeWkPtr = weak_ptr<InterconnectNode>;
using ICInputPtr = shared_ptr<ICInput>;
using ICInputWkPtr = weak_ptr<ICInput>;


/* Because a single track node such as a tee may have more than
 * one inlet, we give each inlet an individual identity to
 * facilitate linking nodes together later
 */
class ICInput
{
public:
	ICNodeWkPtr 	parent;
	int 			status = -1;
};


/* The root class for all interconnects/pieces of "track", both
 * for graphical representation and current propagation logic
 */
class InterconnectNode 	: public Drawable
						, public enable_shared_from_this<InterconnectNode>
{
public:
	static inline const Color 	circOffColor {250, 255, 240};
	static inline const Color 	circOnColor {63, 149, 228};

	InterconnectNode(int inputct = 1, int outputct = 1)
		: input1(make_shared<ICInput>())
	{
		nodeID = nextID++;
		inputCt = inputct;
		outputCt = outputct;
	}
	
	virtual ~InterconnectNode() = default;
	
	static void resetNextID() { nextID = 0; }

	virtual void initInputs () { input1->parent = weak_from_this(); }
	
	void draw(RenderTarget& win, RenderStates st) const override
	{
		if (isActive)
			win.draw(spr);
	}
	
	/* Get signals for this node's inputs and outputs,
	 * using a couple guard methods in the base class
	 * to cover error checking (probably overkill here)
	 */
	ICInputPtr getOutput(int idx)
	{
		if (idx < 0 || idx + 1 > outputCt)
			return nullptr;
		return getOutput_(idx);
	}
	
	virtual ICInputPtr getOutput_(int idx) {
		auto sp = output1.lock();
		return sp ? sp : nullptr;
	}
	
	ICInputPtr getInput(int idx)
	{
		if (idx < 0 || idx + 1 > inputCt)
			return nullptr;
		return getInput_(idx);
	}
	
	virtual void setOutput(int idx, const ICInputPtr& dest)
	{
		output1 = dest;
		/* Subclasses will use idx */
	}
	
	virtual ICInputPtr getInput_(int idx) { return input1; }
	
	virtual void propagateOutput();

	/* My system may be over-convoluted and a little off-beat,
	 * but we will need to link nodes together after all manner
	 * of rotating and axis flipping has been done to them in the
	 * editor. I'm using the string "nesw" (referring to the compass
	 * directions), and manipulating that string correspondent to
	 * the transformations that were performed on the node sprite
	 * in the editor. A given subclass has either one or two indices
	 * that it always picks: which letters of "nesw" end up at those
	 * indices after transformation determines which directions the
	 * node will look on-grid to try to connect to another node
	 * with an opening pointing to the first.
	 */
	virtual string getInputLocs() = 0;
	virtual string getOutputLocs() = 0;

	void setXformedString()
	{
		string str = "nesw";
		if (epsEquals(spr.getRotation(), 270))
			std::rotate(str.begin(), str.begin() + 3, str.end());
		else if (epsEquals(spr.getRotation(), 90))
			std::rotate(str.begin(), str.begin() + 1, str.end());
		else if (epsEquals(spr.getRotation(), 180))
			std::rotate(str.begin(), str.begin() + 2, str.end());
		/* Negative scale values mean the sprite has been flipped
		 * around one or both axes
		 */
		if (spr.getScale().x < 0) {
			swap(str[1], str[3]);
		}
		if (spr.getScale().y < 0) {
			swap(str[0], str[2]);
		}
		xformedStr = str;
	}
		
		
	Sprite 				spr;
	ICInputPtr  		input1;
	ICInputWkPtr 		output1;
	string				name;
	string				txMapKey {"interconnects"};
	string				xformedStr;
	VecfMM 				gridPos {vecF(-1, -1)};
	int 				nodeID;
	int 				inputCt;
	int 				outputCt;
	bool				isActive = true;
	
private:
	static inline int nextID = 0;
};


/* Splitting tees and branches */
class TwoOutputICNode : public InterconnectNode
{
public:
	TwoOutputICNode()
		: InterconnectNode(1, 2)
	{ }
	
	ICInputPtr getOutput_ (int idx) override
	{
		if (idx == 0) {
			auto sp = output1.lock();
			return sp ? sp : nullptr;
		}
		else { // idx == 1
			auto sp = output2.lock();
			return sp ? sp : nullptr;
		}
	}
	
	void setOutput(int idx, const ICInputPtr& dest) override
	{
		if (idx == 0)
			output1 = dest;
		else if (idx == 1)
			output2 = dest;
	}

	ICInputWkPtr 		output2;
};


/* Merging tees and branches */
class TwoInputICNode : public InterconnectNode
{
public:
	TwoInputICNode()
		: InterconnectNode(2, 1)
		, input2(make_shared<ICInput>())
	{ }
	
	void initInputs () override
	{
		input1->parent = weak_from_this();
		input2->parent = weak_from_this();
	}
	
	ICInputPtr getInput_ (int idx) override
	{
		return idx == 0 ? input1 : input2;
	}

	ICInputPtr input2;
};



class ICMergeTee : public TwoInputICNode
{
public:
	string getInputLocs() override { return string({xformedStr[1], xformedStr[3]}); }
	string getOutputLocs() override { return string({xformedStr[2]}); }
};

class ICBranchIn : public TwoInputICNode
{
public:
	string getInputLocs() override { return string({xformedStr[3], xformedStr[2]}); }
	string getOutputLocs() override { return string({xformedStr[1]}); }
};

class ICSplitTee : public TwoOutputICNode
{
public:
	string getInputLocs() override { return string({xformedStr[2]}); }
	string getOutputLocs() override { return string({xformedStr[3], xformedStr[1]}); }
};

class ICBranchOut : public TwoOutputICNode
{
public:
	string getInputLocs() override { return string({xformedStr[3]}); }
	string getOutputLocs() override { return string({xformedStr[1], xformedStr[2]}); }
};

class ICElbow : public InterconnectNode
{
public:
	string getInputLocs() override { return string({xformedStr[2]}); }
	string getOutputLocs() override { return string({xformedStr[1]}); }
};

class ICLElbow : public InterconnectNode
{
public:
	string getInputLocs() override { return string({xformedStr[2]}); }
	string getOutputLocs() override { return string({xformedStr[3]}); }
};

class ICStraightSeg : public InterconnectNode
{
public:
	string getInputLocs() override { return string({xformedStr[3]}); }
	string getOutputLocs() override { return string({xformedStr[1]}); }
};

#endif /* Interconnect_hpp */
