
#include "state.hpp"
#include "sfmlApp.hpp"
#include "CircuitInput.hpp"
#include "LogicGate.hpp"
#include "InterconnectNode.hpp"

State* State::instance_ = nullptr;

void State::debugTxtSetup ()
{
#ifdef EMBEDDED_HPP
	if (!loadByMethod(font, "resources/Monaco.ttf"))
#else
	if (!font.loadFromFile("resources/Monaco.ttf"))
#endif
		cerr << "Failed to load font Monaco.ttf" << endl;
	mouseTxt = Text("", font, 13);
	mouseTxt.sP(8, 9);
	mouseTxt.setFillColor(PURPLE);
		
	debugTxt = Text("", font, 13);
	debugTxt.sP(8, 25);
	debugTxt.setFillColor(Color::Blue);
}

void State::loadFonts ()
{
	fontMap.clear();
	string basePath = "resources/";
	Font f;
	forNum (int(fontList.size())) {
		string fileName = fontList[i].first;
		string filePath = basePath + fileName;
#ifdef EMBEDDED_HPP
		if (!loadByMethod(f, filePath))
#else
		if (!f.loadFromFile(filePath))
#endif
			cerr << "Couldn't load texture " << filePath << endl;
		else
			fontMap.insert({fontList[i].second, f});
	}
}

void State::loadTextures ()
{
	txMap.clear();
	string basePath = "resources/";
	Texture tex;
	forNum (int(txList.size())) {
		string fileName = txList[i].first;
		string filePath = basePath + fileName;
#ifdef EMBEDDED_HPP
		if (!loadByMethod(tex, filePath))
#else
		if (!tex.loadFromFile(filePath))
#endif
			cerr << "Couldn't load texture " << filePath << endl;
		else
			txMap.insert({txList[i].second, tex});
	}
}

void State::loadSounds ()
{
	soundMap.clear();
	buffers.clear();
	buffers.reserve(soundList.size());
	string basePath = "resources/";
	forNum (int(soundList.size())) {
		string fileName = soundList[i].first;
		string filePath = basePath + fileName;
		SoundBuffer sb;
#ifdef EMBEDDED_HPP
		if (!loadByMethod(sb, filePath))
#else
		if (!sb.loadFromFile(filePath))
#endif
			cerr << "Couldn't load sound file " << filePath << endl;
		else {
			buffers.push_back(sb);
			Sound s { buffers.back() };
			soundMap.insert({soundList[i].second, s});
		}
	}
}



void State::onCreate ()
{
	instance_ = this;
	
	debugTxtSetup();
	loadFonts();
	loadTextures();
	loadSounds();

	icButtons.clear();
	int firstGroupCt = 7;
	for (int i = 0; i < firstGroupCt; ++i) {
		icButtons.emplace_back(icToolTags[i], txMap["ic"], vecf(100 + (i * 38), 20),
							   vecf(i * 14, 14));
	}
	for (int i = 0; i < 2; ++i) {
		icButtons.emplace_back(icToolTags[i + firstGroupCt], txMap["inout"], vecf(105 + (firstGroupCt * 38) + i * 50, 25), vecf(i * 20, 0), true);
		icButtons.back().cursorOgn = {10, 13};
	}
	for (int i = 0; i < 6; ++i) {
		icButtons.emplace_back(icToolTags[i + firstGroupCt + 2], txMap["gates"], vecf(205 + (firstGroupCt * 38) + i * 50, 25), vecf((i % 4) * 20, (i / 4) * 20), true);
		icButtons.back().isGate = true;
		icButtons.back().cursorOgn = gateOrigins[i];
	}
	
	cursorShadow.setFillColor(Color(0, 0, 0, 30));
	toolPane.setSize({755, 55});
	toolPane.setFillColor(Color(0, 0, 0, 20));
		
	filenameTbox = Textbox(fontMap["default"], {1500, 25});
	
	icNodes.reserve(2200);
	termini.reserve(40);
	logicGates.reserve(50);
	labels.reserve(100);
	
	reset();
}


void State::reset ()
{
	InterconnectNode::resetNextID();
	icNodes.clear();
	termini.clear();
	logicGates.clear();
	gridLocs.clear();
	labels.clear();
	rects.clear();
	ghosts.clear();
	curTool = "select";
	mode = "edit";
	gridScale = {2, 2};
	redrawGrid();
	cursorSprite.setScale(gridScale);
	cursorShadow.setSize({baseCellSize * gridScale.x, baseCellSize * gridScale.x});
	centerOrigin(cursorShadow);
	activeTbox = nullptr;
	showCursor(true);
	timedMgr->reset();
}


void State::draw ()
{
	if (curTool != "select")
		w->draw(cursorShadow);
	w->draw(gridLinesVtcl);
	w->draw(gridLinesHztl);
	for (auto& gate : logicGates) {
		if (gate->isActive)
			gate->drawSupplyLines(w);
	}
	for (auto& rect : rects)
		w->draw(rect);
	w->draw(toolPane);
	for (auto& ghost : ghosts)
		w->draw(*ghost);
	for (auto& node : icNodes)
		if (node->isActive
			&& node->name != "gateinput" && node->name != "gateoutput"  //////
			)
			w->draw(*node);
	for (auto& gate : logicGates)
		w->draw(*gate);
	for (auto& label : labels)
		w->draw(label);
	for (auto& icb : icButtons)
		w->draw(icb.spr);
	if (mode == "edit" && curTool != "select" && curTool != "erase" && curTool != "move" && curTool != "makeRect") // // streamline
		w->draw(cursorSprite);
	w->draw(filenameTbox);
	w->draw(mouseTxt);
	w->draw(debugTxt);
}


void State::onMouseDown (int x, int y)
{
		/* Either mode */
	if (filenameTbox.tbox.gGB().contains(x, y)) {
		filenameTbox.setActive(true);
		activeTbox = &filenameTbox;
		return;
	}
	else if (filenameTbox.isActive) {
		filenameTbox.setActive(false);
		activeTbox = nullptr;
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
//				propagateAll();
				node->propagateOutput(); // seems to be working
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
		string s = "notnandnorxor";
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

		if (s.find(curTool) != s.npos)
			logicGates.back()->initialize(logicGates.back(), curTool, x, y, txMap, cursorSprite);
		else initializeNode(icNodes.back(), curTool, x, y);
			
		wasntICToolClick:
		if (curTool == "erase") {
			handleErase(x, y);
		}
		
		// remove "select" here if adding actual selection
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
					if (iKP(LShift))
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
	drawingRect = false;
}


void State::onKeyPress(Keyboard::Key k)
{
	switch(k) {
		case Keyboard::Escape:
			if (isCmdPressed())
				gw->close();
			else if (curTool != "select") {
				curTool = "select";
				showCursor(true);
			}
			break;
			
		case Keyboard::R:
			cursorSprite.rotate(iKP(LShift) ? -90 : 90);
			break;
		
		case Keyboard::F:
			cursorSprite.scale(iKP(LShift) ? 1 : -1, iKP(LShift) ? -1 : 1);
			break;
		
		case Keyboard::E:
			if (curTool == "erase")
				break;
			storedTool = curTool;
			oldCursor = isCursorVisible;
			curTool = "erase";
			showCursor(true);
			break;
		
		case Keyboard::W:
			if (curTool == "move")
				break;
			storedTool = curTool;
			oldCursor = isCursorVisible;
			curTool = "move";
			showCursor(true);
			break;
		
		case Keyboard::Q:
//			gridScale.x += iKP(LShift) ? -.1 : .1;
//			gridScale.y += iKP(LShift) ? -.1 : .1;
//			redrawGrid();
//			cursorSprite.setScale(gridScale);
//			for (auto& node : icNodes)
//				node->spr.setScale(gridScale);
			
				// needs more handling: cursorsprite, toolpane, panning, grid...
			changeViewSize();
			break;
		
		case Keyboard::S:
			setTool(*valWhich(icButtons,
								[&](auto& btn){ return btn.tag == "straight"; }));
			break;
		
		case Keyboard::A:
			setTool(*valWhich(icButtons,
								[&](auto& btn){ return btn.tag == "elbow"; }));
			break;
		
		case Keyboard::D:
			setTool(*valWhich(icButtons,
								[&](auto& btn){ return btn.tag == "lelbow"; }));
			break;
		
		case Keyboard::P:
			toggleMode();
			break;
		
		case Keyboard::U:
			animateFlow = !animateFlow;
			break;
		
		case Keyboard::Y:
			reset();
			break;
		
		case Keyboard::I:
			curTool = "makeRect";
			break;
		
		case Keyboard::J:
			saveCircuit();
			break;
		
		case Keyboard::L:
			loadCircuit();
			break;
		
		case Keyboard::Tab:
			filenameTbox.setActive(true); // cycle through all instead
			activeTbox = &filenameTbox;
			break;

//		case Keyboard::T:
//			if (mode == "edit")
//				createLabel();
//			break;

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
			curTool = storedTool;
			storedTool = "select"; //unnec?
			showCursor(oldCursor);
			break;
		
		default:
			break;
	}
}


void State::update (const Time& time)
{
	timedMgr->fireReadyEvents(time);
	
		/* Panning */
	View vw = w->getView();
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
		cursorSprite.move(dif);
		for (auto& icb : icButtons)
			icb.spr.move(dif);
		redrawGrid();
	}
	w->setView(vw);
		/* End panning */
	
	
	vecf alignedPos = alignToGrid(mx, my);
	cursorSprite.setPosition(alignedPos); // fix: only if ictool
	if (curTool != "select")
		cursorShadow.setPosition(mx, my);

	if (clickDraggedIC && (mx != mxOld || my != myOld))
		setNodePosition(clickDraggedIC, mx, my);
	if (clickDraggedGate && (mx != mxOld || my != myOld))
		clickDraggedGate->setPosition(mx, my);
	if (clickDraggedLabel && (mx != mxOld || my != myOld))
		clickDraggedLabel->setPosition({(float)mx, (float)my});
	
	if (draggingICTool && curTool == "straight" && alignedPos != lastCreateLoc) {
		icNodes.push_back(make_shared<ICStraightSeg>());
		initializeNode(icNodes.back(), "straight", mx, my);
		lastCreateLoc = alignedPos;
	}
	
	if (drawingRect && (mx != mxOld || my != myOld)) {
		rects.back().setSize(vecf(mx, my) - rects.back().getPosition());
	}
	
		// DEBUG/TESTING
	mouseTxt.setString(tS(mx) + ", " + tS(my));
	{
		ostringstream oss;
		oss<<"events size: "<<timedMgr->events.size()<<'\n';
		oss<<"eventtags size: "<<timedMgr->pendingTags.size()<<'\n';
		oss<<"ghosts size: "<<ghosts.size()<<'\n';
		for(auto& node : icNodes) {
			if(node->spr.gGB().contains(mx, my)) {
				oss<<node->name<<'\n'<<node->xformedStr <<"\ngridPos: "<<node->gridPos.vec.x<< ", " <<node->gridPos.vec.y<<"\nID: "<<node->nodeID<<'\n'<<"in1 status: "<<node->input1->status<<'\n';
				auto twoIn = dynamic_pointer_cast<TwoInputICNode>(node);
				if (twoIn)
					oss<<"in2 status: "<<twoIn->input2->status;
				
				oss<<"out1 ID: ";
				if (!node->output1.lock())
					oss<<"NULL\n";
				else oss<<node->output1.lock()->parent.lock()->nodeID<<"\nout1 status: "<<node->output1.lock()->status<<'\n';
				auto twoOut = dynamic_pointer_cast<TwoOutputICNode>(node);
				if (twoOut) {
					oss<<"out2 ID: ";
					if (!twoOut->output2.lock())
						oss<<"NULL\n";
					else oss<<twoOut->output2.lock()->parent.lock()->nodeID<<"\nout2 status: "<<twoOut->output2.lock()->status<<'\n';
				}
				break;
			}
		}
//		oss<<"labelCt: "<<labels.size()<<'\n'<<"tbox: ";
//		if (!activeTbox)
//			oss<<"NULL\n";
//		else
//			oss<<(long)activeTbox<<'\n'<<activeTbox->boxTxt.gP().x<<", "<<activeTbox->boxTxt.gP().y<<'\n'<<string(activeTbox->boxTxt.getString())<<'\n';
		debugTxt.setString(oss.str());
	}
	
} //end update



void State::linkICNodes()
{
	for (auto& node : icNodes) {
		string outputs = node->getOutputLocs();
		for (int i = 0; i < node->outputCt; ++i) {
			ICInputPtr dest;
			vecf outputDir;
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

void State::propagateAll()
{
	for (auto& term : termini)
		if (term->isActive && isOfKind<CircuitInput>(term)) {
			if (term->input1->status == -1)
				term->input1->status = 0;
			term->propagateOutput();
		}
}

void State::initializeNode(ICNodePtr& node, string name, int x, int y)
{
	auto terminus = dynamic_pointer_cast<CircuitTerminus>(node);
	if (terminus)
		termini.push_back(terminus);
	node->name = name;
	Sprite* spr = &node->spr;
	spr->setTexture(txMap[node->txMapKey]);
	spr->setTextureRect(icToolTxRects[indexOf(icToolTags, curTool)]);
	spr->setOrigin(cursorSprite.getOrigin());
	spr->setRotation(cursorSprite.getRotation());
	spr->setScale(cursorSprite.getScale());
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


void State::redrawGrid()
{
	gridLinesVtcl.clear();
	gridLinesHztl.clear();
	int rgb = 243, rgb100 = 227;
	Color c = Color(rgb, rgb, rgb);
	Color c10x = Color(rgb100, rgb100, rgb100);
	int cell10 = cellSize() * 10;
	auto vw = w->getView();
	int minx = vw.getCenter().x - vw.getSize().x / 2;
	int maxx = minx + vw.getSize().x;   // if zooming
	int miny = vw.getCenter().y - vw.getSize().y / 2;
	int maxy = miny + vw.getSize().y;   // if zooming
//	for (int x = 0; x <= scrw; x += cellSize()) {
	for (int x = minx - minx % cellSize(); x <= maxx; x += cellSize()) {
		gridLinesVtcl.append(Vertex(vecF(x, miny), !(x % cell10) ? c10x : c));
		gridLinesVtcl.append(Vertex(vecF(x, maxy), !(x % cell10) ? c10x : c));
	}
//	for (int y = 0; y <= scrh; y += cellSize()) {
	for (int y = miny - miny % cellSize(); y <= maxy; y += cellSize()) {
		gridLinesHztl.append(Vertex(vecF(minx, y), !(y % cell10) ? c10x : c));
		gridLinesHztl.append(Vertex(vecF(maxx, y), !(y % cell10) ? c10x : c));
	}
}

void State::setTool(ICNodeButton& icb)
{
	curTool = icb.tag;
	if (icb.isGate) {
		cursorSprite.setTexture(txMap[icb.tag]);
		auto sz = cursorSprite.getTexture()->getSize();
		cursorSprite.setTextureRect(IntRect(0, 0, sz.x, sz.y));
	}
	else {
		cursorSprite.setTexture(*(icb.spr.getTexture()));
		cursorSprite.setTextureRect(icb.spr.getTextureRect());
	}
	cursorSprite.setScale(gridScale);
	cursorSprite.setOrigin(icb.cursorOgn);
	cursorSprite.setRotation(0);
	showCursor(false);
}

void State::setTool(string tool)
{
	curTool = tool;
	showCursor(true);
}

void State::saveCircuit()
{
	static int saveCt = 0;
	string fname = "saved/circuit" + tS(++saveCt) + ".txt";
	auto btxt = filenameTbox.boxTxt.getString();
	if (!btxt.isEmpty())
		fname = "saved/" + string(btxt) + ".txt";
	ofstream fs{fname, std::ios_base::trunc};
	for (auto& node : icNodes) {
		if (node->name == "gateinput" || node->name == "gateoutput")
			continue;
		if (node->spr.getPosition().x < 0 && node->spr.getPosition().y < 0)
			continue;	// unnecessary if fixing erase handling
		Sprite* spr = &node->spr;
//		auto tr = spr->getTextureRect();
		fs
//			<< node->nodeID << ' '
			<< node->name << ' '
//			<< node->txMapKey << ' '
//			<< tr.left << ' '
//			<< tr.top << ' '
//			<< tr.width << ' '
//			<< tr.height << ' '
//			<< spr->getOrigin().x << ' '
//			<< spr->getOrigin().y << ' '
			<< spr->getRotation() << ' '
			<< spr->getScale().x << ' '
			<< spr->getScale().y << ' '
			<< spr->getPosition().x << ' '
			<< spr->getPosition().y << ' '
			<< '\n';
	}
	fs << ":\n";
	for (auto& gate : logicGates) {
		if (gate->spr.getPosition().x < 0 && gate->spr.getPosition().y < 0)
			continue;	// unnecessary if fixing erase handling
		fs
			<< gate->name << ' '
			<< gate->spr.getPosition().x << ' '
			<< gate->spr.getPosition().y << ' '
		// rotation?
			<< '\n';
	}
	fs << ":\n";
	for (auto& label : labels) {
		if (label.boxTxt.getPosition().x < 0 && label.boxTxt.getPosition().y < 0)
			continue;	// unnecessary if fixing erase handling
		fs
		<< string(label.boxTxt.getString()) << '\n'
		<< label.boxTxt.getPosition().x << ' '
		<< label.boxTxt.getPosition().y << ' '
		<< label.boxTxt.getCharacterSize() << ' '
		<< '\n';
	}
	fs.close();
	soundMap["save"].play();
}

bool State::loadCircuit()
{
	reset();
	
	string fname = "saved/circuit1.txt";
	auto btxt = filenameTbox.boxTxt.getString();
	if (!btxt.isEmpty())
		fname = "saved/" + string(btxt) + ".txt";
	std::ifstream circData;
	circData.open(fname);
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
		vecf scale;
		vecf pos;
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
			cursorSprite.setRotation(rot);
			cursorSprite.setScale(scale);
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
			// rotate cursorSprite if gate rotation added
			gate->initialize(gate, gate->name, pos.x, pos.y, txMap, cursorSprite);
		}
		else if (section == "label") {
			createLabel(false);
			auto& label = labels.back();
			label.boxTxt.setString(line); //disregard the stringstream for this line
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
		}
	}
	circData.close();
	setTool("select");
}

void State::handleErase(int x, int y)
{
	for (auto itr = icNodes.begin(); itr != icNodes.end(); ++itr) {
		if ((*itr)->spr.gGB().contains(x, y)) {
			(*itr)->isActive = false;
			
			ghosts.emplace_back(makeSpriteGhost((*itr)->spr));

			setNodePosition(*itr, -1000, -1000); // make sure fix works
			
//			removeNodeFromGrid(*itr);
//			auto term = dynamic_pointer_cast<CircuitTerminus>(*itr);
//			if (term) {
//				auto termItr = find(termini.begin(), termini.end(), term);
//				if (termItr != termini.end())
//					termini.erase(termItr);
//			}
//			itr = icNodes.erase(itr);
			break;	// only erase one node at a time if they're layered
		}
	}
	for (auto itr = logicGates.begin(); itr != logicGates.end(); ++itr) {
		if ((*itr)->spr.gGB().contains(x, y)) {
			(*itr)->isActive = false;
			
				/* Ghost */
			ghosts.emplace_back(makeSpriteGhost((*itr)->spr));

			(*itr)->setPosition(-1000, -1000); //HACK bc pointer err w/ erase
			
			//DID I FORGET REMOVE NODE FROM GRID:  GATES NOT ERASING PROPERLY
//			for (auto itr2 = icNodes.begin(); itr2 != icNodes.end(); ) {
//				auto gateptr = dynamic_pointer_cast<GateICNode>(*itr2);
//				if (gateptr && gateptr->parent.lock() == *itr)
//					itr2 = icNodes.erase(itr2);
//				else ++itr2;
//			}
//			logicGates.erase(itr);
			break;
		}
	}
	for (auto itr = labels.begin(); itr != labels.end(); ++itr) {
		Textbox& label = *itr;
		if (label.boxTxt.getGlobalBounds().contains(x, y)) {
			label.isActive = false;
			
			shared_ptr<Text> txt = make_shared<Text>(label.boxTxt);
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
			
			label.setPosition({-1000, -1000});  // make sure erase fix works
			activeTbox = nullptr;
			itr = labels.erase(itr);
			break;
		}
	}
}

shared_ptr<Sprite> State::makeSpriteGhost(Sprite& src)
{
	shared_ptr<Sprite> s = make_shared<Sprite>(src);
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

void State::toggleMode()
{
	if (mode == "edit" ) {
		mode = "simulate";
		for (auto& label : labels)
			label.setActive(false);
		if (!useDirArrows) {
			for (auto& node : icNodes) {
				if (!isOfKind<CircuitTerminus>(node)) {
					auto tr = node->spr.getTextureRect();
					tr.top -= 14;
					node->spr.setTextureRect(tr);
				}
			}
		}
		linkICNodes();
		propagateAll();
		showCursor(true);
	}
	else if (mode == "simulate") {
		mode = "edit";
		for (auto& node : icNodes) {
			node->spr.setColor(Color::White);
			if (!useDirArrows) {
				if (!isOfKind<CircuitTerminus>(node)) {
					auto tr = node->spr.getTextureRect();
					tr.top += 14;
					node->spr.setTextureRect(tr);
				}
			}
		}
	}
}

void State::createLabel(bool activate)
{
	labels.emplace_back(fontMap["label"], vecf{(float)mx, (float)my}, 24);
	labels.back().onlyShowText = true;
	if (activate) {
		labels.back().isActive = true;
		activeTbox = &labels.back();
	}
	//FIX: currently a TextEntered event is sent after the "T" key press to create label, making a "t" appear in the new label string
}

const vector<pair<string, string>> State::fontList
{
	{ "Monaco.ttf", "default" },
	{ "Abadi MT Condensed Extra Bold", "label" }
};

const vector<pair<string, string>> State::txList
{
	{"interconnect.png", "icSmall"},
	{"ic.png", "ic"},
	{"inout.png", "inout"},
	{"gates.png", "gates"},
	{"not.png", "not"},
	{"and.png", "and"},
	{"nand.png", "nand"},
	{"or.png", "or"},
	{"nor.png", "nor"},
	{"xor.png", "xor"}
};

const vector<pair<string, string>> State::soundList
{
	{ "savefile.mp3", "save" }
};

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
