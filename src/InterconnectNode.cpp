//
//  Interconnect.cpp
//  LogicCircuits
//
//  Created by John Ziegler on 7/13/25.
//  Copyright © 2025 John Ziegler. All rights reserved.
//

#include "state.hpp"


void InterconnectNode::propagateOutput()
{
	/* Determine if this node will output a 1 signal */
	bool willOutput = false;
	for (int i = 0; i < inputCt; ++i) {
		auto inp = getInput(i);
		if (inp->status == -1)
//			return
			;
		if (inp->status == 1)
			willOutput = true;
	}
	
	/* Send our signal or lack thereof to each outlet from this node */
	for (int j = 0; j < outputCt; ++j) {
		auto outp = getOutput(j);
		if (!outp)
			continue;
		outp->status = willOutput ? 1 : 0;
		
		/* Alert the next node in the chain to do the same */
		auto state = State::getSelf();
		if (state->animateFlow) {
			ICNodeWkPtr wkNode = outp->parent;
			state->timedMgr->addEventIf(tS(nodeID) + "op" + tS(j),
//			state->timedMgr->addEvent(
					State::flowAnimDelay,
			[wkNode]() {
				if (auto node = wkNode.lock()) {
					node->propagateOutput();
				}
			}, false);
		}
		else {
			if (auto parentSp = outp->parent.lock())
				parentSp->propagateOutput();
		}
	}
	
	if (willOutput)
		spr.setColor(circOnColor);
	else spr.setColor(circOffColor);
}
