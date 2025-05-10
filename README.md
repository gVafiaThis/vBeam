# vBeam
A 3d FEA program based on the 12 DoF Beam Element (for now...*)
## Installation
Click "Releases" on the right of the GitHub Page and download the latest release. Then just run the .exe
_Curently only supporting windows_
## Features
This is a standalone program, that allows for the creation and analysis of beam structures.
![Image](https://github.com/user-attachments/assets/7eab8e20-fdbe-4bfa-a157-555b67b4245f)
All the required libraries are included and are:
- **Raylib/Raygui**: For the generation of the graphics/UI & general visualization
- **Eigen**: For matrix operations and solving equations.

## Usage 
### Make the Structure
1.**Create the Nodes of the beams**

*   From Nodes->ADD, add the required nodes using either global coordinates, or positioning relative to selected nodes. 

1.**Create beam sections**

*   From Sections->Manage, input the section properties and modify existing sections with SAVE or add new ones with ADD. (__NOTE:__ The program is unitless so be consistent with your units)

1.**Create the beams**
*   From Elements->ADD, 
   add elements for the sections  you specified in the list. (__NOTE:__ every element needs __3__ nodes. The first 2 nodes picked are the ends of the element and the last defines the element's rotation)

*   From Elements->Copy, you can copy and offset existing structures. (__NOTE:__ When copying, new nodes and elements are created. To remove the duplicates, use Nodes->REMOVE->Show Duplicates->Remove Duplicates. __Then__ remove duplicate elements from Elements->Remove->etc.) 

### Solve!
1.**Add Forces**

*   From Forces->ADD, pick nodes and add forces

1.**Add boundary conditions**

*   From BCs->ADD, pick nodes and add fixed nodes (__NOTE:__ In this version*, only the fixed boundary condition is available)

1.**Solve**

*   Click solve on the bottom left 

### Results/Info
1.**View the deformed structure.**

*   Press the Spacebar to toggle deformed state

1.**View Node Info**

*   From Nodes->Info, pick nodes and view results (__NOTE:__ In this version*, only node deflections are available)

1.**View Element Info**

*   From Elements->Info, pick elements and view their local axes orientation (Green is local Y axis, Red is local Z axis) 

1.**Save/Load**

*   Use the buttons on the right  



*For version with more results (e.g. stress,strain,internal beam forces,resultant forces), and features (e.g. other BCs) contact the author.  
