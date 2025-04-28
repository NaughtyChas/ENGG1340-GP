# ENGG1340-GP
Title: **《Rebirth : Me Delivering Keeta in Doomsday》**  
_*Game built for ENGG1340 group project._  

## GROUP MEMBER ##
*
* 
* ZhangQi 3036292697
* WuZijie 3036296930
* Zhu Lianbi 3036296899
*

## GAME DESCRIPTION ##
**Background Story**  
  In the blink of an eye, the world as you knew it ended.  
  One minute, you were just a normal student, bored out of your mind in class, scrolling through your phone and ordering "Keeta Takeout". The next minute—  _BOOM_. Your brain short-circuited, and the world went black...  
  You woke up to the sound of the earth splitting open, your face pressed against cracked asphalt, the sky a sickly shade of blood-red, with strange walls and obstacles rising everywhere.  
  "What the—?!"  
  Cities crumbled, governments fell, and the remnants of humanity scattered into the wastelands, fighting for survival against mutated horrors and rogue machines. The most precious resource? Not just food or weapons— but connection.  
  Your phone buzzes violently. Pulling it out with trembling hands, a notification flashes:  
  _// "Packages remaining today: 3" //_  
  There is a map shown the location of packages and delivery location.  
  You look down. The uniform clings to your body— neon orange, with the Keeta logo smeared.  
  In this fractured world, every package you deliver is a thread binding humanity together: medicine for the sick, letters from lost loved ones, hope in a dying world.  
  They’re lifelines in this apocalyptic wasteland.  
  Your heart beats, faster and faster.  
  Move. Move! Time is running out. Every second wasted is another name crossed off the list. Your muscles burn, your lungs scream— but you must keep going.  
  Your mission? Deliver the packages against all odds— before the world collapses entirely.  

## DESCRIPTION FOR THE GAME ##
* Brief Introduction:
  - This is a text-based single-player puzzle graphics game. Your goal is to transport more goods to the location with limited stamina.
  - Players need to need to choose the right path to avoid obstacles. As well as choose the right time to take and put the goods.
  - The game doesn't have too many restrictions, so you can use your imagination to get through the levels. When the game ends, you will receive a score related to your game performance.
* Detailed Rule Expalnation:
  - Players gain a certain amount of stamina at the beginning of each game, and recover a certain amount of stamina every time they pass a level.
  - The player's goal is to deliver all parcels to the appropriate receiving location in each level before running out of stamina, then leave through the exit and retain as much stamina as possible until the next level!
  - When a player runs out of stamina, the game is over and the player is awarded the final score.
  - Players can take more than one parcel, there will be more stamina consumption; any number of parcels can be put down at any time.
  - There are grids on the map with different effects that cause extra stamina gains and losses (supply stations and speed bumps).
  - Players can choose the difficulty (easy, medium, hard) at the beginning of each game, and the difficulty of the level will not change after the choice; the difficulty is related to the size of the map, the number of obstacles (5, 6, 7), and the number of packages.
* Specific Value Setting：
  - Initial overall stamina: 200.
  - Stamina replenished after each level is cleared: 50.
  - Stamina consumption:
      + No parcels：each step -1.
      + With n parcels📦: each step -(1+n).
      + speed bumps🚧🚧🚧: each step * 2.
  - supply stations🏪: + random 20~50.
* Map Generation:
  - Map size: 9x9 (easy), 11x11 (normal), 13x13 (hard).
  - Starting point🚪: left middle-most grid → Ending point🚪: right middle-most grid.
  - Number of packages: 3 (easy), 4 (normal), 5 (difficult).
  - Number of obstacles： （Difficulty related）easy (5), normal (6), difficult (7)。
* Calculation Of the Final Score: including the number of parcels delivered, the number of levels cleared, the time taken to clear the level, and so on (optional factor: the number of optimal paths).
* Game Features:
  - The start screen will show the historical score.
  - The main interface will clearly display the map, player status, operation instructions, etc.
  - Players can freely choose the difficulty. Difficulty will be linked to several factors of the game to enhance gameplay.
  - Players can pause or resume the previous game at any time.
  - The game has a built-in timer system to synchronise the time with reality.
  - Players have a high degree of flexibility and freedom to organise their routes on the map.

## DESCRIPTION FOR THE CODE ##


