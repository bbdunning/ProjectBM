## 50% Demo ##
Gameplay elements:
    - Player actions: will require hitboxes and animations
        - forward arial
        - backward arial
        - neutral attack
        - forward smash
    - animations:
        - standing: done
        - walking: done
        - run

Graphics technologies
    - Blender .dae export issue
    - Cell Shading (?):
        ask for resources
    - Improve physics engine
        - collision detection okay, focus on collision resolution (primarily with environment)
        - ask for resources
    - Sound Engine (Background music & Sound Effects)
    - Bump Mapping / Normal Mapping

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
* wavedashing


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