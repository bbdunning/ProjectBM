NOTE:
    setGravity(btVector3(0,-15,0)); // when in air
    setGravity(btVector3(0,-10,0)); // when grounded


## 75% Demo ##
- Character controller //done
- interactive stage 
- More Abilities
- Camera interpolation
- Shadow Mapping

## Queue ##
- particles
- deferred shading for lighting on projectiles would be cool
- later on particles
- sound
- bump mapping
- Cell Shading / Outlines
- UI
- Sound Engine (Background music & Sound Effects)

## Graphics Technologies Implemented ##
- Articulated Mesh Animation (wow this took a long time)

## Game Features Implemented ##
- Environmental Collision
- Platforms
- player action states 25%
- action hitboxes - 25% 

## Planned Graphics Technologies ##
- Cell Shading / Outlines
- UI
- Sound Engine (Background music & Sound Effects)
- Bump Mapping / Normal Mapping
- Particles Effects (Billboards)
- Hitbox Visualization
- Input Buffer?

## Planned Game Features ##
* gamepad support
* 2nd player
* action animations
* ledge detection
* ledge grab
* camera tracking characters
* camera shake effect
* airdodge


### Additional Notes ###
* might need to normalize bone weights per vertex

=====

## CONTROLS ##

### View: ###
    * The view is controlled by movement of the mouse.
    * Mouse cursor icon is disabled via glfw command

### Movement: ###
    - WASD controls camera movemend
    - arrow keys control player
    - shift -> move player up
    - ctrl -> move player down

## Other ##
* fully crossplatform
    - migrated from linux laptop to windows 10 pc