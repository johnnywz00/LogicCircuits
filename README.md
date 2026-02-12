# GAMEXXXXXXXXX

## C++/SFML GAMEDESCRIPTIONNNNNNN

(DRAGANDDROPSCREENSHOTHERE)

### ABOUT THE PROJECT
This game is one of many projects that I undertook in the early days of my programming self-teaching. The projects of this period were motivated primarily by the desire to know if I could "solve" the challenge of bringing this or that mechanic or concept into being through programming language, largely without consulting any other sources to see how it was done. The projects were characterized by a lack of concern for code organization, best practices, readability, or maintainability, because my free time was very limited and my chief objective was getting the satisfaction of knowing I got the idea to work. Most of the time, when I achieved that much of the goal, I was ready to move on to the next logical challenge rather than pore over the details of making a well-rounded product fit for public consumption. 
  
I've taken a little time to restructure some things before posting the code here; nonetheless, much about the program remains primitive or suboptimal as it was originally written. 

### FILE DESCRIPTIONS
* **sfmlApp:**  Implements `main()` and the abstract app
* **:**  Implements primary graphical elements; physics; game logic
* **objects:**  Small structs representing balls, pockets, etc.
  
(From my "reusable modules" repo: https://github.com/johnnywz00/SFML-shared-headers)
* **jwz:**  C++ utility functions, #defines, shortcuts
* **jwzsfml:**  Like above, but SFML-specific

### BUILDING INSTRUCTIONS
Ready-made program files are available on the Releases page of this repository, with versions for MacOS, Windows, and Linux. NO INSTALLATION NECESSARY: just download and double-click. If your OS isn't supported by the pre-made versions, or if you have other reasons for building from source:
- Clone this repository, and navigate to the root folder of that clone in a terminal window.
- Run:
<pre>
   cmake -B build
   cmake --build build
</pre>
