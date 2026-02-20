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
		/* Trying to use instant propagation instead of animation
		 * was causing stack overflow for certain circuits like
		 * S-R latch. Extra code could get around this, but for now
		 * instead of turning off animation we set it to a very
		 * low delay.
		 */
//		if (state->animateFlow)
		{
			ICNodeWkPtr wkNode = outp->parent;
			state->timedMgr->addEventIf(tS(nodeID) + "op" + tS(j),
					State::flowAnimDelay,
			[wkNode]() {
				if (auto node = wkNode.lock()) {
					node->propagateOutput();
				}
			});
		}
//		else {
//			if (auto parentSp = outp->parent.lock())
//				parentSp->propagateOutput();
//		}
	}
	
	if (willOutput)
		spr.setColor(circOnColor);
	else spr.setColor(circOffColor);
}
