# Gravity simulation

## Usage

Gravity simulation is a user-friendly program which allows the user to experiment with the force of gravity and get a visual demonstration of it. In most cases (if time per update is low enough) the simulation is very accurate. It runs Newton's law of universal gravitation many times per second, so it is able to quickly simulate complex systems with up to 1000 objects. The objects would act almost like they would in real life.

## Files

The [executable file][file0] requires the [Resources][folder0] folder and files [msvcp140.dll][file1], [msvcp140_1.dll][file2], [vcruntime140.dll][file3] and [vcruntime140_1.dll][file4] to run (all of them are located [here][folder1]).
All the source files can be found [here][folder2].

## Controls

- **Esc** - switch menu
- **Dragging while holding left click** - move around the map
- **Left click on an object** - select the object
- **Zoom in/out (mouse scroll)** - change scale
- **Right click** - delete all the objects near the cursor
- **P** - pause
- **<** -  slow down the time speed
- **>** - speed up the time speed
- **T** / **/** - access the command line
- **F11** - switch to fullscreen mode
- All the other controls are available in the menu

#### Creating an object

First, select center of the new object with middle mouse button / left alt + left click. Then move the mouse to select the object radius and click the same button. After that, move the mouse to set the speed vector of the new object and click the same button again.
If another object is selected, then after new object creation its location and velocity will be set relative to the selected object.

## Commands

- **/save <filename>** - save the current layout with the name "filename", the file "filename.planets" will appear in the same folder
- **/load <filename>** - load the saved layout with the name "filename", it is possible only if the file "filename.planets" is present in the same folder
- **/clear** - clear the layout, reset scale and time speed
- **/random** - clear the layout, reset scale and time speed and put 1000 random objects

## Implementation

The program uses SFML for graphics.
To run calculations most effectively, it runs them in a dedicated thread, while the other one is responsible for graphics and controls.
## Examples

Solar system simulation:
![a](https://user-images.githubusercontent.com/35459417/143690720-e1572a10-acbf-46e9-90a1-c436c35ea92e.png)

The same simulation, but relatively to Earth:
![b](https://user-images.githubusercontent.com/35459417/143690725-0ba517d5-7de5-4cbe-96d6-0ca3e474dbc3.png)

Simulation result of 1000 random object:
![c](https://user-images.githubusercontent.com/35459417/143690722-e66a3af0-569c-479d-be4b-b67886ea5774.png)

[Video example](https://drive.google.com/file/d/1F2qMXRXUaOSm25iculFD9ME9PUAu6IEZ/view?usp=sharing)


   [folder0]: <https://github.com/Petr1Furious/gravity-simulation/blob/master/x64/Release/Resources>
   [folder1]: <https://github.com/Petr1Furious/gravity-simulation/blob/master/x64/Release>
   [folder2]: <https://github.com/Petr1Furious/gravity-simulation/tree/master/Planets>
   [file0]: <https://github.com/Petr1Furious/gravity-simulation/blob/master/x64/Release/Planets.exe>
   [file1]: <https://github.com/Petr1Furious/gravity-simulation/blob/master/x64/Release/msvcp140.dll>
   [file2]: <https://github.com/Petr1Furious/gravity-simulation/blob/master/x64/Release/msvcp140_1.dll>
   [file3]: <https://github.com/Petr1Furious/gravity-simulation/blob/master/x64/Release/vcruntime140.dll>
   [file4]: <https://github.com/Petr1Furious/gravity-simulation/blob/master/x64/Release/vcruntime140_1.dll>
