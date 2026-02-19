//
//  LogicGate.cpp
//  LogicCircuits
//
//  Created by John Ziegler on 7/13/25.
//  Copyright © 2025 John Ziegler. All rights reserved.
//

#include "state.hpp"


void LogicGate::initialize(GatePtr& gateptr, const string& tag, int x, int y, const Sprite& cspr)
{
	auto state = State::getSelf();
	name = tag;
	spr.setTexture(gTexture(tag));
//	auto sz = spr->getTexture()->getSize();
//	spr->setTextureRect(IntRect(0, 0, sz.x, sz.y));
	spr.setOrigin(cspr.getOrigin());
//	spr->setRotation(cursorSprite.getRotation());
	spr.setScale(cspr.getScale());
//	gate->setXformedString();
	state->icNodes.push_back(make_shared<GateInput>());
	inputA = state->icNodes.back();
	initializeGateNode(inputA, gateptr);
	inputA->name = "gateinput";
	state->icNodes.push_back(make_shared<GateInput>());
	inputB = state->icNodes.back();
	initializeGateNode(inputB, gateptr);
	inputB->name = "gateinput";
	state->icNodes.push_back(make_shared<GateOutput>());
	output1 = state->icNodes.back();
	initializeGateNode(output1, gateptr);
	output1->name = "gateoutput";
	setPosition(x, y);
	supplyOffset = soMap[name];
}


void LogicGate::setPosition(float x, float y)
{
	auto state = State::getSelf();
	auto newPos = state->alignToGrid(x, y);
	if (output1->gridPos.vec.x > -1 && output1->gridPos.vec.y > -1) {
		state->removeNodeFromGrid(inputA);
		state->removeNodeFromGrid(inputB);
		state->removeNodeFromGrid(output1);
	}
	spr.setPosition(newPos);
	
	// MODIFY IF ADDING GATE ROTATION
	auto gp = state->toGridPos(x, y);
	inputA->gridPos = {gp + gridOffsets().first};
	state->gridLocs.emplace(inputA->gridPos, inputA);
	float factor = state->baseCellSize * state->gridScale.x;
	inputA->spr.setPosition(newPos + vecF{gridOffsets().first * factor});
	inputB->gridPos = {gp + gridOffsets().second};
	state->gridLocs.emplace(inputB->gridPos, inputB);
	inputB->spr.setPosition(newPos + vecF{gridOffsets().second * factor});
	output1->gridPos = {gp};
	state->gridLocs.emplace(output1->gridPos, output1);
	output1->spr.setPosition(newPos);
	flowRects.clear();
	updateRects();
}

void LogicGate::initializeGateNode(ICNodePtr& node, GatePtr& gate)
{
	Sprite* spr = &node->spr;
	spr->setTexture(gTexture("interconnects"));
	spr->setTextureRect(IntRect(14, 0, 14, 14));
	centerOrigin(*spr);
	spr->setRotation(0);
	spr->setScale(State::getSelf()->gridScale);
	node->setXformedString();
	node->initInputs();
	auto gnode = dynamic_pointer_cast<GateICNode>(node);
	if (gnode)
		gnode->parent = gate;
//	gnode->parent = this;
}


void LogicGate::draw(RenderTarget& win, RenderStates st) const
{
	if (drawGateShape)
		win.draw(gateShape);
	win.draw(spr);
	if (State::getSelf()->mode == "simulate") {
		for (auto& rect : flowRects)
			win.draw(rect);
	}
}

void NotGate::draw(RenderTarget& win, RenderStates st) const
{
	LogicGate::draw(win, st);
	if (!A()) {
		RectangleShape r {{4, 4}};
		r.setFillColor(Color::Black);
		r.setScale(State::getSelf()->gridScale);
		r.setPosition(cornerToOgnCoords(vecF(2, 26)));
		win.draw(r);
	}
}
	
vector<RectangleShape> LogicGate::dataToRectShapes(intvec data)
{
	vector<RectangleShape> ret;
	for (int i = 0; i < data.size(); i += 4) {
		RectangleShape r {{(float)data[i + 2], (float)data[i + 3]}};
		r.setPosition(cornerToOgnCoords(vecF(data[i], data[i + 1])));
		// add pvec to setposition to handle rotation
		//rotate actual rectangles
		r.setScale(State::getSelf()->gridScale);
		r.setFillColor(inputA->circOnColor);
		ret.push_back(r);
	}
	return ret;
}

vecF LogicGate::cornerToOgnCoords(vecF fromCorner) const
{
	auto scale = State::getSelf()->gridScale.x;
	return {spr.getPosition() - spr.getOrigin() * scale + fromCorner * scale};
}

void LogicGate::drawSupplyLines(RenderWindow* w)
{
	RectangleShape r;
	Color c = InterconnectNode::circOnColor;
	r.setFillColor(c);
	r.setSize({8, 20});
	r.setPosition(cornerToOgnCoords(supplyOffset));
	for (int i = 0; i < 6; ++i) {
		w->draw(r);
		r.move(0, 20);
		c.a -= 30;
		r.setFillColor(c);
	}
	r.setSize({8, (w->getView().getCenter().y + w->getView().getSize().y / 2) -
		r.getPosition().y + 10});
	w->draw(r);
}

void LogicGate::propagateOutput()
{
		flowRects = updateRects();
		if (auto nextNode = output1->output1.lock()) {
			calcAndSetOutput(nextNode->status);
			auto state = State::getSelf();
			/* Trying to use instant propagation instead of animation
			 * was causing stack overflow for certain circuits like
			 * S-R latch. Extra code could get around this, but for now
			 * instead of turning off animation we set it to a very
			 * low delay.
			 */
//			if (state->animateFlow)
			{
				state->timedMgr->addEventIf("gate" + fS(spr.getPosition().x) + "," + fS(spr.getPosition().y),
										  State::flowAnimDelay, [wkNode = nextNode->parent](){
					if (auto node = wkNode.lock()) {
						node->propagateOutput();
					}
				});
			}
//			else {
//				if (auto nextParent = (nextNode->parent).lock())
//					nextParent->propagateOutput();
//			}
		}
}

void GateInput::propagateOutput()
{
	/* The parent gate takes care of any animation delay */
	if (auto parentSp = parent.lock())
		parentSp->propagateOutput();
}
