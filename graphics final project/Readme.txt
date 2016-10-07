
/*
 *  Radhika Mahadev Paryekar
 *  CSCI 4229/5229
 *  Project Review
 *  Student ID Number : 105739854
 *
 *  source code : finalproject.c
 *  Executable : finalproject

 *  Car Game Simulation
 *
 *  Display 1:
 *  Game of cars
 *  Press Space to enter Display 2 and select your car
 *  It also displays the controls for playing the game (to be used once in display 3: game scene)
 *  
 *  Display 2:
 *  Press 1 or 2 to select your car
 *  Press Enter once you have selected the car
 *
 *  Display 3 (game scene):
 *  You have entered the game
 *  Press up/down/right/left arrow to navigate the car
 *  Press z/x to increase decrease speed of car
 *  Press +/- to zoom in zoom out
 *  Press PgUp/PgDown- to view the scene from top/return to ground
 *  If you collide against any of the road pylons, collision is detected and game is over
 *  Also if the car goes off the road, you enter a new display saying "car out of bounds" and game over
 *  Once the car reaches the end of the scene the car gets translated back to the beginning, 
    and you can continue playing in a loop till u collide with any of the road pylons.

 *  Display 4 ********#####(MUST SEE)#### ********
 *  When the car collides with the road pylons, the game enters a new display saying "Collision detected" and "game over"
 *  Implemented collision detection with bounding circles algorithm
 *
 *  Display 5 ********#####(MUST SEE)#### ********
 *  When the car goes off the road, the game enters a new display showing "Car out of bounds" and "game over"
 *
 *
 *  Press ESC to exit the game or r/R to restart
 *
 *  Key bindings:
 *  Space      To enter display to select car
 *  1/2        To select a car
 *  Enter      To enter the game
 *  arrows     To navigate the car
 *  +/-        Zoom in and zoom out 
 *  PgDn/PgUp  Overhead view, PgUp to view the scene from top and pgDn to return to ground
 *  Z/X        To increase/decrease speed
 *  r/R        To restart the game
 *  ESC        Exit
 */
The 3D scene consists of a skycube with a road, trees, buildings at the side. 
Lamp posts, traffic signal, zebra crossing and road signs to give the feeling of motion. 
A dragon hovering overhead.
Road pylons scattered on the middle of the road as obstacles for collision detection.
Cant go off the road or collide with roadpylons or else game over.
