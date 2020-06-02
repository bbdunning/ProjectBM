## 90% Demo & Beyond ##
- Particle System
- UI
- charge shot
- Outline Shader
- More Abilities
- Camera interpolation
- force field effect on screen

## 100% Demo ##
- controller support
- split screen camera

## 75% Demo ##
- Character controller
    - animations match character actions
    - raycast to check if grounded
- Shadow Mapping
    - soft shadows
- Audio
    - irrKlang
    - background music
    - sound effects
    - ball hitting ground
- Shading / Visual style
    - new Fragment Shader
        - Cell shading
        - Uses binning
    - redid textures for the map

## Queue ##
- deferred shading for lighting on projectiles would be cool
- bump mapping
- Cell Shading / Outlines
- Sound Engine (Background music & Sound Effects)

## Graphics Technologies Implemented ##
- Articulated Mesh Animation (wow this took a long time)
- smooth shadow mapping

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