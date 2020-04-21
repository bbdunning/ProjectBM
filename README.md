Lab 1

https://github.com/assimp/assimp




* change shader to use spec image tex
* move light

Ignore this
=====

## Final Project Features ##
    * Switched to Assimp
    * can import any 3d model type
    * textures are automatically loaded in  from model files
    * interactive playable character

    Disclaimer - you need to have the assimp library installed on your machine to run this code. As it is a large library, I did not include the source in my project.

## CONTROLS ##

### View: ###
    * The view is controlled by movement of the mouse.
    * Mouse cursor icon is disabled via glfw command (hopfully does not create any cross compatibility errors)

### Movement: ###
    * W and S dolley the camera forward and backward
    * A and D strafe the camera
    * Left Shift raises the camera
    * LeftCtrl lowers the camera

### Misc: ###
    * Q and E move the light left and right along the X axis.

## New files ##

### GameObject ###
    * includes a list of meshes, materials, and textures for a specfic object

### Input Handler ###
    * An object for creating key press flags and storing user input


## progress made in 476 ##
* fully crossplatform
    - migrated from linux laptop to windows 10 pc

* implemented player class
    - much better movement and physics now
    - double jump

## TO DO ##
* player action states 25%
* action hitboxes - 25% 
* gamepad support
* 2nd player
* animations
* ledge detection
* ledge grab
* camera tracking characters
* camera shake effect
* airdodge
* PARTICLES

## advanced stuff ##
* moonwalking
* wavedashing