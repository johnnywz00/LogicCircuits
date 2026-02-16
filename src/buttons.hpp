//
//  buttons.hpp
//  LogicCircuits
//
//  Created by John Ziegler on 7/13/25.
//  Copyright © 2025 John Ziegler. All rights reserved.
//

#ifndef buttons_hpp
#define buttons_hpp

#include "jwzsfml.hpp"


class ICNodeButton
{
public:
	ICNodeButton( string tag_, const Texture& tx, vecf pos, vecf txRect, bool is20 = false)
		: tag(tag_)
	{
		spr.setTexture(tx);
		if (is20)
			spr.setTextureRect(IntRect(txRect.x, txRect.y, 20, 20));
		else spr.setTextureRect(IntRect(txRect.x, txRect.y, 14, 14));
		spr.setScale(2,2);
		centerOrigin(spr);
		spr.sP(pos);
		cursorOgn = spr.getOrigin();
	}
	
	vecf  cursorOgn;
	Sprite spr;
	string tag;
	bool isGate = false;
};





#endif /* buttons_hpp */
