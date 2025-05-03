# 《Rebirth : Me Delivering Keeta in Doomsday》

> *Game built for ENGG1340 group project*  

## Group Member
* Meng Xiangkun 3036452675
  - Game developent, Project management
* 
* Zhang Qi 3036292697
* Wu Zijie 3036296930
* Zhu Lianbi 3036296899
* Zhao Jiayu 3036451322

## Game Descritption
### Background Story  
  In the blink of an eye, the world as you knew it ended.  
  One minute, you were just a normal student, bored out of your mind in class, scrolling through your phone and ordering "Keeta Takeout". The next minute—  _BOOM_. Your brain short-circuited, and the world went black...  
  You woke up to the sound of the earth splitting open, your face pressed against cracked asphalt, the sky a sickly shade of blood-red, with strange walls and obstacles rising everywhere.  
  "What the—?!"  
  Cities crumbled, governments fell, and the remnants of humanity scattered into the wastelands, fighting for survival against mutated horrors and rogue machines. The most precious resource? Not just food or weapons, but connection.  
  Your phone buzzes violently. Pulling it out with trembling hands, a notification flashes:  
  
> _// "Packages remaining today: 3" //_

  There is a map shown the location of packages and delivery location.  
  You look down. The uniform clings to your body— neon orange, with the Keeta logo smeared.  
  In this fractured world, every package you deliver is a thread binding humanity together: medicine for the sick, letters from lost loved ones, hope in a dying world.  
  They’re lifelines in this apocalyptic wasteland.  
  Your heart beats, faster and faster.  
  Move. Move! Time is running out. Every second wasted is another name crossed off the list. Your muscles burn, your lungs scream— but you must keep going.  
  Your mission? Deliver the packages against all odds— before the world collapses entirely.  

---

### Game Overview  
This is a text-based single-player puzzle graphics game where you navigate a post-apocalyptic world as a delivery courier, balancing stamina, obstacles, and strategy to complete your missions. 
  
**Game Goals**  
* Transport more goods to the location with limited stamina.  
* Choose the right path to avoid obstacles, and choose the right time to take and drop the goods.  
* Survive for as many levels as you can.
* Earn a better score by improving your game performance.
  
**Game Features**  
✔ Dynamic Difficulty – Adjusts map size, obstacles, and parcels.  
✔ Real-Time Timer – Syncs with actual playtime.  
✔ Pause/Resume – Save progress mid-game.  
✔ High Freedom Routing – Plan your own path strategically.  
✔ Score Tracking – Compete for the highest historical score.  

---

## How to run the game?

The game will be built and tested on the Linux (Ubuntu) platform, so playing this game on a Linux machine is recommended. Steps are:

1. Cloning this repository to your machine:

`git clone git@github.com:NaughtyChas/ENGG1340-GP.git`

2. Enter this project's  directory:

`cd ENGG1340-GP`

3. Build the game:

`make`

If you have built the game before, and something happens so you want to rebuild the game, run the following command to clean the build executable:

`make clean`

4. Run the game. The game executable can be found in the `bin` directory under the root directory. Run the game using the following command:

`./bin/main`

If you are running on the Windows platform, please head to the [GitHub Actions](https://github.com/NaughtyChas/ENGG1340-GP/actions/workflows/buildExe.yml) page to download the Windows executable.
You can also build your own, but it is somehow complicated so I recommend downloading this from the Actions instead.

> [!WARNING]
> We don't guarantee our game can run smoothly without flaws on the Windows platform, since the package `ncurses` is for Linux.
> However, we did add Windows support during coding and development. Suggest using Linux for the best experience.

---

## Game Rules & Mechanics  
**Detailed Rule Expalnation**  
  - Players gain a certain amount of stamina at the beginning of each game, and recover a certain amount of stamina every time they pass a level.  
  - The player's goal is to deliver all parcels to the appropriate receiving location in each level before running out of stamina, then leave through the exit and retain as much stamina as possible until the next level!  
  - When a player runs out of stamina, the game is over and the player is awarded the final score.  
  - Players can take more than one parcel, there will be more stamina consumption; any number of parcels can be put down at any time.  
  - There are grids on the map with different effects that cause extra stamina gains and losses (supply stations and speed bumps).  
  - Players can choose the difficulty (easy, medium, hard) at the beginning of each game, and the difficulty of the level will not change after the choice; the difficulty is related to the size of the map, the number of obstacles (5, 6, 7), and the number of packages.
  
**Stamina System**
  - Initial overall stamina is different across different diffs, which is:

    |Easy|Medium|Hard|
    |:-:|:-:|:-:|
    |200|270|350|

  - Stamina will be replenished after completing each level, which:

    |Easy|Medium|Hard|
    |:-:|:-:|:-:|
    |+75|+100|+150|
       
  - Stamina consumption:  
      + No parcels in hand： cost 1 per step.  
      + With n parcels📦: cost `1+n` per step.  
      + Speed bumps🚧: Double stamina cost in the next move.  
  - Supply stations🏪: reward `60-100` random stamina.
  
**Difficulty & Map Generation**   
Player will start at the center of the left boundary and end each round at the same y-location on the opposite boundary.

| Difficulty | Map Size | No. of Packages | No. of Obstacles | 
|------------|----------|-----------------|------------------|
| Easy | 15x15 | 3 | 4 | 
| Normal | 20x20 | 4 | 5 | 
| Hard | 25x25 | 5 | 6 |
  
**Final Score Calculation**  
The score you get will be affected by the number of stamina points used and the time taken to clear each level.
Every time you pass a level, the score will accumulate until game over. Then your final score will be calculated.

### Tips for Playing the Game
  - **Route Planning**: Before starting to move, take a moment to scan the entire map. Identify the locations of supply stations, speed bumps, obstacles, packages, and the exit. Try to plan a route that minimizes the number of steps, especially when passing through speed bumps, as they double your stamina consumption per step.
  - **Stamina Management**: Don't rush to pick up all the packages at once. Calculate how much stamina you'll need to reach the delivery locations and the exit. It might be more beneficial to pick up packages in batches, especially if there are supply stations on the way.
  - **Use Supply Stations Wisely**: When you're near a supply station, make sure to stop and recharge your stamina. If you're low on stamina and have a long way to go, it's worth the detour to visit a supply station to avoid running out of stamina prematurely.
  - **Difficulty Selection**: If you're new to the game, start with the "easy" mode to get familiar with the mechanics and the map layout. As you gain more experience, you can gradually increase the difficulty for a more challenging experience.
  - **Experiment with Different Strategies**: Don't be afraid to try different approaches, such as picking up packages in a different order or taking alternative routes. Sometimes, a less obvious path might save you more stamina in the long run.

## Coding Implementations

*Will be merged from another PR.


