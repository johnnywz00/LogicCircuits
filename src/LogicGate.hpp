//
//  LogicGate.hpp
//  LogicCircuits
//
//  Created by John Ziegler on 7/13/25.
//  Copyright © 2025 John Ziegler. All rights reserved.
//

#ifndef LogicGate_hpp
#define LogicGate_hpp

#include "state.hpp"


class LogicGate;
using GatePtr = shared_ptr<LogicGate>;
using GateWkPtr = weak_ptr<LogicGate>;


class GateICNode : public InterconnectNode
{
public:
	GateWkPtr		parent;
};



class GateInput : public GateICNode
{
public:
	GateInput() : GateICNode() { outputCt = 0; }
	string getInputLocs() override { return string({xformedStr[3]}); }
	string getOutputLocs() override { return string({xformedStr[1]}); }
	void propagateOutput() override;
};



class GateOutput : public GateICNode
{
public:
	string getInputLocs() override { return string({xformedStr[3]}); }
	string getOutputLocs() override { return string({xformedStr[1]}); }
};


   

class LogicGate : public Drawable
{
public:
	virtual ~LogicGate () = default;
	
	void draw (RenderTarget&, RenderStates) const override;
	
	/* The "rects" here are simply small blue blips drawn
	 * to represent where current is flowing within bounds
	 * of the gate sprite
	 */
	virtual vector<RectangleShape> updateRects () = 0;
	
	virtual pair<vecF, vecF> gridOffsets () = 0;
	
	virtual void propagateOutput ();
	
	virtual void calcAndSetOutput (int& outStatus) { }
	
	virtual void initialize (GatePtr&, const string&, int x, int y, const Sprite&);
	
	virtual void setPosition (float x, float y);
	
	void initializeGateNode (ICNodePtr&, GatePtr&);
	
	void drawSupplyLines (RenderWindow*);
	
	Sprite 		spr;
	vector<RectangleShape>
				flowRects;
	ICNodePtr 	inputA = nullptr;
	ICNodePtr 	inputB = nullptr;
	ICNodePtr 	output1 = nullptr;
	string 		name;
	vecF 		supplyOffset;
	bool 		isActive = true;
	
protected:
	/* Pixel offsets for where to start drawing
	 * the supply lines that feed the gate
	 */
	static inline map<string, vecF> soMap = {
		{"not", 	{11, 52}}
		, {"and", 	{27, 58}}
		, {"nand", 	{15, 52}}
		, {"or", 	{16, 45}}
		, {"nor", 	{19, 45}}
		, {"xor",	{46, 64}}
	};
	
	bool A() const { return inputA->input1->status == 1; }
	bool B() const { return inputB->input1->status == 1; }
	
	vector<RectangleShape> dataToRectShapes (intvec data);
	
	vecF cornerToOgnCoords (vecF fromCorner) const;
	
	vector<RectangleShape>& addRectsToVec (vector<RectangleShape>& vec, intvec data)
	{
		return vecPlusEqVec(vec, dataToRectShapes(data));
	}
};



class NotGate : public LogicGate
{
public:
	vector<RectangleShape> updateRects () override
	{
		vector<RectangleShape> ret;
		if (A())
			addRectsToVec(ret, {0, 27, 5, 2});
		else addRectsToVec(ret, {8, 16, 2, 5,     8, 16, 10, 2,   16, 16, 2, 13,
								 16, 27, 12, 2,   7, 27, 4, 2});
		return ret;
	}
	
	void propagateOutput () override
	{
		flowRects = updateRects();
		if (auto nextNode = output1->output1.lock()) {
			nextNode->status = !A() ? 1 : 0;
			if (auto nextParent = (nextNode->parent).lock())
				nextParent->propagateOutput();
		}
	}
	
	pair<vecF, vecF> gridOffsets () override { return {{-1, 0}, {-999999, -999999}}; }
};



class AndGate : public LogicGate
{
public:
	vector<RectangleShape> updateRects () override
	{
		vector<RectangleShape> ret;
		if (A())
			addRectsToVec(ret, {0, 13, 3, 2});
		if (!A())
			addRectsToVec(ret, {6, 21, 2, 8,   5,13,4,2});
		if (B())
			addRectsToVec(ret, {0, 41, 3, 2});
		if (!B())
			addRectsToVec(ret, {6, 31, 2, 4,   5,41,4,2});
		if (!(A() && B()))
			addRectsToVec(ret, {6, 29, 19, 2});
		if (A() && B())
			addRectsToVec(ret, {28, 18, 10, 2,   28, 18, 2, 5,   27,29,4,2,
								36, 18, 2, 11,   36, 27, 6, 2});
		return ret;
	}
	
	void calcAndSetOutput (int& outStatus) override
	{
		outStatus = A() && B() ? 1 : 0;
	}
	
	pair<vecF, vecF> gridOffsets () override { return {{-2, -1}, {-2, 1}}; }
};



class OrGate : public LogicGate
{
public:
	vector<RectangleShape> updateRects () override
	{
		vector<RectangleShape> ret;
		if (A())
			addRectsToVec(ret, {0, 14, 10, 2,   8, 14, 2, 14});
		if (B())
			addRectsToVec(ret, {0, 28, 8, 2});
		if (A() || B())
			addRectsToVec(ret, {8, 28, 6, 2,   31, 16, 4, 2,   32, 5, 2, 5,
								32, 5, 10, 2,   40, 5, 2, 25,   40, 28, 16, 2});
		if (!A() && !B())
			addRectsToVec(ret, {17, 16, 2, 6,   17, 16, 12, 2,   16, 28, 4, 2});
		return ret;
	}
	
	void calcAndSetOutput (int& outStatus) override
	{
		outStatus = A() || B() ? 1 : 0;
	}
	
	pair<vecF, vecF> gridOffsets() override { return {{-3, -1}, {-3, 0}}; }
};



class XorGate : public LogicGate
{
public:
	vector<RectangleShape> updateRects () override
	{
		vector<RectangleShape> ret;
		if (A())
			addRectsToVec(ret, {0, 14, 14, 2,   8, 16, 2, 12,   8, 26, 8, 2,
								20, 26, 11, 2,   29, 28, 2, 8});
		if (B())
			addRectsToVec(ret, {0, 42, 26, 2,   17, 22, 2, 20});
		if (!A() && B())
			addRectsToVec(ret, {16, 14, 4, 2,   17, 3, 2, 5,
								17, 3, 22, 2,   37, 5, 2, 11});
		if (A() && !B())
			addRectsToVec(ret, {29, 50, 2, 5,   31, 53, 8, 2,
								37, 18, 2, 35,   28, 42, 4, 2});
		if (A() != B())  //XOR
			addRectsToVec(ret, {37, 16, 7, 2,   60, 27, 2, 4,   68, 28, 16, 2});
		else if (A() == B())
			addRectsToVec(ret, {46, 16, 4, 2,   47, 5, 2, 5,
								47, 5, 15, 2,   60, 5, 2, 20});
		return ret;
	}
	
	void calcAndSetOutput (int& outStatus) override
	{
		outStatus = A() != B() ? 1 : 0;
	}
	
	pair<vecF, vecF> gridOffsets () override { return {{-5, -1}, {-5, 1}}; }
};



class NAndGate : public LogicGate
{
public:
	vector<RectangleShape> updateRects () override
	{
		vector<RectangleShape> ret;
		if (A())
			addRectsToVec(ret, {0, 18, 20, 2});
		if (!A()) addRectsToVec(ret, {23, 8, 2, 4,   22, 18, 4, 2});
		if (B())
			addRectsToVec(ret, {0, 32, 5, 2});
		if (!B()) addRectsToVec(ret, {8, 21, 2, 5,   8, 6, 2, 11,
								10, 6, 13, 2,   7, 32, 4, 2});
		if (!(A() && B()))
			addRectsToVec(ret, {23, 6, 11, 2,   32, 8, 2, 26,   34, 32, 22, 2});
		return ret;
	}
	
	void calcAndSetOutput (int& outStatus) override
	{
		outStatus = !(A() && B()) ? 1 : 0;
	}
	
	pair<vecF, vecF> gridOffsets () override { return {{-3, -1}, {-3, 0}}; }
};



class NOrGate : public LogicGate
{
public:
	vector<RectangleShape> updateRects () override
	{
		vector<RectangleShape> ret;
		if (A())
			addRectsToVec(ret, {0, 13, 16, 2});
		if (B())
			addRectsToVec(ret, {0, 31, 16, 2});
		if (!B())
			addRectsToVec(ret, {18, 31, 4, 2,   19, 21, 2, 4});
		if (!A() && !B())
			addRectsToVec(ret, {18, 13, 4, 2,   19, 3, 2, 4,   19, 3, 12, 2,
								29, 5, 2, 12,   29, 15, 27, 2});
		return ret;
	}
	
	void calcAndSetOutput (int& outStatus) override
	{
		outStatus = !(A() || B()) ? 1 : 0;
	}
	
	pair<vecF, vecF> gridOffsets () override { return {{-3, 0}, {-3, 1}}; }
};

#endif /* LogicGate_hpp */
