# FPS Game in Console

## Description
This is a simple FPS (First-Person Shooter) game implemented in C++ for the Windows console. The game renders a 3D environment using ASCII characters and allows the player to navigate through a map, interacting with objects such as walls, floor, and ceiling. The game is designed to demonstrate basic principles of 3D graphics, raycasting, and game object management in a console environment.

## How to Run
0. Make sure you are using Windows.
1. Clone the repository.
2. Make sure your terminal has a size of 120 x 40.
3. Compile the code by running `g++ -std=c++11 *.cpp -o ./build/main` in the terminal from the root directory.
4. Start the game by running `./build/main`.

## Game Structure
### main.cpp
The `main.cpp` file contains the core logic of the game, including the game loop, input handling, and rendering.

#### Classes
1. **GameObject**: The base class for all game objects, providing common properties and methods such as position, character representation, color, speed, and movement.

2. **Cube**: A derived class from `GameObject`, representing a cube in the game world. It includes methods for detecting intersections with rays and determining the character representation based on the distance from the player.

3. **Floor**: Another derived class from `GameObject`, representing the floor. It has methods for intersection detection and character representation.

4. **Ceiling**: Similar to `Floor`, but represents the ceiling.

5. **Player**: A class representing the player, including properties for position, movement speed, and angle of view. It handles player-specific interactions like movement and rotation.

6. **Game**: The main class that handles the overall game logic, including setting up the console, initializing the map, building the world from the map, handling input, updating the game state, and rendering the scene.

### geometry.cpp
The `geometry.cpp` file contains the implementation of basic geometric constructs used in the game.

#### Classes
1. **Vector3D**: A class representing a 3D vector, including methods for normalization, dot product, vector addition, subtraction, scaling, and comparison.

2. **Line**: A class representing a line in 3D space, defined by a point and a direction. It includes methods for calculating points on the line and determining the parameter for a given point.

3. **Plane**: A class representing a plane in 3D space, defined by a normal vector and a point. It includes methods for finding the intersection point with a line.

### geometry.h
The `geometry.h` file contains the declarations of the geometric classes used in the game.

## Game Mechanics
### Map and Objects
- The game world is represented as a grid map, with different characters representing various objects. The map is defined in the `setUpMap` method of the `Game` class.
- The game world is built from the map using the `buildWorldFromMap` method, which converts map characters into game objects like cubes, floor, and ceiling.

### Rendering
- The game uses a simple raycasting technique to render the 3D scene onto the console screen. Rays are cast from the player's position through each pixel of the screen, and intersections with game objects are calculated to determine what is visible.
- The `render` method in the `Game` class handles the raycasting and drawing of objects based on their distance from the player, using different ASCII characters for visual depth cues.

### Input Handling
- Player movement and rotation are controlled using the `W`, `S`, `A`, `D` keys for forward, backward, left camera turn, right camera turn, respectively.
- Input handling is performed in the `handleInput` method of the `Game` class, which adjusts the player's position and view angle based on the keys pressed.

## Conclusion
This FPS game in the console is a basic demonstration of 3D rendering and game development principles using C++ and ASCII art. It provides a foundation for further development and experimentation with more complex game mechanics and rendering techniques.
