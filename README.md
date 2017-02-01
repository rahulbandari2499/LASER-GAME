# LASER-GAME
A basic OPENGL Laser game with falling bricks. 

# EXTRA LIBRARIES REQUIRED
  * libao-dev
  * mpg123-dev
  
# HOW TO RUN
  * make
  * ./sample2D

# HOW TO PLAY

        1.Keyboard Controls:
		* Use keys a,d to tilt the canon up and down
		* Use keys s,f to move the canon up and down 
		* Use keys n,m to increase/decrease the speed of bricks falling
		* Use Ctrl+left and Ctrl+right to move the red basket left and right
		* Use Alt+left and Alt+right to move the green basket left and right
		* Use space bar to shoot the laser/bullet
		* Use left and right arrow keys to pan the screen
		* Use up and down keys to zoom in and zoom out

	2.Mouse Controls:
		* Can move any object by clicking on it and dragging
		* Left mouse button can be used to shoot the laser in the required direction
		* Use right mouse button to pan left/right

# RULES
    * Hit all the black blocks.
	  * Collect the red bricks in red basket and green bricks in green bakset 
	  * Letting any of the black bricks falling into baskets will end the game
	  * Hitting black bricks gives 5 points
	  * Hitting any other brick will reduce the score by 3 points(-3 points.)
	  * There are 5 chances allowed for wrong brick shooting.(5 misfires).
	  * There are 3 mirrors which reflect the laser.
	  * Score will be reduced by -3 point for not collecting the bricks in their respective baskets.
	  * score will be incresed by 5 points by collecting corresponding blocks in corresponding baskets.
	  * greater than 5 misfires=>GAME OVER
	  * black block collected=>GAME OVER
	 
# BEHAVIOUR OF OBJECTS

	* LASER GUN=>It can shoot laser beams for every 2 seconds.It can move on the wall attached to it vertically and it can rotate the gun in a range of -75 to 75 degrees.It shoots only BLACK blocks falling from top.

	* BASKETS=>there are two baskets of color red and green.they can translate in x direction only.they collect the corresponding color blocks falling from upwards.

	* MIRRORS=>they ONLY reflect the laser beam shooted from the laser gun using reflection laws of physics.THERE are three mirrors over here.IT is POLISHED on one side.

	* BLOCKS=>red,green,black blocks fall from the top and they get generated for every two seconds with a random x- coordinate.

	* WALLS=>There are three walls on left,top,right and they can absorb the laser beams.

	* SCORE BOARD=>IT gets updated on the basis of the rules.it is placed on the top-right corner of the window.

	* MISFIRE BOARD=>If the laser gun shoots the red or green blocks falling from the top,then misfire count increase.IF more than 5 misfires happen ,then YOU LOSE.it is placed on the top-left corner of the window.

	* LEVEL BOARD=>IT get updated on the basis of the score and the winning points increase accordingly to the level basis.


