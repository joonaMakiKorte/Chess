# Chess Engine Project

**TODO** Project description

## Features

- **C# WPF Frontend**: Modern graphical interface with intuitive controls
- **C++ Backend**: High-performance chess engine compiled as a DLL
- **Game Modes**:
  - Human vs. Human (local multiplayer)
  - Human vs. Computer (three difficulty levels)
- **AI Features**:
  - Minimax algorithm with alpha-beta pruning
  - Adaptive evaluation function that adjusts strategy beweeen midgame and endgame phases
  - Progressive difficulty levels that modify search depth and tactical awareness
    
The engine combines .NET's rich UI capabilities with C++'s computational efficiency for optimal chess gameplay.

## Screenshots

**TODO**

## Requirements
- **Microsoft Visual Studio 2022** for compatability with MSVC and modern C++ features (C++20).
- **C++ Runtime**: Ensure you have the **MSVC C++ runtime** installed for compiling C++ projects.

## Build Instructions for MSVC (Visual Studio)
1. **Clone the Repository**:
   ```bash
   git clone https://github.com/joonaMakiKorte/Chess.git
   cd Chess
   ```

2. **Open the Solution in Visual Studio**:
- Double-click on **`Chess.sln`** in the root of the repository to open the project in Visual Studio.

3. **Build the Project:**
- Once the solution is opened in Visual Studio, set the desired configuration to **`Release`** from the toolbar.
- To build the project:
  1. Click **Build** from the menu.
  2. Choose **Build Solution** (**`Ctrl + Shift + B`**).
- This will build both the **C++ engine** and the **C# UI**.

4. **Running the Application:**
- To run the application, set the **ChessUI** project as the **startup project** in Visual Studio:
  1. Right-click on **ChessUI** in Solution Explorer.
  2. Choose **Set as StartUp Project.**
- Then press **F5** or click **Start without Debugging** to run the application.

5. **Clean and Rebuild**: If you encounter any build issues, it's always good to clean the solution and rebuild:
- In Visual Studio, go to **Build -> Clean Solution** and then **Build Solution**.

6. **Verify the DLL Placement** (If still encountering issues):
- After building the project, verify that the **DLL** (**`ChessEngine.dll`**) has been copied to the correct location, which should be the
  **ChessUI/bin/x64/Release** folder (make sure you built in Release and not in Debug).

## Project Structure

### ChessEngine (C++)

- **`src/`**: Contains the main source code.
  - **`Bitboard.cpp`**: Contains main bitboard data and functions of the engine.
  - **`ChessAI.cpp`**: Handles the algorithms of AI opponent.
  - **`ChessEngineExports.cpp`**: Main entry point for C# to use the functions of DLL, like an API.
  - **`ChessBoard.cpp`**: A bridge between DLL entry point and bitboard to connect the two.
  - **`Moves.cpp`**: Handles move generation for bitboard + pinned piece and king danger computation.
  - **`MoveTables.cpp`**: Generates lookup tables for piece moves at initial DLL call.
  - **`Tables.cpp`**: Generates few utility lookup tables and allocates space on the heap for the AIs data tables at initial DLL call.
  - **`Magic.cpp`**: Handles magic bitboards for sliding piece move generation.
- **`include/`**: Header files.
  - **`Utils.hpp`**: Contains frequently called utility functions.
  - **`CustomTypes.hpp`**: Custom types to simplify chess programming.
  - **`BitboardConstants.hpp`**: Large amount of constants for the game. Handled at compile time.
  - **`Scoring.hpp`**: Adjustable scores used by AI for board state evaluation and move generation. Allows the tweaking of AI performance.
- **`pch.h/pch.cpp`**: Precompiled header to reduce compile time significantly.

### ChessUI (C#)
- **TODO**

## Chess Engine
The chess engine implements 64-bit bitboard logic, enabling highly optimized board operations through bitwise manipulation. 
Board states are communicated to the frontend via [FEN notation](https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation), which is parsed on the C# side.

For efficient data handling:
- Moves are encoded in 32-bit format for compact storage
- The bitboard class decodes and applies moves to the board
- State validation checks for check/mate/stalemate after each move
- Threefold repetition detection uses [Zobrist hashing](https://www.chessprogramming.org/Zobrist_Hashing), with hash keys updated incrementally via XOR

## AI

### Implementation
The AI combines advanced search algorithms with dynamic evaluation functions to determine optimal moves:
 - **Search algorithm**: Explores the game tree for optimal moves
 - **Evaluation function**: Assesses board states to guide decision-making

### Search algorithm
The core [minimax](https://en.wikipedia.org/wiki/Minimax) algorithm with alpha-beta pruning:
- Alternates turns recursively using apply-evaluate-undo sequences
- Limits search depth for practical evaluation times
- Prunes clearly disadvantageous branches for efficiency

### Evaluation function
Dual-phase evaluation system:

**Midgame**:
- Material and positional scoring
- [King safety](https://www.chessprogramming.org/King_Safety) metrics:
  - Pawn shield coverage
  - Nearby pawn storms
  - Open/semi-open king files
     
**Endgame**:
- Prioritizes king activity
- Rewards [passed pawns](https://en.wikipedia.org/wiki/Passed_pawn#:~:text=In%20chess%2C%20a%20passed%20pawn,same%20file%20or%20adjacent%20files.)
- Adjusts positional scoring with dynamic scaling

### Enhancements
**Seach Optimization**:
- [Quiescence-search](https://en.wikipedia.org/wiki/Quiescence_search) prevents horizon effects
- Delta-pruning limits Q-search depth
- [Transposition tables](https://www.chessprogramming.org/Transposition_Table) drastically reduce evaluation time

**Move Ordering**:
- [MVV-LVA](https://www.chessprogramming.org/MVV-LVA) prioritization
- [Killer-heuristics](https://www.chessprogramming.org/Killer_Move)
- [History-heuristics](https://www.chessprogramming.org/History_Heuristic#:~:text=a%20dynamic%20move%20ordering%20method,the%20move%20has%20been%20made.)
- Transposition table hints
- Tactical move bonuses
- Dynamic midgame-to-endgame transition weighting

**Endgame Specialization**:
- +1 search depth extension for checks
- Forced mate detection

### Benchmarks
- Estimated Elo: 2200+
- Performance: Up to 3 million nodes/sec at depth 5
- Response Time: <1 sec per move across all difficulties
  
*Performance achieved with MSVC Release build: Maximum Optimization (Favor Speed), Enhanced Instruction Set (AVX2)*

## Acknowledgements
- **[Chess Programming Wiki](http://chessprogramming.org/)**: For providing the main insights and theories of chess programming principles
