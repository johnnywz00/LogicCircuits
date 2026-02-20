
#include "state.hpp"
#include "sfmlApp.hpp"


void State::onCreate ()
{
	instance_ = this;
	timedMgr->setCapacity(25000);
	rwin->setFramerateLimit(1000);
	
	gSound("inputOn").setVolume(20);
	gSound("inputOff").setVolume(50);
	debugTxtSetup();
	
	instrucsSpr.setTexture(gTexture("instrucs"));
	float factor = scrh / (instrucsSpr.getTexture()->getSize().y + 40);
	instrucsSpr.setScale(factor, factor);
	instrucsSpr.setPosition(20, 20);
	
	circListSpr.setTexture(gTexture("otherCircs"));
	circListSpr.setOrigin(circListSpr.gLB().width - 1, circListSpr.gLB().height / 2);
	circListSpr.setScale(factor, factor);
	circListSpr.setPosition(scrw, scrcy);

	instrucsBtn.setTexture(gTexture("instrucsBtn"));
	instrucsBtn.setScale(1.8, 1.2);
	centerOrigin(instrucsBtn);
	instrucsBtn.setPosition(scrw - 400, 35);
	
	instrBtnLabel = Text("Instructions", gFont("instr"), 14);
	centerOrigin(instrBtnLabel);
	instrBtnLabel.setPosition(instrucsBtn.gP() - vecF(0, 3));
	instrBtnLabel.setFillColor(CAPPUCCINO);
	
	flowDelayTxt = Text("Flow delay: .02", gFont("instr"), 18);
	centerOrigin(flowDelayTxt);
	flowDelayTxt.setPosition(instrucsBtn.gP() + pVec(230, 181));
	flowDelayTxt.setFillColor(DKAZURE);
	
	icButtons.clear();
	int firstGroupCt = 7;
	forNum (firstGroupCt) {
		icButtons.emplace_back(icToolTags[i], gTexture("interconnects"), vecF(100 + (i * 38), 20), vecF(i * 14, 14));
	}
	forNum (2) {
		icButtons.emplace_back(icToolTags[i + firstGroupCt], gTexture("termini"), vecF(105 + (firstGroupCt * 38) + i * 50, 25), vecF(i * 20, 0), true);
		icButtons.back().cursorOgn = {10, 13};
	}
	forNum (6) {
		icButtons.emplace_back(icToolTags[i + firstGroupCt + 2], gTexture("gateButtons"), vecF(205 + (firstGroupCt * 38) + i * 50, 25), vecF((i % 4) * 20, (i / 4) * 20), true);
		icButtons.back().isGate = true;
		icButtons.back().cursorOgn = gateOrigins[i];
	}
	
	cursorShadow.setFillColor(Color(0, 0, 0, 30));
	
	toolPane.setSize({755, 55});
	toolPane.setFillColor(Color(0, 0, 0, 20));
		
	filenameTbox = Textbox(gFont("debug"), {1500, 25});
	
	icNodes.reserve(2200);
	termini.reserve(40);
	logicGates.reserve(50);
	labels.reserve(100);
	
	reset();
}

bool State::handleTextEvent (Event& event)
{
	if (activeTbox
		&& (event.type == Event::TextEntered
			|| event.type == Event::KeyPressed
			|| event.type == Event::KeyReleased)) {
		if (event.type == Event::TextEntered) {
			if (event.text.unicode == 8)
				if (iKP(LShift))
					activeTbox->clear();
				else activeTbox->deleteLastChar();
				else if (event.text.unicode == 9) ; // Don't write the \t
				else activeTbox->appendText(event.text.unicode);
		}
		if (event.type == Event::KeyPressed
			&& (event.key.code == Keyboard::Escape || event.key.code == Keyboard::Enter)) {
			activeTbox->setActive(false);
			activeTbox = nullptr;
		}
		return true;
	}
	return false;
}

void State::onMouseDown (int x, int y)
{
	/* Either mode */
	if (displayInstr) {
		displayInstr = false;
		return;
	}
	else if (filenameTbox.tbox.gGB().contains(x, y)) {
		filenameTbox.setActive(true);
		activeTbox = &filenameTbox;
		return;
	}
	else if (filenameTbox.isActive) {
		filenameTbox.setActive(false);
		activeTbox = nullptr;
		return;
	}
	else if (instrucsBtn.gGB().contains(x, y)) {
		displayInstr = true;
		return;
	}
	
	/* Clicks during running mode */
	if (mode == "simulate") {
		for (auto& node : icNodes) {
			if (!isOfKind<CircuitInput>(node))
				continue;
			if (node->spr.gGB().contains(x, y)) {
				int cur = node->input1->status;
				node->input1->status = (cur == 1  ? 0 : 1);
				gSound(cur == 1 ? "inputOff" : "inputOn").play();
				node->propagateOutput();
			}
		}
		return;
	}
	
	/* Clicks during editor mode */
	bool clickedTool = false;
	for (auto& icb : icButtons) {
		if (icb.spr.gGB().contains(x, y)) {
			setTool(icb);
			clickedTool = true;
			break;
		}
	}
	if (!clickedTool) {
		/* Adding Interconnect pieces to the tableau */
		if (curTool == "elbow")
			icNodes.push_back(make_shared<ICElbow>());
		else if (curTool == "straight") {
			icNodes.push_back(make_shared<ICStraightSeg>());
			draggingICTool = true;
		}
		else if (curTool == "obranch")
			icNodes.push_back(make_shared<ICBranchOut>());
		else if (curTool == "ibranch")
			icNodes.push_back(make_shared<ICBranchIn>());
		else if (curTool == "mtee")
			icNodes.push_back(make_shared<ICMergeTee>());
		else if (curTool == "stee")
			icNodes.push_back(make_shared<ICSplitTee>());
		else if (curTool == "lelbow")
			icNodes.push_back(make_shared<ICLElbow>());
		
		else if (curTool == "circuitin")
			icNodes.push_back(make_shared<CircuitInput>());
		else if (curTool == "circuitout")
			icNodes.push_back(make_shared<CircuitOutput>());
		
		else if (curTool == "not")
			logicGates.push_back(make_shared<NotGate>());
		else if (curTool == "and")
			logicGates.push_back(make_shared<AndGate>());
		else if (curTool == "nand")
			logicGates.push_back(make_shared<NAndGate>());
		else if (curTool == "or")
			logicGates.push_back(make_shared<OrGate>());
		else if (curTool == "nor")
			logicGates.push_back(make_shared<NOrGate>());
		else if (curTool == "xor")
			logicGates.push_back(make_shared<XorGate>());
		else goto wasntICToolClick;
		
		if (gateNames.find(curTool) != string::npos)
			logicGates.back()->initialize(logicGates.back(), curTool, x, y, cursorSpr);
		else initializeNode(icNodes.back(), curTool, x, y);
		
	wasntICToolClick:
		if (curTool == "erase") {
			handleErase(x, y);
			lastEraseLoc = alignToGrid(x, y);
			draggingEraser = true;
		}
		
		else if (curTool == "move" || curTool == "select") {
			bool startingDrag = false;
			for (auto& node : icNodes) {
				if (isOfKind<GateICNode>(node))
					continue;
				if (node->spr.gGB().contains(x, y)) {
					clickDraggedIC = node;
					startingDrag = true;
				}
			}
			for (auto& gate : logicGates) {
				if (gate->spr.gGB().contains(x, y)) {
					clickDraggedGate = gate;
					startingDrag = true;
				}
			}
			
			bool settingActive = false;
			for (auto& label : labels) {
				if (label.boxTxt.gGB().contains(x, y)) {
					clickDraggedLabel = &label;
					startingDrag = true;
					label.setActive(true);
					activeTbox = &label;
					settingActive = true;
				}
				else label.setActive(false);
			}
			if (!settingActive) {
				activeTbox = nullptr;
				if (!startingDrag) {
					if (isShiftPressed())
						createLabel();
				}
			}
		}
		
		else if (curTool == "makeRect") {
			RectangleShape r;
			r.setOutlineColor(DKORANGE75);
			r.setOutlineThickness(1);
			Color c = ORANGE75;
			c.a = 60;
			r.setFillColor(c);
			r.setPosition(x, y);
			rects.push_back(r);
			drawingRect = true;
		}
	}
}


void State::onMouseUp (int x, int y)
{
	clickDraggedIC = nullptr;
	clickDraggedGate = nullptr;
	clickDraggedLabel = nullptr;
	draggingICTool = false;
	draggingEraser = false;
	drawingRect = false;
}


void State::onKeyPress(Keyboard::Key k)
{
	switch(k) {
		case Keyboard::Escape:
			if (isShiftPressed())
				app->close();
			else if (curTool != "select") {
				curTool = "select";
				showCursor(true);
			}
			break;
			
			/* Rotate the tool */
		case Keyboard::R:
			/* Logic gates can't be rotated */
			if (gateNames.find(curTool) == string::npos)
				cursorSpr.rotate(iKP(LShift) ? -90 : 90);
			break;
			
			/* Flip the tool mirror-fashion */
		case Keyboard::F:
			cursorSpr.scale(iKP(LShift) ? 1 : -1, iKP(LShift) ? -1 : 1);
			break;
			
			/* Eraser */
		case Keyboard::E:
			if (curTool == "erase")
				break;
			storedTool = curTool;
			oldCursor = isCursorVisible;
			curTool = "erase";
			showCursor(false);
			cursorSpr.setTexture(gTexture("eraser"));
			cursorSpr.setTextureRect(IntRect(0, 0, cursorSpr.getTexture()->getSize().x, cursorSpr.getTexture()->getSize().y));
			centerOrigin(cursorSpr);
			cursorSpr.setScale({1, 1});
			break;
			
			/* Move pieces */
		case Keyboard::W:
			if (curTool == "move")
				break;
			storedTool = curTool;
			oldCursor = isCursorVisible;
			curTool = "move";
			showCursor(true);
			break;
			
		case Keyboard::Q:
			changeViewSize();
			break;
			
			/* Straight section tool */
		case Keyboard::S:
			setTool(*valWhich(icButtons,
							  [&](auto& btn){ return btn.tag == "straight"; }));
			break;
			
			/* Elbow tool */
		case Keyboard::A:
			setTool(*valWhich(icButtons,
							  [&](auto& btn){ return btn.tag == "elbow"; }));
			break;
			
			/* Opposite elbow tool */
		case Keyboard::D:
			setTool(*valWhich(icButtons,
							  [&](auto& btn){ return btn.tag == "lelbow"; }));
			break;
			
			/* Switch modes */
		case Keyboard::M:
			toggleMode();
			break;
			
			/* Turn on/off current flow animation */
		case Keyboard::U:
			animateFlow = !animateFlow;
			if (!animateFlow) {
				storedAnimDelay = flowAnimDelay;
				flowAnimDelay = .001;
			}
			else {
				flowAnimDelay = storedAnimDelay;
			}
			break;
			
		case Keyboard::Y:
			reset();
			break;
			
		case Keyboard::I:
			curTool = "makeRect";
			break;
			
			/* Save to file */
		case Keyboard::J:
			saveCircuit();
			break;
			
			/* Load from file */
		case Keyboard::L:
			loadCircuit();
			break;
			
			/* Activate the filename textbox */
		case Keyboard::Tab:
			filenameTbox.setActive(true);
			activeTbox = &filenameTbox;
			break;

			/* Toggle debug stats */
		case Keyboard::Slash:
			if (isShiftPressed())
				showDbgTxt = !showDbgTxt;
			break;

		default:
			break;
	}
}


void State::onKeyRelease(Keyboard::Key k)
{
	switch(k) {
		case Keyboard::W:
			// reverting back from move tool: fall through unless new functionality added
		case Keyboard::E:
			if (indexOf(icToolTags, storedTool) != -1)
				setTool(*valWhich(icButtons,
								  [&](auto& btn){ return btn.tag == storedTool; }));
			else setTool(storedTool);
			showCursor(oldCursor);
			break;

		default:
			break;
	}
}

void State::update (const Time& time)
{
	timedMgr->fireReadyEvents(time);
	
	adjustVal(O, flowAnimDelay, .003, .001, .7);
	
	/* Panning */
	View vw = rwin->getView();
	auto oldPos = vw.getCenter();
	bool changedView;
	if (iKP(Left)) {
		vw.move(-5, 0);
		changedView = true;
		if (vw.getCenter().x < scrcx)
			vw.setCenter(scrcx, vw.getCenter().y);
	}
	if (iKP(Up)) {
		vw.move(0, -5);
		changedView = true;
		if (vw.getCenter().y < scrcy)
			vw.setCenter(vw.getCenter().x, scrcy);
	}
	if (iKP(Right)) {
		vw.move(5, 0);
		changedView = true;
	}
	if (iKP(Down)) {
		vw.move(0, 5);
		changedView = true;
	}
	if (changedView) {
		auto dif = vw.getCenter() - oldPos;
		toolPane.move(dif);
		cursorSpr.move(dif);
		for (auto& icb : icButtons)
			icb.spr.move(dif);
		redrawGrid();
	}
	rwin->setView(vw);
	/* End panning */
	
	vecF alignedPos = alignToGrid(mouseVec.x, mouseVec.y);
	bool toolCreatesSprite = indexOf(icToolTags, curTool) != -1;
	if (toolCreatesSprite) {
		cursorSpr.setPosition(alignedPos);
		cursorShadow.setPosition(mouseVec.x, mouseVec.y);
	}
	else cursorSpr.setPosition(mouseVec.x, mouseVec.y);
	
	if (clickDraggedIC && (mouseVec.x != oldMouse.x || mouseVec.y != oldMouse.y))
		setNodePosition(clickDraggedIC, mouseVec.x, mouseVec.y);
	else if (clickDraggedGate && (mouseVec.x != oldMouse.x || mouseVec.y != oldMouse.y))
		clickDraggedGate->setPosition(mouseVec.x, mouseVec.y);
	else if (clickDraggedLabel && (mouseVec.x != oldMouse.x || mouseVec.y != oldMouse.y))
		clickDraggedLabel->setPosition({(float)mouseVec.x, (float)mouseVec.y});
	
	else if (draggingICTool && curTool == "straight" && alignedPos != lastCreateLoc) {
		icNodes.push_back(make_shared<ICStraightSeg>());
		initializeNode(icNodes.back(), "straight", mouseVec.x, mouseVec.y);
		lastCreateLoc = alignedPos;
	}
	
	else if (draggingEraser && alignedPos != lastEraseLoc) {
		handleErase(mouseVec.x, mouseVec.y);
		lastEraseLoc = alignedPos;
	}
	
	else if (drawingRect && (mouseVec.x != oldMouse.x || mouseVec.y != oldMouse.y)) {
		rects.back().setSize(vecF(mouseVec.x, mouseVec.y) - rects.back().getPosition());
	}
	
	flowDelayTxt.setString("Flow delay: " + fS(flowAnimDelay, 3));
	
	/* DEBUG / TESTING */
	mouseTxt.setString(tS(mouseVec.x) + ", " + tS(mouseVec.y));
	{
		ostringstream oss;
		oss << " \n \n";
		oss << "events size: " << timedMgr->events.size() << '\n';
		oss << "eventtags size: " << timedMgr->pendingTags.size() << '\n';
		oss << "gridLocs size: " << gridLocs.size() << '\n';
		oss << "icNodes size: " << icNodes.size() << '\n';
		oss << "logicGates size: " << logicGates.size() << '\n';
		if (mode == "simulate")
			oss << "anim delay: " << fS(flowAnimDelay, 4)<< '\n';
		for(auto& node : icNodes) {
			if(node->spr.gGB().contains(mouseVec.x, mouseVec.y)) {
				oss << node->name << '\n' << node->xformedStr << "\ngridPos: " << node->gridPos.vec.x << ", " << node->gridPos.vec.y << "\nID: " << node->nodeID << '\n' << "in1 status: " << node->input1->status << '\n';
				auto twoIn = dynamic_pointer_cast<TwoInputICNode>(node);
				if (twoIn)
					oss << "in2 status: " << twoIn->input2->status << '\n';
				
				oss << "out1 ID: ";
				if (!node->output1.lock())
					oss << "NULL\n";
				else oss << node->output1.lock()->parent.lock()->nodeID << "\nout1 status: " << node->output1.lock()->status << '\n';
				auto twoOut = dynamic_pointer_cast<TwoOutputICNode>(node);
				if (twoOut) {
					oss << "out2 ID: ";
					if (!twoOut->output2.lock())
						oss << "NULL\n";
					else oss << twoOut->output2.lock()->parent.lock()->nodeID << "\nout2 status: " << twoOut->output2.lock()->status << '\n';
				}
				break;
			}
		}
//		oss << "labelCt: " << labels.size() << '\n' << "tbox: ";
//		if (!activeTbox)
//			oss << "NULL\n";
//		else
//			oss << (long)activeTbox << '\n' << activeTbox->boxTxt.gP().x << ", "
//			<< activeTbox->boxTxt.gP().y << '\n' << string(activeTbox->boxTxt.getString())
//			<< '\n';
		debugTxt.setString(oss.str());
	}
} //end update

void State::draw ()
{
	if (mode == "edit")
		editDraw();
	else if (mode == "simulate")
		simulateDraw();
	rwin->draw(instrucsBtn);
	rwin->draw(instrBtnLabel);
	if (displayInstr) {
		rwin->draw(instrucsSpr);
		rwin->draw(circListSpr);
	}
}

void State::removeNodeFromGrid(ICNodePtr& node)
{
	auto eqr = gridLocs.equal_range(node->gridPos);
	for (auto itr = eqr.first; itr != eqr.second; ++itr) {
		if (itr->second == node) {
			gridLocs.erase(itr);
			break;
		}
	}
}

void State::debugTxtSetup ()
{
	mouseTxt = Text("", gFont("debug"), 13);
	mouseTxt.sP(8, 9);
	mouseTxt.setFillColor(PURPLE);
	
	debugTxt = Text("", gFont("debug"), 13);
	debugTxt.sP(8, 25);
	debugTxt.setFillColor(Color::Blue);
}

void State::reset ()
{
	draggingICTool = false;
	draggingEraser = false;
	drawingRect = false;
	displayInstr = false;
	clickDraggedIC = nullptr;
	clickDraggedGate = nullptr;
	clickDraggedLabel = nullptr;
	activeTbox = nullptr;
	InterconnectNode::resetNextID();
	icNodes.clear();
	termini.clear();
	logicGates.clear();
	gridLocs.clear();
	labels.clear();
	rects.clear();
	ghosts.clear();
	mode = "edit";
	curTool = "select";
	storedTool = "select";
	gridScale = {2, 2};
	cursorSpr.setScale(gridScale);
	cursorShadow.setSize({baseCellSize * gridScale.x, baseCellSize * gridScale.x});
	centerOrigin(cursorShadow);
	activeTbox = nullptr;
	showCursor(true);
	timedMgr->reset();
	rwin->setView(rwin->getDefaultView());
	redrawGrid();
}

void State::toggleMode()
{
	if (mode == "edit" ) {
		mode = "simulate";
		for (auto& label : labels)
			label.setActive(false);
		linkICNodes();
		propagateAll();
		showCursor(true);
	}
	
	else if (mode == "simulate") {
		mode = "edit";
		timedMgr->reset();
		for (auto& node : icNodes) {
			node->spr.setColor(Color::White);
		}
	}
}

void State::redrawGrid()
{
	gridLinesVtcl.clear();
	gridLinesHztl.clear();
	int rgb = 243, rgb100 = 227;
	Color c = Color(rgb, rgb, rgb);
	Color c10x = Color(rgb100, rgb100, rgb100);
	int cell10 = cellSize() * 10;
	auto vw = rwin->getView();
	int minx = vw.getCenter().x - vw.getSize().x / 2;
	int maxx = minx + vw.getSize().x;   // if zooming
	int miny = vw.getCenter().y - vw.getSize().y / 2;
	int maxy = miny + vw.getSize().y;   // if zooming
	for (int x = minx - minx % cellSize(); x <= maxx; x += cellSize()) {
		gridLinesVtcl.append(Vertex(vecF(x, miny), !(x % cell10) ? c10x : c));
		gridLinesVtcl.append(Vertex(vecF(x, maxy), !(x % cell10) ? c10x : c));
	}
	for (int y = miny - miny % cellSize(); y <= maxy; y += cellSize()) {
		gridLinesHztl.append(Vertex(vecF(minx, y), !(y % cell10) ? c10x : c));
		gridLinesHztl.append(Vertex(vecF(maxx, y), !(y % cell10) ? c10x : c));
	}
}

void State::setTool(ICNodeButton& icb)
{
	curTool = icb.tag;
	if (icb.isGate) {
		cursorSpr.setTexture(gTexture(icb.tag));
		auto sz = cursorSpr.getTexture()->getSize();
		cursorSpr.setTextureRect(IntRect(0, 0, sz.x, sz.y));
	}
	else {
		cursorSpr.setTexture(*(icb.spr.getTexture()));
		cursorSpr.setTextureRect(icb.spr.getTextureRect());
	}
	cursorSpr.setScale(gridScale);
	cursorSpr.setOrigin(icb.cursorOgn);
	cursorSpr.setRotation(0);
	showCursor(false);
}

void State::setTool(string tool)
{
	curTool = tool;
	showCursor(true);
}

void State::initializeNode(ICNodePtr& node, string name, int x, int y)
{
	auto terminus = dynamic_pointer_cast<CircuitTerminus>(node);
	if (terminus)
		termini.push_back(terminus);
	node->name = name;
	Sprite* spr = &node->spr;
	spr->setTexture(gTexture(node->txMapKey));
	spr->setTextureRect(icToolTxRects[indexOf(icToolTags, curTool)]);
	spr->setOrigin(cursorSpr.getOrigin());
	spr->setRotation(cursorSpr.getRotation());
	spr->setScale(cursorSpr.getScale());
	node->setXformedString();
	setNodePosition(node, x, y);
	node->initInputs();
}

void State::setNodePosition(ICNodePtr& node, float x, float y)
{
	auto newPos = alignToGrid(x, y);
	if (node->gridPos.vec.x > -1 && node->gridPos.vec.y > -1)
		removeNodeFromGrid(node);
	node->spr.setPosition(newPos);
	node->gridPos = {toGridPos(x, y)};
	gridLocs.emplace(node->gridPos, node);
	if (curTool == "straight")
		lastCreateLoc = newPos;
	
	auto ptr = dynamic_pointer_cast<CircuitTerminus>(node);
	if (ptr) {
		ptr->txt.setPosition(newPos + pVec(4 * gridScale.x, 270 + ptr->spr.getRotation()));
	}
}

void State::linkICNodes()
{
	for (auto& node : icNodes) {
		string outputs = node->getOutputLocs();
		for (int i = 0; i < node->outputCt; ++i) {
			ICInputPtr dest;
			vecF outputDir;
			switch(outputs[i]) {
				case 'n': outputDir = { 0, -1}; break;
				case 'e': outputDir = { 1,  0}; break;
				case 's': outputDir = { 0,  1}; break;
				case 'w': outputDir = {-1,  0}; break;
			}
			auto eqr = gridLocs.equal_range(VecfMM(node->gridPos.vec + outputDir));
			for (auto itr = eqr.first; itr != eqr.second; ++itr) {
				bool shouldBreak = false;
				for (int j = 0; j < itr->second->inputCt; ++j) {
					if (itr->second->getInputLocs()[j] == oppositeDirTo(outputs[i])) {
						dest = itr->second->getInput(j);
						shouldBreak = true;
						break;
					}
				}
				if (shouldBreak)
					break;
			}
			node->setOutput(i, dest);
		}
	}
}

void State::createLabel(bool activate)
{
	labels.emplace_back(gFont("label"), vecF{(float)mouseVec.x, (float)mouseVec.y}, 24);
	labels.back().onlyShowText = true;
	if (activate) {
		labels.back().isActive = true;
		activeTbox = &labels.back();
	}
}

void State::handleErase(int x, int y)
{
	for (auto itr = icNodes.begin(); itr != icNodes.end(); ++itr) {
		auto sp = *itr;
		if (!sp)
			continue;
		if (sp->spr.gGB().contains(x, y)) {
			sp->isActive = false;
			ghosts.emplace_back(makeSpriteGhost(sp->spr));
			removeNodeFromGrid(sp);
			auto term = dynamic_pointer_cast<CircuitTerminus>(sp);
			if (term) {
				auto termItr = find(termini.begin(), termini.end(), term);
				if (termItr != termini.end())
					termini.erase(termItr);
			}
			icNodes.erase(itr);
			/* Only erase one node at a time if they're layered */
			break;
		}
	}
	for (auto itr = logicGates.begin(); itr != logicGates.end(); ++itr) {
		auto sp = *itr;
		if (!sp)
			continue;
		if (sp->spr.gGB().contains(x, y)) {
			sp->isActive = false;
			ghosts.emplace_back(makeSpriteGhost(sp->spr));
			for (auto itr2 = icNodes.begin(); itr2 != icNodes.end(); ) {
				auto gateptr = dynamic_pointer_cast<GateICNode>(*itr2);
				if (gateptr && gateptr->parent.lock() == *itr) {
					removeNodeFromGrid(*itr2);
					itr2 = icNodes.erase(itr2);
				}
				else ++itr2;
			}
			logicGates.erase(itr);
			break;
		}
	}
	for (auto itr = labels.begin(); itr != labels.end(); ++itr) {
		Textbox& label = *itr;
		if (label.boxTxt.getGlobalBounds().contains(x, y)) {
			label.isActive = false;
			/* Ghost the deleted text */
			TextPtr txt = make_shared<Text>(label.boxTxt);
			txt->setOutlineColor(Color::Transparent);
			timedMgr->addEvent(.016, [txt, this](FusePtr fuse) {
				Color c = txt->getFillColor();
				if (c.a >= 12) {
					c.a -= 12;
					txt->setFillColor(c);
				}
				else {
					ghosts.erase(find(ghosts.begin(), ghosts.end(), txt));
					timedMgr->removeEvent(fuse);
				}
			}, true);
			ghosts.push_back(std::move(txt));
			
			if (activeTbox)
				activeTbox->isActive = false;
			activeTbox = nullptr;
			labels.erase(itr);
			break;
		}
	}
}

SpritePtr State::makeSpriteGhost(Sprite& src)
{
	SpritePtr s = make_shared<Sprite>(src);
	timedMgr->addEvent(.016, [s, this](FusePtr fuse) {
		Color c = s->getColor();
		if (c.a >= 12) {
			c.a -= 12;
			s->setColor(c);
		}
		else {
			ghosts.erase(find(ghosts.begin(), ghosts.end(), s));
			timedMgr->removeEvent(fuse);
		}
	}, true);
	return s;
}

void State::propagateAll()
{
	for (auto& term : termini)
		if (term->isActive && isOfKind<CircuitInput>(term)) {
			if (term->input1->status == -1)
				term->input1->status = 0;
			term->propagateOutput();
		}
	/* This may be unnecessary except for the one case
	 * of a no-input oscillator
	 */
	for (auto& gate : logicGates)
		if (isOfKind<NotGate>(gate) || isOfKind<NOrGate>(gate))
			gate->propagateOutput();
}

void State::changeViewSize ()
{
	View vw {rwin->getView()};
	auto sz = vw.getSize();
	float ratio = sz.y / sz.x;
	vw.setSize(sz.x + 10 * (iKP(LShift) ? 1 : -1), sz.y + (10 * ratio) * (iKP(LShift) ? 1 : -1));
	rwin->setView(vw);
}

bool State::loadCircuit()
{
	reset();
	
	string fname = "circuit1.txt";
	auto btxt = filenameTbox.boxTxt.getString();
	if (!btxt.isEmpty())
		fname = stripExtension(string(btxt)) + ".txt";
	std::ifstream circData {Resources::executingDir() / "resources" / "saved" / fname};
	if (!circData.is_open()) {
		cerr << "Couldn't load saved file. \n";
		return;
	}
	string line;
	
	string section = "node";
	while (getline(circData, line)) {
		if (line.find(":") != line.npos) {
			if (section == "node")
				section = "gate";
			else if (section == "gate")
				section = "label";
			continue;
		}
		vecF scale;
		vecF pos;
		float rot;
		std::stringstream ss(line);
		string token;
		ss >> token;
		if (section != "label")
			setTool(*valWhich(icButtons, [&](auto& btn){ return btn.tag == token;}));
		if (section == "node") {
			createNode(token);
			auto& node = icNodes.back();
			node->name = token;
			ss >> token;
			rot = stof(token);
			ss >> token;
			scale.x = stof(token);
			ss >> token;
			scale.y = stof(token);
			ss >> token;
			pos.x = stof(token);
			ss >> token;
			pos.y = stof(token);
			cursorSpr.setRotation(rot);
			cursorSpr.setScale(scale);
			initializeNode(node, node->name, pos.x, pos.y);
		}
		else if (section == "gate") {
			createGate(token);
			auto& gate = logicGates.back();
			gate->name = token;
			ss >> token;
			pos.x = stof(token);
			ss >> token;
			pos.y = stof(token);
			gate->initialize(gate, gate->name, pos.x, pos.y, cursorSpr);
		}
		else if (section == "label") {
			createLabel(false);
			auto& label = labels.back();
			label.boxTxt.setString(line); // Disregard the stringstream for this line
			getline(circData, line);
			ss = stringstream(line);
			ss >> token;
			pos.x = stof(token);
			ss >> token;
			pos.y = stof(token);
			label.setPosition(pos);
			ss >> token;
			label.boxTxt.setCharacterSize(stoi(token));
			label.setPosition(pos - label.borderOffset);
			label.isActive = false;
		}
	}
	circData.close();
	setTool("select");
}

void State::saveCircuit()
{
	static int saveCt = 0;
	string fname = "circuit" + tS(++saveCt) + ".txt";
	auto btxt = filenameTbox.boxTxt.getString();
	if (!btxt.isEmpty())
		fname = stripExtension(string(btxt)) + ".txt";
	ofstream fs{Resources::executingDir() / "resources" / "saved" / fname, std::ios_base::trunc};
	for (auto& node : icNodes) {
		if (node->name == "gateinput" || node->name == "gateoutput")
			continue;
		Sprite* spr = &node->spr;
		fs
			<< node->name << ' '
			<< spr->getRotation() << ' '
			<< spr->getScale().x << ' '
			<< spr->getScale().y << ' '
			<< spr->getPosition().x << ' '
			<< spr->getPosition().y << ' '
			<< '\n';
	}
	fs << ":\n";
	for (auto& gate : logicGates) {
		fs
			<< gate->name << ' '
			<< gate->spr.getPosition().x << ' '
			<< gate->spr.getPosition().y << ' '
			<< '\n';
	}
	fs << ":\n";
	for (auto& label : labels) {
		fs
		<< string(label.boxTxt.getString()) << '\n'
		<< label.boxTxt.getPosition().x << ' '
		<< label.boxTxt.getPosition().y << ' '
		<< label.boxTxt.getCharacterSize() << ' '
		<< '\n';
	}
	fs.close();
	gSound("save").play();
}

void State::editDraw ()
{
	auto w = rwin;
	if (curTool != "select"
		&& curTool != "erase"
		&& curTool != "move")
		w->draw(cursorShadow);
	w->draw(gridLinesVtcl);
	w->draw(gridLinesHztl);
	for (auto& rect : rects)
		w->draw(rect);
	w->draw(toolPane);
	for (auto& ghost : ghosts)
		w->draw(*ghost);
	
	for (auto& node : icNodes)
		if (node->isActive
			&& node->name != "gateinput"
			&& node->name != "gateoutput" )
			w->draw(*node);
	for (auto& gate : logicGates)
		w->draw(*gate);
	for (auto& label : labels)
		w->draw(label);
	
	for (auto& icb : icButtons)
		w->draw(icb.spr);
	if (indexOf(icToolTags, curTool) != -1
		|| curTool == "erase")
		w->draw(cursorSpr);
	w->draw(filenameTbox);
	if (showDbgTxt) {
		w->draw(mouseTxt);
		w->draw(debugTxt);
	}
}

void State::simulateDraw ()
{
	auto w = rwin;
	for (auto& gate : logicGates) {
		if (gate->isActive)
			gate->drawSupplyLines(w);
	}
	for (auto& rect : rects)
		w->draw(rect);
	for (auto& node : icNodes)
		if (node->isActive
			&& node->name != "gateinput"
			&& node->name != "gateoutput" )
			w->draw(*node);
	for (auto& gate : logicGates)
		w->draw(*gate);
	for (auto& label : labels)
		w->draw(label);
	w->draw(flowDelayTxt);
	w->draw(filenameTbox);
	if (showDbgTxt)
		w->draw(debugTxt);
}

map<string, function<ICNodePtr()>> State::nodeFactoryMap {
	{"elbow", [](){ return make_shared<ICElbow>(); } },
	{"straight", [](){ return make_shared<ICStraightSeg>(); } },
	{"obranch", [](){ return make_shared<ICBranchOut>(); } },
	{"ibranch", [](){ return make_shared<ICBranchIn>(); } },
	{"mtee", [](){ return make_shared<ICMergeTee>(); } },
	{"stee", [](){ return make_shared<ICSplitTee>(); } },
	{"lelbow", [](){ return make_shared<ICLElbow>(); } },
	{"circuitin", [](){ return make_shared<CircuitInput>(); } },
	{"circuitout", [](){ return make_shared<CircuitOutput>(); } }
};

map<string, function<GatePtr()>> State::gateFactoryMap {
	{"and", [](){ return make_shared<AndGate>(); } },
	{"or", [](){ return make_shared<OrGate>(); } },
	{"not", [](){ return make_shared<NotGate>(); } },
	{"xor", [](){ return make_shared<XorGate>(); } },
	{"nor", [](){ return make_shared<NOrGate>(); } },
	{"nand", [](){ return make_shared<NAndGate>(); } }
};
