#pragma once

#include <raylib.h>
#include "raymath.h"
#include "VBeams.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"     // Required for UI controls

namespace {
    Vector3 GetCameraForward(Camera* camera)
    {
        return Vector3Normalize(Vector3Subtract(camera->target, camera->position));
    }

    // Returns the cameras up vector (normalized)
    // Note: The up vector might not be perpendicular to the forward vector
    Vector3 GetCameraUp(Camera* camera)
    {
        return Vector3Normalize(camera->up);
    }

    // Returns the cameras right vector (normalized)
    Vector3 GetCameraRight(Camera* camera)
    {
        Vector3 forward = GetCameraForward(camera);
        Vector3 up = GetCameraUp(camera);

        return Vector3Normalize(Vector3CrossProduct(forward, up));
    }

    // Moves the camera in its forward direction
    void CameraMoveForward(Camera* camera, float distance, bool moveInWorldPlane)
    {
        Vector3 forward = GetCameraForward(camera);

        if (moveInWorldPlane)
        {
            // Project vector onto world plane
            forward.y = 0;
            forward = Vector3Normalize(forward);
        }

        // Scale by distance
        forward = Vector3Scale(forward, distance);

        // Move position and target
        camera->position = Vector3Add(camera->position, forward);
        camera->target = Vector3Add(camera->target, forward);
    }

    // Moves the camera in its up direction
    void CameraMoveUp(Camera* camera, float distance)
    {
        Vector3 up = GetCameraUp(camera);

        // Scale by distance
        up = Vector3Scale(up, distance);

        // Move position and target
        camera->position = Vector3Add(camera->position, up);
        camera->target = Vector3Add(camera->target, up);
    }

    // Moves the camera target in its current right direction
    void CameraMoveRight(Camera* camera, float distance, bool moveInWorldPlane)
    {
        Vector3 right = GetCameraRight(camera);

        if (moveInWorldPlane)
        {
            // Project vector onto world plane
            right.y = 0;
            right = Vector3Normalize(right);
        }

        // Scale by distance
        right = Vector3Scale(right, distance);

        // Move position and target
        camera->position = Vector3Add(camera->position, right);
        camera->target = Vector3Add(camera->target, right);
    }

    // Moves the camera position closer/farther to/from the camera target
    void CameraMoveToTarget(Camera* camera, float delta)
    {
        float distance = Vector3Distance(camera->position, camera->target);

        // Apply delta
        distance += delta;

        // Distance must be greater than 0
        if (distance <= 0) distance = 0.001f;

        // Set new distance by moving the position along the forward vector
        Vector3 forward = GetCameraForward(camera);
        camera->position = Vector3Add(camera->target, Vector3Scale(forward, -distance));
    }

    // Rotates the camera around its up vector
    // Yaw is "looking left and right"
    // If rotateAroundTarget is false, the camera rotates around its position
    // Note: angle must be provided in radians
    void CameraYaw(Camera* camera, float angle, bool rotateAroundTarget)
    {
        // Rotation axis
        Vector3 up = GetCameraUp(camera);

        // View vector
        Vector3 targetPosition = Vector3Subtract(camera->target, camera->position);

        // Rotate view vector around up axis
        targetPosition = Vector3RotateByAxisAngle(targetPosition, up, angle);

        if (rotateAroundTarget)
        {
            // Move position relative to target
            camera->position = Vector3Subtract(camera->target, targetPosition);
        }
        else // rotate around camera.position
        {
            // Move target relative to position
            camera->target = Vector3Add(camera->position, targetPosition);
        }
    }


    // Rotates the camera around its right vector, pitch is "looking up and down"
    //  - lockView prevents camera overrotation (aka "somersaults")
    //  - rotateAroundTarget defines if rotation is around target or around its position
    //  - rotateUp rotates the up direction as well (typically only usefull in CAMERA_FREE)
    // NOTE: angle must be provided in radians
    void CameraPitch(Camera* camera, float angle, bool lockView, bool rotateAroundTarget, bool rotateUp)
    {
        // Up direction
        Vector3 up = GetCameraUp(camera);

        // View vector
        Vector3 targetPosition = Vector3Subtract(camera->target, camera->position);

        if (lockView)
        {
            // In these camera modes we clamp the Pitch angle
            // to allow only viewing straight up or down.

            // Clamp view up
            float maxAngleUp = Vector3Angle(up, targetPosition);
            maxAngleUp -= 0.001f; // avoid numerical errors
            if (angle > maxAngleUp) angle = maxAngleUp;

            // Clamp view down
            float maxAngleDown = Vector3Angle(Vector3Negate(up), targetPosition);
            maxAngleDown *= -1.0f; // downwards angle is negative
            maxAngleDown += 0.001f; // avoid numerical errors
            if (angle < maxAngleDown) angle = maxAngleDown;
        }

        // Rotation axis
        Vector3 right = GetCameraRight(camera);

        // Rotate view vector around right axis
        targetPosition = Vector3RotateByAxisAngle(targetPosition, right, angle);

        if (rotateAroundTarget)
        {
            // Move position relative to target
            camera->position = Vector3Subtract(camera->target, targetPosition);
        }
        else // rotate around camera.position
        {
            // Move target relative to position
            camera->target = Vector3Add(camera->position, targetPosition);
        }

        if (rotateUp)
        {
            // Rotate up direction around right axis
            camera->up = Vector3RotateByAxisAngle(camera->up, right, angle);
        }
    }

    // Rotates the camera around its forward vector
    // Roll is "turning your head sideways to the left or right"
    // Note: angle must be provided in radians
    void CameraRoll(Camera* camera, float angle)
    {
        // Rotation axis
        Vector3 forward = GetCameraForward(camera);

        // Rotate up direction around forward axis
        camera->up = Vector3RotateByAxisAngle(camera->up, forward, angle);
    }

    // Returns the camera view matrix
    Matrix GetCameraViewMatrix(Camera* camera)
    {
        return MatrixLookAt(camera->position, camera->target, camera->up);
    }

    void CameraRotateAround(Camera* camera, Vector3 target, float yawAng, float pitchAng) {
        // Rotation axis
        Vector3 up = GetCameraUp(camera);

        // Rotation axis
        Vector3 right = GetCameraRight(camera);

        // View vector
        Vector3 targetPosition = Vector3Subtract(camera->target, camera->position);

        // Move position relative to target
        camera->position = Vector3Subtract(target, Vector3RotateByAxisAngle(targetPosition, right, -pitchAng));
        //camera->up = Vector3RotateByAxisAngle(camera->up, right, -pitchAng);


        targetPosition = Vector3Subtract(camera->target, camera->position);
        camera->position = Vector3Subtract(target, Vector3RotateByAxisAngle(targetPosition, up, -yawAng));

    }
    
    void UpdateCameraProAroundTarget(Camera* camera, Vector3 movement, Vector3 rotation, float zoom)
    {
        // Required values
        // movement.x - Move forward/backward
        // movement.y - Move right/left
        // movement.z - Move up/down
        // rotation.x - yaw
        // rotation.y - pitch
        // rotation.z - roll
        // zoom - Move towards target

        bool lockView = true;
        bool rotateAroundTarget = true;
        bool rotateUp = false;
        bool moveInWorldPlane = true;

        // Camera rotation
        CameraPitch(camera, -rotation.y * DEG2RAD, lockView, rotateAroundTarget, rotateUp);
        CameraYaw(camera, -rotation.x * DEG2RAD, rotateAroundTarget);
        CameraRoll(camera, rotation.z * DEG2RAD);

        // Camera movement
        CameraMoveForward(camera, movement.x, moveInWorldPlane);
        CameraMoveRight(camera, movement.y, moveInWorldPlane);
        CameraMoveUp(camera, movement.z);

        // Zoom target distance
        CameraMoveToTarget(camera, zoom);
    }
}




//class NodeMap2vector {
//    std::map <Vector3, size_t> nodeMap;
//    std::vector<size_t> removedIds;
//    
//
//    size_t getId(Vector3 input) {
//        size_t foundId = nodeMap.find(input);
//    }
//};


void CameraControls(Vector3& rotCenter, Camera3D& camera, const BoundingBox& objectsBoundBox);

void SetupScene(Camera3D& camera);

void RenderNodes(bool deformed,Beams::Model model, std::vector<size_t>& selected);

void PickNodes_ForElement(const Camera3D& camera, size_t& pickingNodesForElems, const std::vector<Beams::Node>& nodes, std::vector<size_t>& selectedNodes, Beams::Model& model);

void runViewer(Beams::Model& model) {
	Camera3D camera;
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    Vector3 rotCenter{ 0.0f,0.0f,0.0f };
    BoundingBox objectsBoundBox{ Vector3{-1.0f,-1.0f,-1.0f},{1.0f,1.0f,1.0f} };

    SetupScene(camera);
    

	while (!WindowShouldClose())		// run the loop untill the user presses ESCAPE or presses the Close button on the window
	{
        static std::vector<size_t> selectedNodes;
        static size_t pickingNodesForElems = 0;
        auto& elements = model.getElements();
        static bool deformed = false;
        CameraControls(rotCenter, camera, objectsBoundBox);
        

        auto& nodes = model.getNodes();
        if (IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE)) {
            Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);

            for (auto& node : nodes) {
                Vector3 nodeCenter{ node.x / 10,node.y / 10,node.z / 10 };
                RayCollision collision = GetRayCollisionSphere(ray, nodeCenter, 0.15f);
                if (collision.hit) {
                    camera.target = nodeCenter;
                }
            }
        }

        if (IsKeyReleased(KEY_SPACE)) {
            deformed = !deformed;
        }

        PickNodes_ForElement(camera, pickingNodesForElems, nodes, selectedNodes, model);




		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(RAYWHITE);
			
			BeginMode3D(camera);
				
            RenderNodes(deformed,model, selectedNodes);

            for (auto& element : elements) {
                Vector3 n1{ element.node1->x / 10,element.node1->y / 10,element.node1->z / 10 };
                Vector3 n2{ element.node2->x / 10,element.node2->y / 10,element.node2->z / 10 };
                if (deformed) {
                    n1 += model.getDeflection(element.node1->id);
                    n2 += model.getDeflection(element.node2->id);
                }
                DrawCapsule(n1,n2,0.1f,2,2,BLUE);
                
            }
			
            EndMode3D();

		// draw some text using the default font
		DrawText("Hello Raylib", 200, 200, 20, BLACK);

        if (pickingNodesForElems) {
            DrawText("Pick Nodes For Elements", 100, 200, 20, RED);
        }

        // Check all possible UI states that require controls lock
        //if (splineTypeEditMode || (selectedPoint != -1) || (selectedControlPoint != NULL)) GuiLock();
        static int nodeSectionEl = 0;
        static bool dropdownEdit = false;
        static bool NodeAddActive = false;

        if (dropdownEdit) GuiLock();

        if (NodeAddActive)
        {
            static int xNew;
            static int yNew;
            static int zNew;
            static bool xActive = false, yActive = false, zActive = false;
            static bool xNeg = false;
            static bool yNeg = false;
            static bool zNeg = false;
            static bool spin = false;

            NodeAddActive = !GuiWindowBox(Rectangle{ 440, 24, 288, 136 }, "AddNode");
            if (GuiValueBox(Rectangle{ 464, 56, 56, 24 }, "X", &xNew, 0, 100, xActive)) xActive = !xActive;
            if (GuiValueBox(Rectangle{ 464, 88, 56, 24 }, "Y", &yNew, 0, 100, yActive)) yActive = !yActive;
            if (GuiValueBox(Rectangle{ 464, 120, 56, 24 }, "Z", &zNew, 0, 100, zActive)) zActive = !zActive;
                GuiCheckBox(Rectangle{ 536, 56, 24, 24 }, "Neg", &xNeg);
            GuiCheckBox(Rectangle{ 536, 88, 24, 24 }, "Neg", &yNeg);
            GuiCheckBox(Rectangle{ 536, 120, 24, 24 }, "Neg", &zNeg);
        
            if (GuiButton(Rectangle{ 592, 88, 120, 24 }, "ADD NODE!")){
                NodeAddActive = false;
                xNew = (xNeg) ? -xNew : xNew;
                yNew = (yNeg) ? -yNew : yNew;
                zNew = (zNeg) ? -zNew : zNew;
                model.addNode(Vector3{ (float)xNew,(float)yNew,(float)zNew });
            }
        }
        //GuiToggleGroup((Rectangle) { 160, 64, 40, 24 }, "ONE;TWO;THREE", & ToggleGroup002Active);
        //if (GuiDropdownBox((Rectangle) { 24, 64, 120, 24 }, "ONE;TWO;THREE", & DropdownBox000Active, DropdownBox000EditMode)) DropdownBox000EditMode = !DropdownBox000EditMode;

        // Draw spline config
        //GuiLabel(Rectangle{ 12, 62, 140, 24 }, TextFormat("Spline thickness: %i", (int)splineThickness));
        //GuiSliderBar(Rectangle{ 12, 60 + 24, 140, 16 }, NULL, NULL, & splineThickness, 1.0f, 40.0f);

        //GuiCheckBox((Rectangle) { 12, 110, 20, 20 }, "Show point helpers", & splineHelpersActive);

        //if (splineTypeEditMode) GuiUnlock();
        GuiLabel(Rectangle{12, 10, 140, 24 }, "Spline type:");
        if (GuiDropdownBox(Rectangle{ 12, 8 + 24, 140, 28 }, "NODES;SECTIONS;ELEMENTS", &nodeSectionEl, dropdownEdit)) dropdownEdit = !dropdownEdit;
        if (GuiButton(Rectangle{ 12, 70, 140, 28 }, "Solve!")) {
            model.solve();
        }
        if (nodeSectionEl == 0) {
            if (GuiButton(Rectangle{ 12 + 140 + 12, 8 + 24, 50, 28 }, "ADD")) {
                NodeAddActive = true;
            }
        }



        GuiUnlock();

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();

       





	}

	// cleanup

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return;
}

void PickNodes_ForElement(const Camera3D& camera, size_t& pickingNodesForElems, const std::vector<Beams::Node>& nodes, std::vector<size_t>& selectedNodes, Beams::Model& model)
{
    if (IsKeyDown(KEY_LEFT_SHIFT) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);
        RayCollision collision;
        /*for (auto& node : nodes) {
        for (size_t i = 0; i < nodes.size(); i++) {
        Vector3 nodeCenter{ nodes[i].x / 10 ,nodes[i].y / 10 ,nodes[i].z / 10 };
        collision = GetRayCollisionSphere(ray, nodeCenter, 0.15f);
        if (collision.hit) {

        selectedNodes.push_back(i);
        }
        }
        }*/

        if (pickingNodesForElems == 0) {
            for (size_t i = 0; i < nodes.size(); i++) {
                Vector3 nodeCenter{ nodes[i].x / 10,nodes[i].y / 10,nodes[i].z / 10 };
                collision = GetRayCollisionSphere(ray, nodeCenter, 0.15f);
                if (collision.hit) {
                    pickingNodesForElems = 1;
                    selectedNodes.push_back(i);
                    std::cout << "Picked 1";
                    break;
                }
            }
        }
        else if (pickingNodesForElems == 1) {
            for (size_t i = 0; i < nodes.size(); i++) {
                Vector3 nodeCenter{ nodes[i].x / 10,nodes[i].y / 10,nodes[i].z / 10 };
                collision = GetRayCollisionSphere(ray, nodeCenter, 0.15f);
                if (collision.hit) {
                    pickingNodesForElems = 2;
                    selectedNodes.push_back(i);
                    std::cout << "Picked 2";
                    break;

                }
            }
        }
        else if (pickingNodesForElems == 2) {
            for (size_t i = 0; i < nodes.size(); i++) {
                Vector3 nodeCenter{ nodes[i].x / 10,nodes[i].y / 10,nodes[i].z / 10 };
                collision = GetRayCollisionSphere(ray, nodeCenter, 0.15f);
                if (collision.hit) {
                    pickingNodesForElems = 0;
                    model.addElement(selectedNodes[0], selectedNodes[1], i,0);//<---------------------- FIX THIS! FIX!!!!
                    selectedNodes.clear();
                    std::cout << "Picked 3";
                    break;

                }
            }

        }
    }
    else if (IsKeyUp(KEY_LEFT_SHIFT)) {
        pickingNodesForElems = 0;
        selectedNodes.clear();

    }
}

void RenderNodes(bool deformed,Beams::Model model,std::vector<size_t>& selected)
{
    const std::vector<Beams::Node>& nodes = model.getNodes();


    for (auto& node : nodes) {
        Vector3 nodeCoords{ node.x / 10,node.y / 10,node.z / 10 };
        
        if (node.free_flag) { 
            DrawSphere(nodeCoords, 0.15f, RED);
            continue; 
        }

        if (deformed) {
            nodeCoords += model.getDeflection(node.id);
        }
        DrawSphere(nodeCoords, 0.15f, RED);

        Vector3 force = model.getForce(node.id); //moronic func, remaining from before it was like this.
        force = Vector3Normalize(force);
        Vector3 forceEndpoint = Vector3Add(nodeCoords, force);
        if (force.x  + force.y + force.z > 0.001) {
            DrawLine3D(nodeCoords, forceEndpoint  , DARKBROWN);
            DrawCylinderEx(Vector3Add(nodeCoords, force / 2), forceEndpoint, 0.1f, 0.0f, 10, DARKBROWN);
            //DrawLine3D(nodeCoords, Vector3Subtract(nodeCoords, force), YELLOW);

        }
        
        
    }


    for (size_t pos : selected) {
        DrawSphereWires(Vector3{ nodes[pos].x / 10,nodes[pos].y / 10,nodes[pos].z / 10 }, 0.2, 5, 5, DARKGREEN);
    }

    //DrawCube(Vector3Zero(), 2.0f, 2.0f, 2.0f, RED);
    DrawCubeWires(Vector3Zero(), 2.0f, 2.0f, 2.0f, MAROON);
    DrawGrid(10, 10.0f);
}

void SetupScene(Camera3D& camera)
{
    const int screenWidth = 1280;
    const int screenHeight = 800;

    // Camera position
    camera.position.x = -100.0f;
    camera.position.y = 120.0f;
    camera.position.z = -100.0f;

    //camera target
    camera.target.x = 0.0f;
    camera.target.y = 0.0f;
    camera.target.z = 0.0f;

    camera.up.x = 0.0f;
    camera.up.y = 1.0f;
    camera.up.z = 0.0f;// Camera up vector (rotation towards target)

    camera.fovy = 5.0f;                                    // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;




    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    // Tell the window to use vsync and work on high DPI displays
    //SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);


    // Create the window and OpenGL context
    InitWindow(screenWidth, screenHeight, "VBeams");
}

void CameraControls(Vector3& rotCenter, Camera3D& camera, const BoundingBox& objectsBoundBox)
{

    if (IsKeyPressed(KEY_Z)) {
        rotCenter = Vector3Zero();
        camera.target = rotCenter;
    }
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {

        Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);

        // Check collision between ray and box
        
        RayCollision collision = GetRayCollisionBox(ray, objectsBoundBox);
        if (collision.hit)
        {
            // Generate new random colors
            rotCenter = collision.point;
            camera.target = rotCenter;
        }

    }
    else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {

        //UpdateCamera(&camera, CAMERA_THIRD_PERSON);
        //camera.target = Vector3{ 0.0f,0.0f, 0.0f };
        CameraRotateAround(&camera, camera.target, GetMouseDelta().x * 0.003f, GetMouseDelta().y * 0.005f);
        //         (&camera,Vector3{0,0,0},
        //	Vector3{
        //	GetMouseDelta().x * 0.3f,                            // Rotation: yaw
        //	GetMouseDelta().y * 0.3f,                            // Rotation: pitch
        //	0.0f                                                // Rotation: roll
        //},
        //GetMouseWheelMove() * 2.0f);                              // Move to target (zoom)
        SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);



    }
    else {
        UpdateCameraProAroundTarget(&camera, Vector3{ 0,0,0 }, Vector3{ 0,0,0 }, GetMouseWheelMove() * 10.0f);
        SetMouseCursor(MOUSE_CURSOR_ARROW);

    }
}
