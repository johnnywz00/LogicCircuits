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
using ICInputPtr = shared_ptr<ICInput>;


class ICInput
{
public:
	ICInput () { }
	int status = -1;
	weak_ptr<InterconnectNode> parent;
};



class InterconnectNode : public Drawable, public enable_shared_from_this<InterconnectNode>
{
public:
	InterconnectNode(int inputct = 1, int outputct = 1)
		: input1(make_shared<ICInput>())
	{
		nodeID = nextID++;
		inputCt = inputct;
		outputCt = outputct;
//		input1->parent = weak_from_this();
	}
	
	virtual ~InterconnectNode() = default;
	
	virtual void initInputs ()
	{
		input1->parent = weak_from_this();
	}
	
	void draw(RenderTarget& win, RenderStates st) const override
	{
		if (isActive)
			win.draw(spr);
	}
		
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
	}
	
	virtual ICInputPtr getInput_(int idx) { return input1; }
	
	virtual string getInputLocs() = 0;

	virtual string getOutputLocs() = 0;

	virtual void propagateOutput();
	
	static void resetNextID() { nextID = 0; }
	
	static Color circOffColor; //{193, 183, 165};
	static Color circOnColor;
	int 		nodeID;
	string		name;
	bool		isActive = true;
	VecfMM 		gridPos {vecF(-1, -1)};
	Sprite 		spr;
	string		txMapKey {"interconnects"};
	string		xformedStr;
	int 		inputCt;
	int 		outputCt;
	ICInputPtr  	input1;
	weak_ptr<ICInput> 	output1;
	
	void setXformedString()
	{
		string str = "nesw";
		if (epsEquals(spr.getRotation(), 270))
			std::rotate(str.begin(), str.begin() + 3, str.end());
		else if (epsEquals(spr.getRotation(), 90))
			std::rotate(str.begin(), str.begin() + 1, str.end());
		else if (epsEquals(spr.getRotation(), 180))
			std::rotate(str.begin(), str.begin() + 2, str.end());
		if (spr.getScale().x < 0) {
			swap(str[1], str[3]);
		}
		if (spr.getScale().y < 0) {
			swap(str[0], str[2]);
		}
		xformedStr = str;
	}
	
private:
	static inline int nextID = 0;
};



class TwoOutputICNode : public InterconnectNode
{
public:
	TwoOutputICNode() : InterconnectNode(1, 2) { }
	weak_ptr<ICInput> output2;
	
	ICInputPtr getOutput_ (int idx) override {
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
};



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
