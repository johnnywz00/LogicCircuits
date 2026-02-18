#ifndef LOGIC_CIRCUITS_H
#define LOGIC_CIRCUITS_H

#include "jwzsfml.hpp"
#include "timedeventmanager.hpp"
#include "resourcemanager.hpp"

#include "InterconnectNode.hpp"
#include "LogicGate.hpp"
#include "CircuitInput.hpp"
#include "buttons.hpp"


class FullscreenOnlyApp;


class State
{
public:
	static constexpr int baseCellSize = 14;

	static State* getSelf () { return instance_; }

/* Methods called by FullscreenOnlyApp */
	void onCreate ();
	
	bool handleTextEvent (Event&);
	
	void onMouseDown (int x, int y);
	
	void onMouseUp (int x, int y);
	
	void onKeyPress (Keyboard::Key);
	
	void onKeyRelease (Keyboard::Key);
	
	void update (const Time& time);
	
	void draw ();
	
/* Methods called by editor classes */
	vecF toGridPos (int mousex, int mousey)
	{
		return {float(mousex / cellSize()), float(mousey / cellSize())};
	}
	
	vecF alignToGrid (int mousex, int mousey)
	{
		return {
			float(mousex - mousex % cellSize() + cellSize() / 2),
			float(mousey - mousey % cellSize() + cellSize() / 2)
		};
	}
	
	void removeNodeFromGrid (ICNodePtr& node);

	
	RenderWindow*  		 			rwin;
	FullscreenOnlyApp* 				app;
	TimedEventManager*      		timedMgr;
	
	vector<ICNodePtr> 				icNodes;
	multimap<VecfMM, ICNodePtr> 	gridLocs;

	string 							mode;
	vecF							gridScale {2, 2};
	vecI							oldMouse
									, mouseVec
	;
	bool							animateFlow = true;

private:
	const vector<string> 	icToolTags {
		"elbow", "straight", "obranch", "mtee", "stee", "ibranch", 
		"lelbow",  "circuitin", "circuitout", "not", "and", "nand",
		"or", "nor", "xor"
	};
	const vector<vecF> 		gateOrigins {
		{21, 28}, {35, 28}, {49, 33},
		{49, 29}, {49, 16}, {77, 29}
	};
	const vector<IntRect> 	icToolTxRects {
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
	

	int cellSize () { return baseCellSize * gridScale.x; }
	
	char oppositeDirTo (char dir)
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

	void debugTxtSetup ();
	
	void reset ();
	
	void toggleMode ();
	
	void showCursor (bool stat)
	{
		rwin->setMouseCursorVisible(stat);
		isCursorVisible = stat;
	}
	
	void redrawGrid ();
	
	void setTool (ICNodeButton&);
	void setTool (string);
	
	void createNode (string type)
	{
		if (nodeFactoryMap.count(type))
			icNodes.push_back(nodeFactoryMap[type]());
		else cerr << "Node type not found. " << type << endl;
	}
	
	void createGate (string type)
	{
		if (gateFactoryMap.count(type))
			logicGates.push_back(gateFactoryMap[type]());
		else cerr << "Gate type not found. " << type << endl;
	}
	
	void initializeNode (ICNodePtr&, string, int x, int y);
	
	void setNodePosition (ICNodePtr& node, float x, float y);
	
	void linkICNodes ();
	
	void createLabel (bool activate = true);
	
	void handleErase (int x, int y);
	
	shared_ptr<Sprite> makeSpriteGhost (Sprite& src);
	
	void propagateAll ();
		
	void changeViewSize ();
	
	bool loadCircuit ();
	
	void saveCircuit ();
	

	static inline State* 			instance_ = nullptr;
	static map<string, function<ICNodePtr(void)>>
									nodeFactoryMap;
	static map<string, function<GatePtr(void)>>
									gateFactoryMap;
	
	vector<TerminusPtr>				termini;
	vector<GatePtr> 				logicGates;
	vector<ICNodeButton> 			icButtons;
	vector<Textbox> 				labels;
	vector<RectangleShape> 			rects;
	vector<shared_ptr<Drawable>>	ghosts;
	Sprite							cursorSprite;
	RectangleShape					cursorShadow;
	RectangleShape					toolPane;
	Textbox							filenameTbox;
	Text    						mouseTxt
									, debugTxt
	;
	VertexArray						gridLinesVtcl {Lines};
	VertexArray						gridLinesHztl {Lines};

	ICNodePtr 						clickDraggedIC {nullptr};
	GatePtr 						clickDraggedGate {nullptr};
	Textbox*						clickDraggedLabel {nullptr};
	Textbox*						activeTbox {nullptr};

	string							curTool;
	string							storedTool;
	vecF							lastCreateLoc;
	bool 							isCursorVisible = true;
	bool 							oldCursor = true;
	bool							draggingICTool = false;
	bool							drawingRect = false;
	bool							useDirArrows = true;
}; //end class State

#endif




