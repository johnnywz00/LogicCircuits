# LogicCircuits

## C++/SFML computer logic circuit editor and visualizer

<img width="1728" height="1117" alt="Screenshot 2026-02-12 at 4 42 19 PM" src="https://github.com/user-attachments/assets/039f68c3-d21e-4163-b005-a63239fb21b8" />

### ABOUT THE PROJECT

As the subtitle implies, this program allows you to select logic gates (AND, OR, NOT, NOR, NAND, XOR), place them where you will on a blank circuit board, and then connect them with interconnects (pipelines, more or less). You can place on/off switches and output indicators at appropriate places, and when you switch to simulation mode, current will flow from every "on" switch down all of the paths determined by the logic gates, ending at one or more output indicators. The switches can be flipped mid simulation, with realtime updating of the current flow and output. 
<details>
<summary>What's a logic gate?</summary>
To take a step back, a logic gate is a collection of a small number of transistors. A transistor can be thought of as a sort of gate on a pipeline: when it's open, current flows through; closed, it doesn't. (One "end" of each transistor is always receiving a continual supply of current that is trying to "get out" the other end when the gate is opened. In the middle, a third line from elsewhere will "push" the gate shut when there's current, or the gate will "spring back open" when that line is off.) When you arrange/connect the transistors in just the right way, interesting things can be done with them. For the particular clump of transistors known as an AND gate, you have two "pipes" coming in from elsewhere, and one pipe going out. The way the little transistor-gates are connected with each other, current (a 1 rather than 0) will only ever flow out of the AND gate if current is presently flowing in Incoming #1 AND Incoming #2. There is an OR gate which, yes, sends a 1 out if Incoming #1 OR Incoming #2 has current flowing. 
The NOT gate is only a single transistor, and whatever signal flows into it, the opposite will flow out.
An XOR (exclusive-OR) gate outputs 1 only if both of its inputs are different.
NAND (NOT AND) outputs 1 at all times *except* when both inputs are 1. (Interesting fact: although it would be an inefficient use of transistors, you could create an entire computer that functions only on NAND gates.)
NOR (NOT OR) outputs 1 *only* when both inputs are 0. 
</details>

Long ago, I was fascinated by reading the section in David Macaulay's *The Way Things Work* about transistors and logic gates. Much more recently, I took myself through a Computer Science textbook since it wasn't something I had formally broached before. Doing so rekindled the old fascination and also expanded my awareness of how many different functions of a computer were made up of the same logic gates that Macaulay's adder was. Whereas coding itself was (and is) usually the arena where I ply myself to reason through and discover the answer to a challenge without consulting existing resources or paradigms, here was a new type of logical challenge where I could take the six standard gate types and try to figure out how to arrange them to create a circuit that did something other than add (for adding was the only illustrated circuit in Macaulay, and even that I couldn't remember the details of).
So I must needs make a C++ simulator based on my new knowledge: getting, first, the challenge of coding the program to work, and secondly, the challenge of using that program to successfully arrange and connect logic gates to accomplish the purpose of a given circuit (say, subtraction this time), a basic list of which I was made acquainted with by the textbook. Before moving on to other study topics, I covered adder, subtractor, equality comparator, greater/less than comparator, multiplexer, decoder, odd parity, "majority rules", and oscillator. The S-R latch I think was mostly copied from a diagram. 
<details>
<summary>What those mean</summary>
**Multiplexer:** The multiplexer receives signals/input from multiple sources at the same time, but only transmits the status of one of them; it chooses which by also receiving a certain number of inputs that represent a number, where the number represents which line to transmit. For instance, if the multiplexer has 8 input lines coming in from elsewhere, it will also have 3 more input lines to constitute the selector, because 3 bits are sufficient to represent 8 different numbers in binary. If 011 (binary 3) is coming in through the selector lines, the multiplexer will output the status of Incoming Line #3, be it a 1 or 0.
**Decoder:** The decoder is close to being an opposite of the multiplexer. Instead of many lines coming in, it has many lines going out. It has inputs representing a number like a multiplexer does; this time, the number means "Send a 1 down Line #[this number] and 0s down every other output line." Thus numbers can be used like identification codes to send the control flow to different parts of the computer circuitry. 
**Odd parity:** This circuit reads a group of input lines and "counts" the total number of 1s. If the amount is an even number, the circuit outputs a 1, if odd, a 0. This is often used for error checking over long-range transmissions where bits could possibly become garbled by physical factors during the "journey". The "parity bit" from this circuit is added to each packet of data, so that each data packet sent is assured to contain an odd number of 1s. The receiver knows something went wrong if one of its packets comes in with an even number of 1s, and can request the packet again.
**Majority rules:** The circuit will output a 1 only if there are more 1s than 0s amongst its inputs. In addition to more commonplace uses, this can be used in safety-critical situations: A signal can be sent through three identical modules, all feeding out into the majority circuit. Under normal circumstances these will all produce the same signal, but in the event that one module malfunctions and produces a spurious signal, the majority checker will keep the system running based on the correct output of the other two modules.
**Oscillator:** Here a chain of gates send their output back to their input (as well as a branch that goes onward), creating a cycle that is continually switching the output off and on. 
**S-R Latch:** (Set/reset latch) This is the basic building block of "memory" in a computer processor: while electrical pulses are typically racing around and changing or disappearing at light speed, this circuit "captures" a 1 or 0 state indefinitely by using a feedback loop (similar to the oscillator). Its inputs can determine whether to "set" it to 1 or "reset" it to 0, and it will output the value that is currently "trapped" in the feedback loop. When many of these are used together (with some extra sophistications to align them with the computer's pulse clock), the processor can store whole numbers for use in complicated calculations without using a hard disk or RAM memory. 
</details>
I get a very geekish satisfaction watching the currents flow within a finished circuit upon flipping the input switches on or off. 

All of the graphics were hand-drawn in a pixel editor. 

### FILE DESCRIPTIONS
* **sfmlApp:**  Implements `main()` and the abstract app
* **state:**  Implements primary graphical elements, game logic
* **LogicGate:**  Handles graphical representation and simulation logic for logic gates
* **InterconnectNode:**  Handles graphical representation and simulation logic for interconnect nodes, the "tracks" that current flows along
* **CircuitTerminus:**  Specialization of InterconnectNode to handle the points where interconnects conceptually leave the circuit board
* **buttons:**  UI buttons for selecting tools in the editor
  
(From my "reusable modules" repo: https://github.com/johnnywz00/SFML-shared-headers)
* **jwz:**  C++ utility functions, #defines, shortcuts
* **jwzsfml:**  Like above, but SFML-specific
* **resourcemanager:**  Static class for accessing resource files globally
* **timedeventmanager:**  Manages fuses/daemons, delayed callbacks

### BUILDING INSTRUCTIONS
Ready-made program files are available on the Releases page of this repository, with versions for MacOS, Windows, and Linux. NO INSTALLATION NECESSARY: just download and double-click. If your OS isn't supported by the pre-made versions, or if you have other reasons for building from source:
- Clone this repository, and navigate to the root folder of that clone in a terminal window.
- Run:
<pre>
   cmake -B build
   cmake --build build --parallel
</pre>
