#ifndef LOGIC_CIRCUITS_H
#define LOGIC_CIRCUITS_H


#include "InterconnectNode.hpp"
#include "LogicGate.hpp"
#include "CircuitInput.hpp"
#include "buttons.hpp"

class SFGameWindow;
class TimedEventManager;

class CircuitTerminus;
class CircuitInput;
class CircuitOutput;
class LogicGate;

class State {
public:

//==============  BOILERPLATE =======================
//==============              ======================

	void onCreate ();
	
	void reset ();
	
	void debugTxtSetup ();
	
	void loadFonts ();
	
	void loadTextures ();
	
	void loadSounds ();
	
	void onMouseDown (int x, int y);
	
	void onMouseUp (int x, int y);
	
	void onKeyPress (Keyboard::Key);
	
	void onKeyRelease (Keyboard::Key);
	
	void update (const Time& time);
	
	void draw ();
	
	
	RenderWindow*  		 	w;
	SFGameWindow* 		 	gw;
	TimedEventManager*      timedMgr;
	int             	 	mx = 0,
	my = 0,
	mxOld = 0,
	myOld = 0;
	
	map<string, Font> 			fontMap;
	static const vector<pair<string, string>>
	fontList;
	
	map<string, Texture> 		txMap;
	static const vector<pair<string, string>>
	txList;
	
	vector<SoundBuffer> 		buffers;
	map<string, Sound> 			soundMap;
	static const vector<pair<string, string>>
	soundList;
	
	static State* getSelf () { return instance_; }
	static State* instance_;
//================== END BOILERPLATE =================
//====================================================
	
	
	vector<ICNodePtr> 					icNodes;
	vector<shared_ptr<CircuitTerminus>>	termini;
	vector<shared_ptr<LogicGate>> 		logicGates;
	vector<ICNodeButton> 				icButtons;
	vector<Textbox> 					labels;
	vector<RectangleShape> 				rects;
	vector<shared_ptr<Drawable>>		ghosts;
	vector<string> 						icToolTags {
		"elbow", "straight", "obranch", "mtee", "stee", "ibranch", "lelbow",  "circuitin", "circuitout", "not", "and", "nand", "or", "nor", "xor"
	};
	vector<vecf> 						gateOrigins {
		{21, 28}, {35, 28}, {49, 33}, {49, 29}, {49, 16}, {77, 29}
	};
	vector<IntRect> 					icToolTxRects {
		{0, 14, 14, 14},
		{14, 14, 14, 14},
		{14 * 2, 14, 14, 14},
		{14 * 3, 14, 14, 14},
		{14 * 4, 14, 14, 14},
		{14 * 5, 14, 14, 14},
		{14 * 6, 14, 14, 14},
		{0, 0, 20, 20},
		{20, 0, 20, 20}
	};
	multimap<VecfMM, ICNodePtr> 		gridLocs;
	
	static constexpr int baseCellSize = 14;
	Sprite				cursorSprite;
	RectangleShape		cursorShadow;
	RectangleShape		toolPane;
	bool 				isCursorVisible = true;
	string				curTool = "select";
	string				storedTool = "select";
	bool 				oldCursor = true;
	bool				draggingICTool = false;
	bool				drawingRect = false;
	vecf				lastCreateLoc;
	ICNodePtr 			clickDraggedIC {nullptr};
	shared_ptr<LogicGate> clickDraggedGate {nullptr};
	Textbox*			clickDraggedLabel {nullptr};
	VertexArray			gridLinesVtcl {Lines};
	VertexArray			gridLinesHztl {Lines};
	vecf				gridScale {2, 2};
	string 				mode = "edit";
	bool				useDirArrows = true;
	bool				animateFlow = true;

	Textbox				filenameTbox;
	Textbox*			activeTbox {nullptr};
	
	void toggleMode();
	
	void handleErase(int x, int y);
	
	int cellSize() { return baseCellSize * gridScale.x; }
	
	vecf alignToGrid(int mousex, int mousey)
	{
		return {
			float(mousex - mousex % cellSize() + cellSize() / 2),
			float(mousey - mousey % cellSize() + cellSize() / 2)
		};
	}
	
	vecf toGridPos(int mousex, int mousey)
	{
		return {float(mousex / cellSize()), float(mousey / cellSize())};
	}
	
	void showCursor(bool stat)
	{
		w->setMouseCursorVisible(stat);
		isCursorVisible = stat;
	}
	
	void redrawGrid();
	
	void setNodePosition(ICNodePtr& node, float x, float y);
		
	void removeNodeFromGrid(ICNodePtr& node)
	{
		auto eqr = gridLocs.equal_range(node->gridPos);
		for (auto itr = eqr.first; itr != eqr.second; ++itr) {
			if (itr->second == node) {
				gridLocs.erase(itr);
				break;
			}
		}
	}
	
	void propagateAll();
	
	void initializeNode(ICNodePtr&, string, int x, int y);
	
	void linkICNodes();
	
	void setTool(ICNodeButton&);
	void setTool(string);
	
	void createLabel(bool activate = true);
	
	char oppositeDirTo(char dir)
	{
		switch(dir) {
			case 'n': return 's';
			case 's': return 'n';
			case 'e': return 'w';
			case 'w': return 'e';
			default: break;
		}
		return dir;
	}
	
	void changeViewSize()
	{
		View vw {w->getView()};
		auto sz = vw.getSize();
		float ratio = sz.y / sz.x;
		vw.setSize(sz.x + 10 * (iKP(LShift) ? 1 : -1), sz.y + (10 * ratio) * (iKP(LShift) ? 1 : -1));
		w->setView(vw);
	}
	
	void saveCircuit();
	
	bool loadCircuit();
	
	void createNode(string type)
	{
		if (nodeFactoryMap.count(type))
			icNodes.push_back(nodeFactoryMap[type]());
		else cerr << "Node type not found. " << type << endl;
	}
	
	void createGate(string type)
	{
		if (gateFactoryMap.count(type))
			logicGates.push_back(gateFactoryMap[type]());
		else cerr << "Gate type not found. " << type << endl;
	}
	
	shared_ptr<Sprite> makeSpriteGhost(Sprite& src);

	
	static map<string, function<ICNodePtr(void)>> nodeFactoryMap;
	
	static map<string, function<shared_ptr<LogicGate>(void)>> gateFactoryMap;
	
	
	
////////////  DEBUG  /////////////////////

	Font    			 font;
	Text    			 mouseTxt,
						 debugTxt;

}; //end class State


#endif




