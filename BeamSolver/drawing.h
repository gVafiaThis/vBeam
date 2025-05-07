#pragma once

#include <raylib.h>
#include "raymath.h"
#include "VBeams.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"     // Required for UI controls

#undef RAYGUI_IMPLEMENTATION            // Avoid including raygui implementation again

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
extern "C" {
    #include "MODIFIED_gui_window_file_dialog.h"

}

#include "saveFile.h"

namespace {//modified or exposed raylib funcs
    //------------------------------------------------------------------------------------------------------------------------
    //HELPER FUNCTIONS FOR CAMERA - Modified from raylib core
    //------------------------------------------------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------------------------------------------------
    //Gui Elements - Modified from raygui
    //------------------------------------------------------------------------------------------------------------------------

    //modified GuiValueBoxFloat 
    int v_GuiValueBoxFloat(Rectangle bounds, const char* text, char* textValue, float* value, bool editMode)
    {
#if !defined(RAYGUI_VALUEBOX_MAX_CHARS)
#define RAYGUI_VALUEBOX_MAX_CHARS  32
#endif

        int result = 0;
        GuiState state = guiState;

        /*char textValue[RAYGUI_VALUEBOX_MAX_CHARS + 1] = "\0";
        snprintf(textValue, sizeof(textValue), "%2.2f", *value);*/
        //static char textValue[32] = "";

        Rectangle textBounds = { 0 };
        //Text will never be NULL here
        //if (text != NULL)
        //{
        textBounds.width = (float)GetTextWidth(text) + 2;
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + bounds.width + GuiGetStyle(VALUEBOX, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height / 2 - GuiGetStyle(DEFAULT, TEXT_SIZE) / 2;
        if (GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT) == TEXT_ALIGN_LEFT) textBounds.x = bounds.x - textBounds.width - GuiGetStyle(VALUEBOX, TEXT_PADDING);
        //}

        // Update control
        //--------------------------------------------------------------------
        if ((state != STATE_DISABLED) && !guiLocked && !guiControlExclusiveMode)
        {
            Vector2 mousePoint = GetMousePosition();

            bool valueHasChanged = false;

            if (editMode)
            {
                state = STATE_PRESSED;

                int keyCount = (int)strlen(textValue);

                // Only allow keys in range [48..57]
                if (keyCount < RAYGUI_VALUEBOX_MAX_CHARS)
                {
                    if (GetTextWidth(textValue) < bounds.width)
                    {
                        int key = GetCharPressed();
                        if (((key >= 48) && (key <= 57)) ||
                            (key == '.') ||
                            ((keyCount == 0) && (key == '+')) ||  // NOTE: Sign can only be in first position
                            ((keyCount == 0) && (key == '-')))
                        {
                            textValue[keyCount] = (char)key;
                            keyCount++;

                            valueHasChanged = true;
                        }
                    }
                }

                // Pressed backspace
                if (IsKeyPressed(KEY_BACKSPACE))
                {
                    if (IsKeyDown(KEY_LEFT_CONTROL)) {
                        while (keyCount != 0) {
                            keyCount--;
                            textValue[keyCount] = '\0';
                        }
                        valueHasChanged = true;
                    }
                    else if (keyCount > 0)
                    {
                        keyCount--;
                        textValue[keyCount] = '\0';
                        valueHasChanged = true;
                    }
                }

                if (valueHasChanged) *value = TextToFloat(textValue);

                if ((IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) || IsKeyPressed(KEY_TAB) || (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) result = 1;
            }
            else
            {
                if (CheckCollisionPointRec(mousePoint, bounds))
                {
                    state = STATE_FOCUSED;
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) result = 1;
                }
            }
        }
        //--------------------------------------------------------------------

        // Draw control
        //--------------------------------------------------------------------
        Color baseColor = BLANK;
        if (state == STATE_PRESSED) baseColor = GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_PRESSED));
        else if (state == STATE_DISABLED) baseColor = GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_DISABLED));

        GuiDrawRectangle(bounds, GuiGetStyle(VALUEBOX, BORDER_WIDTH), GetColor(GuiGetStyle(VALUEBOX, BORDER + (state * 3))), baseColor);
        if (textValue[0] == '\0')  GuiDrawText("0", GetTextBounds(VALUEBOX, bounds), TEXT_ALIGN_CENTER, GetColor(GuiGetStyle(VALUEBOX, TEXT + (state * 3))));

        else GuiDrawText(textValue, GetTextBounds(VALUEBOX, bounds), TEXT_ALIGN_CENTER, GetColor(GuiGetStyle(VALUEBOX, TEXT + (state * 3))));
        // Draw cursor
        if (editMode)
        {
            // NOTE: ValueBox internal text is always centered
            Rectangle cursor = { bounds.x + GetTextWidth(textValue) / 2 + bounds.width / 2 + 1,
                                bounds.y + 2 * GuiGetStyle(VALUEBOX, BORDER_WIDTH), 4,
                                bounds.height - 4 * GuiGetStyle(VALUEBOX, BORDER_WIDTH) };
            GuiDrawRectangle(cursor, 0, BLANK, GetColor(GuiGetStyle(VALUEBOX, BORDER_COLOR_PRESSED)));
        }

        // Draw text label if provided
        GuiDrawText(text, textBounds,
            (GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT) == TEXT_ALIGN_RIGHT) ? TEXT_ALIGN_LEFT : TEXT_ALIGN_RIGHT,
            GetColor(GuiGetStyle(LABEL, TEXT + (state * 3))));
        //--------------------------------------------------------------------

        return result;
    }

}

namespace Rendering{

    enum GuiFlags {
        DROPDOWN_EDIT = 1 << 0,
        NODE_ADD_ACTIVE = 1 << 1,
        NODE_REMOVE_ACTIVE = 1 << 2,
        EL_ADD_ACTIVE = 1 << 3,
        EL_REMOVE_ACTIVE = 1 << 4,
        SHOW_NODE_INFO = 1 << 5,
        SHOW_ELEM_INFO = 1 << 6,
        SECTION_WINDOW = 1 << 7,
        FORCE_ADD_ACTIVE = 1 << 8,
        FORCE_REMOVE_ACTIVE = 1 << 9,
        SHOW_FORCE_INFO = 1 << 10,
        BC_ADD_ACTIVE = 1 << 11,
        BC_REMOVE_ACTIVE = 1 << 12,
        SHOW_BC_INFO = 1 << 13,
        EL_COPY_ACTIVE = 1<<14
    };
    namespace {
        class SelectionBox {
            Vector2 point1{ 0 , 0 }, point2{ 0,0 };
        public:
            bool active = false;
            void set_point1(Vector2&& point) {
                point1 = point;
            }
            void set_point2(Vector2&& point) {
                point2 = point;
            }
            Vector2 get_point1() {
                return point1;
            }
            Vector2 get_point2() {
                return point2;
            }
            Vector2 get_BottomLeft() {
                return Vector2{ (point1.x < point2.x) ? point1.x : point2.x, (point1.y < point2.y) ? point1.y : point2.y };
            };
            Vector2 get_TopRight() {
                return Vector2{ (point1.x > point2.x) ? point1.x : point2.x, (point1.y > point2.y) ? point1.y : point2.y };
            };
        };

        
        struct modelDrawState {
            bool deformed = false;
            std::vector<size_t> selectedNodes;
            std::vector<size_t> selectedElems;
            std::vector<size_t> infoNodes;
            std::vector<size_t> infoElems;
            int activeDropdownMenu = 0; //Which action section (for nodes, elements, sections etc) is currently active
            SelectionBox selectionBox;
        };
    }

    void getInput_CameraControl(Camera3D& camera, Beams::Model& model);
    void SetupScene(Camera3D& camera);
    void RenderNodes(Beams::Model& model, modelDrawState& modelState);
    void getInput_pickNode(const Camera3D& camera, Beams::Model& model, modelDrawState& modelState);
    void RenderElements(Beams::Model& model, modelDrawState& modelState);
    void drawGuiActionWindows(uint32_t& ActionFlags, Beams::Model& model, modelDrawState& modelState);
    void drawInfo(uint32_t& ActionFlags, const Camera& camera, Beams::Model& model, modelDrawState& modelState);
    void drawGuiActionMenu(uint32_t& ActionFlags, Beams::Model& model, modelDrawState& modelState);
    void getInput_pickElem(const Camera3D& camera,  Beams::Model& model, modelDrawState &modelState);


    //TODO: Deformed Nodes calculation once and updates when solving. 
    void runViewer(Beams::Model& model) {
        Camera3D camera;
        Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
        Vector3 rotCenter{ 0.0f,0.0f,0.0f };
        BoundingBox objectsBoundBox{ Vector3{-1.0f,-1.0f,-1.0f},{1.0f,1.0f,1.0f} };

        SetupScene(camera);

        // Custom file dialog
        GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
        char fileNameToLoad[512] = { 0 };
        sprintf(fileDialogState.filterExt, "DIR;.vbeam\0");

        //Model state - All info needed to do or view actions  
        modelDrawState modelState;
        uint32_t ActionFlags = 0; //Which action is currently happening

        while (!WindowShouldClose())// run the loop until the user presses ESCAPE or presses the Close button on the window
        {
            if (fileDialogState.SelectFilePressed)
            {
                std::cout << fileNameToLoad << "\n";
                // load file 
                if (!fileDialogState.saveFileMode) {
                    strcpy(fileNameToLoad, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
                    Saving::loadModel(model, fileNameToLoad);
                }
                else {
                    strcpy(fileNameToLoad, TextFormat("%s" PATH_SEPERATOR "%s.vbeam", fileDialogState.dirPathText, fileDialogState.fileNameText));
                    Saving::saveModel(model, fileNameToLoad);
                }
           


                fileDialogState.SelectFilePressed = false;
            }

            std::vector<size_t>& selectedNodes = modelState.selectedNodes;
            std::vector<size_t>& selectedElems = modelState.selectedElems;
            std::vector<size_t>& infoNodes = modelState.infoNodes;
            std::vector<size_t>& infoElems = modelState.infoElems;
            bool& deformed = modelState.deformed;
            int& activeDropdownMenu = modelState.activeDropdownMenu;
            

            static uint32_t ActionFlags = 0; //Which action are we curently doing


            getInput_CameraControl(camera, model);


            if (IsKeyReleased(KEY_SPACE)) {
                deformed = !deformed;
            }
            if (!model.isSolved()) deformed = false;


            bool nodePicking = ActionFlags &
                (GuiFlags::NODE_ADD_ACTIVE | GuiFlags::NODE_REMOVE_ACTIVE | GuiFlags::EL_ADD_ACTIVE | GuiFlags::SHOW_NODE_INFO | GuiFlags::FORCE_ADD_ACTIVE | GuiFlags::FORCE_REMOVE_ACTIVE
                    | GuiFlags::BC_ADD_ACTIVE | GuiFlags::BC_REMOVE_ACTIVE);
            if (nodePicking) {
                getInput_pickNode(camera, model, modelState);
            }
            else selectedNodes.clear();


            bool elemPicking = ActionFlags & (GuiFlags::EL_REMOVE_ACTIVE | GuiFlags::SHOW_ELEM_INFO | GuiFlags::EL_COPY_ACTIVE);
            if (elemPicking) {
                getInput_pickElem(camera, model, modelState);
            }
            else selectedElems.clear();

            // drawing
            BeginDrawing();
            // Setup the back buffer for drawing (clear color and depth buffers)
                ClearBackground(RAYWHITE);

            
                //if file browser window is active, no rendering happens
                GuiWindowFileDialog(&fileDialogState);
                if (fileDialogState.windowActive) {
                    EndDrawing();
                    continue;
                }

            
                BeginMode3D(camera);


                    RenderElements(model, modelState);

                    RenderNodes(model, modelState);

                EndMode3D();
                //if box selecting draw the box
                SelectionBox& selectionBox = modelState.selectionBox;
                if (selectionBox.active) {
                    DrawRectangleV(selectionBox.get_BottomLeft(), selectionBox.get_TopRight()- selectionBox.get_BottomLeft(), Color{253, 249, 0, 100});
                    //GuiLock();
                    if (IsKeyPressed(KEY_A)) {
                        Vector2 mousePos = GetMousePosition();
                        std::cout<<"asd\n";
                    }
                }
                drawInfo(ActionFlags, camera, model, modelState);
            
                drawGuiActionMenu(ActionFlags, model, modelState);

                drawGuiActionWindows(ActionFlags,model,modelState);

                if (GuiButton(Rectangle{ (float)GetScreenWidth() - 150, 32, 65, 30 }, GuiIconText(ICON_FILE_OPEN, "Load"))) {
                    fileDialogState.windowActive = true;
                    fileDialogState.saveFileMode = false;
                }
                if (GuiButton(Rectangle{ (float)GetScreenWidth() - 75, 32, 65, 30 }, GuiIconText(ICON_FILE_SAVE, "Save"))) {
                    fileDialogState.windowActive = true;
                    fileDialogState.saveFileMode = true;
                }
                // end the frame and get ready for the next one  (display frame, poll input, etc...)
            EndDrawing();

        }

        // cleanup

        // destroy the window and cleanup the OpenGL context
        CloseWindow();
        return;
    }

    void drawGuiActionMenu(uint32_t& ActionFlags, Beams::Model& model, modelDrawState& modelState)
    {
        //------------------------------------------------------------------------------
        //Dereference needed variables
        //------------------------------------------------------------------------------
        int& activeDropdownMenu = modelState.activeDropdownMenu;
        std::vector<size_t>& infoNodes = modelState.infoNodes;
        std::vector<size_t>& selectedNodes = modelState.selectedNodes;
        std::vector<size_t>& infoElems = modelState.infoElems;
        std::vector<size_t>& selectedElems = modelState.selectedElems;
        
        //------------------------------------------------------------------------------
        //Set up all window gui components places and sizes, respective to window size.
        //TODO: Don't create the variables every time, static and change when window changes
        //------------------------------------------------------------------------------

        int height = GetScreenHeight();
        int width = GetScreenWidth();
        static const Rectangle dropdownPos{ 12, 8 + 24, 140, 28 };
        static const Rectangle but1{ 160, 8 + 24, 50, 28 };
        static const Rectangle but2{ 218, 8 + 24, 50, 28 };
        static const Rectangle but3{ 276, 8 + 24, 50, 28 };
        static const Rectangle but4{ 334, 8 + 24, 50, 28 };

        static const Rectangle labelPos{ 12, 10, 140, 24 };
        Rectangle solvePos{ 12, height-40, 140, 28 };

        GuiLabel(labelPos, "Control:");
        if (GuiDropdownBox(dropdownPos, "NODES;ELEMENTS;SECTIONS;FORCES;BCs", &activeDropdownMenu, ActionFlags & GuiFlags::DROPDOWN_EDIT)) ActionFlags ^= GuiFlags::DROPDOWN_EDIT;
        if (GuiButton(solvePos, "Solve!")) {
            model.solve();
        }
            //quality of life improvement
        static int prevDropdown = activeDropdownMenu;
        if (prevDropdown != activeDropdownMenu) {
            prevDropdown = activeDropdownMenu;
            ActionFlags = 0;
        }

        if (activeDropdownMenu == 0) {
            if (GuiButton(but1, "ADD")) {
                ActionFlags = GuiFlags::NODE_ADD_ACTIVE;
            }

            if (GuiButton(but2, "REMOVE")) {
                ActionFlags = GuiFlags::NODE_REMOVE_ACTIVE;
            }

            if (GuiButton(but3, "INFO")) {
                ActionFlags = GuiFlags::SHOW_NODE_INFO;
                for (auto infoPos : infoNodes) {
                    selectedNodes.push_back(infoPos);
                }
            }

        }
        else if (activeDropdownMenu == 1) {
            if (GuiButton(but1, "ADD")) {
                ActionFlags = GuiFlags::EL_ADD_ACTIVE;
            }

            if (GuiButton(but2, "REMOVE")) {
                ActionFlags = GuiFlags::EL_REMOVE_ACTIVE;
            }

            if (GuiButton(but3, "INFO")) {
                ActionFlags = GuiFlags::SHOW_ELEM_INFO;
                for (auto infoPos : infoElems) {
                    selectedElems.push_back(infoPos);
                }
            }
            if (GuiButton(but4, "COPY")) {
                ActionFlags = GuiFlags::EL_COPY_ACTIVE;
            }
            
        }
        else if (activeDropdownMenu == 2) {
            if (GuiButton(but1, "MANAGE")) {
                ActionFlags = GuiFlags::SECTION_WINDOW;
            }

        }

        else if (activeDropdownMenu == 3) {
            if (GuiButton(but1, "ADD")) {
                ActionFlags = GuiFlags::FORCE_ADD_ACTIVE;
            }

            if (GuiButton(but2, "REMOVE")) {
                ActionFlags = GuiFlags::FORCE_REMOVE_ACTIVE;
            }

            if (GuiButton(but3, "INFO")) {
                ActionFlags = GuiFlags::SHOW_FORCE_INFO;
            }
        }
        else if (activeDropdownMenu == 4) {
            if (GuiButton(but1, "ADD")) {
                ActionFlags = GuiFlags::BC_ADD_ACTIVE;
            }

            if (GuiButton(but2, "REMOVE")) {
                ActionFlags = GuiFlags::BC_REMOVE_ACTIVE;
            }

            if (GuiButton(but3, "INFO")) {
                ActionFlags = GuiFlags::SHOW_BC_INFO;
            }
        }

        GuiUnlock();
    }

    void drawGuiActionWindows(uint32_t& ActionFlags, Beams::Model& model, modelDrawState& modelState)
    {
        //TODO! remove Camera from here. Move show info on renderNodes and Elements
        //------------------------------------------------------------------------------
        //Dereference needed variables
        //------------------------------------------------------------------------------
        const Beams::NodeContainer& nodes = model.getNodes();
        const size_t& nodeSize = nodes.size();
        const std::vector<Beams::vBeam>& elements = model.getElements();
        bool& deformed = modelState.deformed;
        std::vector<size_t>& infoNodes = modelState.infoNodes;
        std::vector<size_t>& infoElems = modelState.infoElems;
        std::vector<size_t>& selectedNodes = modelState.selectedNodes;
        std::vector<size_t>& selectedElems = modelState.selectedElems;
        
        //------------------------------------------------------------------------------
        //Set up all window gui components places and sizes, respective to window size.
        //TODO: Don't create the variables every time, static and change when window changes
        //------------------------------------------------------------------------------

        int height = GetScreenHeight();
        int width = GetScreenWidth();

        Rectangle windowPos{ 424.0f/1280.0f*width, 8.0f/800.0f*height, 448.0f/1280.0f*width, 136.0f/800.0f*height };

        Rectangle OkButPos{ 736.0f/1280.0f*width, 112.0f/800.0f*height, 120.0f/1280.0f*width, 24.0f/800.0f*height };
        Rectangle MiddleButPos{ 588.0f / 1280.0f * width, 112.0f / 800.0f * height, 120.0f / 1280.0f * width, 24.0f / 800.0f * height };
        Rectangle clearButPos{ 440.0f/1280.0f*width, 112.0f/800.0f*height, 120.0f/1280.0f*width, 24.0f/800.0f*height };
        Rectangle clearButIndentedPos{ 560.0f / 1280.0f * width, 112.0f / 800.0f * height, 120.0f / 1280.0f * width, 24.0f / 800.0f * height };

        Rectangle xInputPos{ 444.0f/1280.0f*width, 43.0f/800.0f*height, 56.0f/1280.0f*width, 24.0f/800.0f*height };
        Rectangle yInputPos{ 444.0f/1280.0f*width, 75.0f/800.0f*height, 56.0f/1280.0f*width, 24.0f/800.0f*height };
        Rectangle zInputPos{ 444.0f/1280.0f*width, 107.0f/800.0f*height, 56.0f/1280.0f*width, 24.0f/800.0f*height };
        Rectangle xNegPos{ 506.0f/1280.0f*width, 43.0f/800.0f*height, 24.0f/1280.0f*width, 24.0f/800.0f*height };
        Rectangle yNegPos{ 506.0f/1280.0f*width, 75.0f/800.0f*height, 24.0f/1280.0f*width, 24.0f/800.0f*height };
        Rectangle zNegPos{ 506.0f/1280.0f*width, 107.0f/800.0f*height, 24.0f/1280.0f*width, 24.0f/800.0f*height };

        Rectangle textPos{ 436.0f/1280.0f*width, 40.0f/800.0f*height, 400.0f/1280.0f*width, 56.0f/800.0f*height };
        Rectangle textIndentedPos{ 560.0f/1280.0f*width, 43.0f/800.0f*height, 350.0f/1280.0f*width, 56.0f/800.0f*height };
        Rectangle listPos{ 432.0f/1280.0f*width, 40.0f/800.0f*height, 120.0f/1280.0f*width, 96.0f/800.0f*height };

        Rectangle secArea_InputPos{ 580.0f/1280.0f*width, 40.0f/800.0f*height, 56.0f/1280.0f*width, 24.0f/800.0f*height };
        Rectangle secIyy_InputPos{ 580.0f/1280.0f*width, 76.0f/800.0f*height, 56.0f/1280.0f*width, 24.0f/800.0f*height };
        Rectangle secE_InputPos{ 580.0f/1280.0f*width, 112.0f/800.0f*height, 56.0f/1280.0f*width, 24.0f/800.0f*height };
        Rectangle secIxx_InputPos{ 656.0f/1280.0f*width, 40.0f/800.0f*height, 56.0f/1280.0f*width, 24.0f/800.0f*height };
        Rectangle secIzz_InputPos{ 656.0f/1280.0f*width, 76.0f/800.0f*height, 56.0f/1280.0f*width, 24.0f/800.0f*height };
        Rectangle secG_InputPos{ 656.0f/1280.0f*width, 112.0f/800.0f*height, 56.0f/1280.0f*width, 24.0f/800.0f*height };
        Rectangle sec_SaveButPos{ 736.0f / 1280.0f * width, 76.0f / 800.0f * height, 120.0f / 1280.0f * width, 24.0f / 800.0f * height };



        if (GuiFlags::DROPDOWN_EDIT & ActionFlags) GuiLock();

        if (GuiFlags::NODE_ADD_ACTIVE & ActionFlags)
        {

            //static bool xNeg = false;
            //static bool yNeg = false;
            //static bool zNeg = false;
            static bool relativeAdd = false;

            static float xNew;
            static float yNew;
            static float zNew;
            static char xText[33];
            static char yText[33];
            static char zText[33];
            static bool xActive = false, yActive = false, zActive = false;


            if (GuiWindowBox(windowPos, "Add Node")) ActionFlags &= ~GuiFlags::NODE_ADD_ACTIVE;
            if (v_GuiValueBoxFloat(xInputPos, "X", &xText[0], & xNew, xActive)){
                xActive = !xActive;
                if (IsKeyPressed(KEY_TAB)) yActive = true;
            }
            else if (v_GuiValueBoxFloat(yInputPos, "Y", &yText[0], & yNew, yActive)){
                yActive = !yActive;
                if (IsKeyPressed(KEY_TAB) && IsKeyDown(KEY_LEFT_SHIFT)) xActive = true;
                else if (IsKeyPressed(KEY_TAB)) zActive = true;
            }
            else if (v_GuiValueBoxFloat(zInputPos, "Z", &zText[0], &zNew, zActive)){
                zActive = !zActive;
                if (IsKeyPressed(KEY_TAB) && IsKeyDown(KEY_LEFT_SHIFT)) yActive = true;
            }
            else if (IsKeyPressed(KEY_TAB)) xActive = true;
            GuiLabel(textIndentedPos, "Set Coordinates and ADD NODE (or rClick)\nPick existing nodes and toggle Relative Add\n to add nodes relative to the ones picked\nHold LeftCtrl and click or Box select to unpick");

            GuiToggle(clearButIndentedPos, "Relative Add", &relativeAdd);
            if (GuiButton(OkButPos, "ADD NODE!") || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                //NodeAddActive = false;
                //xNew = (xNeg) ? -xNew : xNew;
                //yNew = (yNeg) ? -yNew : yNew;
                //zNew = (zNeg) ? -zNew : zNew;
                if (relativeAdd) {
                    for (size_t selectedPos : selectedNodes) {
                        const Beams::Node& selectedN = nodes.get_byPos(selectedPos);
                        model.addNode(Vector3{ (float)selectedN.x+ (float)xNew,(float)selectedN.y + (float)yNew,(float)selectedN.z + (float)zNew });
                    }
                    selectedNodes.clear();
                }
                else {
                    model.addNode(Vector3{ (float)xNew,(float)yNew,(float)zNew });
                }
            }
        
        }

        if (GuiFlags::NODE_REMOVE_ACTIVE & ActionFlags) {
            {
                static bool showingDuplicates = false;
                static std::unordered_map<size_t, size_t> duplicateNodePositions;
                static bool toggleActive = false;
                if (GuiWindowBox(windowPos, "RemoveNode")) ActionFlags &= ~GuiFlags::NODE_REMOVE_ACTIVE;
                GuiLabel(textPos, "Pick with lClick or Box Select. Hold LeftCtrl to unpick.\nClick CLEAR to clear selection.\nClick REMOVE (or rClick) to remove selected.\nNodes also remove Elements.");

                if (!showingDuplicates) {
                    if (GuiButton(OkButPos, "REMOVE") || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                        std::sort(selectedNodes.rbegin(), selectedNodes.rend());
                        for (size_t selected : selectedNodes) {
                            model.removeNode(selected);
                        }
                        selectedNodes.clear();
                    };

                    if (GuiButton(MiddleButPos, "SHOW DUPLICATES")) {
                        selectedNodes.clear();
                        duplicateNodePositions.clear();
                        duplicateNodePositions = model.findDuplicateNodes();
                        for (auto posPair : duplicateNodePositions) selectedNodes.push_back(posPair.first);
                        showingDuplicates = true;
                    }
                }
                else {


                    if (GuiButton(OkButPos, "REMOVE DUPLICATES") || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                        model.removeDuplicateNodes(duplicateNodePositions);
                        duplicateNodePositions.clear();
                        showingDuplicates = false;
                        selectedNodes.clear();
                    };
                }
                if (GuiButton(clearButPos, "CLEAR")) {
                    selectedNodes.clear();
                    showingDuplicates = false;
                }

            }
        }

        if (GuiFlags::SHOW_NODE_INFO & ActionFlags) {
            {
                if (GuiWindowBox(windowPos, "Node Information")) ActionFlags &= ~GuiFlags::SHOW_NODE_INFO;
                GuiLabel(textPos, "Pick with lClick or Box Select. Hold LeftCtrl to unpick.\nClick CLEAR to clear selection.\nClick SELECT (or rClick) to view info for selected Nodes.\nThe info persists unless the nodes are unpicked or cleared.");
                if (GuiButton(OkButPos, "SELECT") || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                    infoNodes.clear();
                    for (size_t selected : selectedNodes) {
                        infoNodes.push_back(selected);
                    }
                    selectedNodes.clear();
                    ActionFlags &= ~GuiFlags::SHOW_NODE_INFO;
                };
                if (GuiButton(clearButPos, "CLEAR")) {
                    selectedNodes.clear();
                    infoNodes.clear();
                }
            }
        }

        if (GuiFlags::EL_ADD_ACTIVE & ActionFlags) {

            //DrawText("Pick Nodes While Holding Shift", 400, 200, 20, RED);
            static int sectionId = 0;
            static int ListViewTop = 0;
            auto& sections = model.getSections();

            std::string listNames;

            for (auto section : sections) {
                listNames += std::to_string(section.first);
                listNames += ";";
            }
            if (listNames.size() > 0) listNames.erase(listNames.size() - 1);


            if (GuiWindowBox(windowPos, "Create Element")) ActionFlags &= ~GuiFlags::EL_ADD_ACTIVE;
            GuiListView(listPos, listNames.data(), &ListViewTop, &sectionId);
            //std::string info = "Pick 3 nodes and the corresponding section.";
            GuiLabel(textIndentedPos, "Pick 3 nodes and the corresponding section.\nThe third node determines the beam orientation.\nManage Section Properties in \"Sections\" from the dropdown.\nADD ELEMENT (or rClick) adds the element.");
            if ((GuiButton(OkButPos, "ADD ELEMENT") || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) && selectedNodes.size() == 3) {
                //NodeAddActive = false;
                model.addElement(selectedNodes[0], selectedNodes[1], selectedNodes[2], static_cast<size_t>(sectionId));
                selectedNodes.clear();
            }



            if (selectedNodes.size() > 3) selectedNodes.pop_back();


        }

        if (GuiFlags::EL_REMOVE_ACTIVE & ActionFlags) {
            {
                static bool showingDuplicates = false;
                static std::vector<size_t> duplicateElemPositions;

                if (GuiWindowBox(windowPos, "RemoveElement")) ActionFlags &= ~GuiFlags::EL_REMOVE_ACTIVE;
                GuiLabel(textPos, "Pick elements with lClick (or BoxSelect).\nUnpick with LeftCtrl + lClick (or BoxSelect).\nCLEAR clears selection. Remove (or rClick) removes selected.\nDuplicate Elements are only counted if duplicate Nodes are removed");
                if (!showingDuplicates) {
                    if (GuiButton(OkButPos, "REMOVE") || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                        std::sort(selectedElems.rbegin(), selectedElems.rend());//reverse sort
                        for (size_t selected : selectedElems) {
                            model.removeElement(selected);
                        }
                        selectedElems.clear();

                    };
                    if (GuiButton(MiddleButPos, "SHOW DUPLICATES")) {
                        selectedElems.clear();
                        duplicateElemPositions.clear();
                        duplicateElemPositions = model.findDuplicateElements();
                        for (auto pos : duplicateElemPositions) selectedElems.push_back(pos);
                        showingDuplicates = true;
                    }
                }
                else {
                    if (GuiButton(OkButPos, "REMOVE DUPLICATES") || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                        model.removeDuplicateElems(duplicateElemPositions);
                        duplicateElemPositions.clear();
                        showingDuplicates = false;
                        selectedElems.clear();
                    };
                }
                if (GuiButton(clearButPos, "CLEAR")) {
                    selectedElems.clear();
                }
                //GuiToggle(clearButPos, "TOGGLE PICK", &pickElemsActive);
            }
        }

        if (GuiFlags::SHOW_ELEM_INFO & ActionFlags) {
            {

                if (GuiWindowBox(windowPos, "Element Information")) ActionFlags &= ~GuiFlags::SHOW_ELEM_INFO;
                GuiLabel(textPos, "Pick with lClick or Box Select. Hold LeftCtrl to unpick.\nClick CLEAR to clear selection.\nClick SELECT (or rClick) to view persitent info for selected Elements.\nThe Green line is local y-Axis Direction, the Blue line is local z-Axis Direction.");

                if (GuiButton(OkButPos, "Select") || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                    infoElems.clear();
                    for (size_t selected : selectedElems) {
                        infoElems.push_back(selected);
                    }
                    selectedElems.clear();
                    ActionFlags &= ~GuiFlags::SHOW_ELEM_INFO;

                };
                if (GuiButton(clearButPos, "Clear")) {
                    selectedElems.clear();
                    infoElems.clear();
                }
            }
        }

        if (GuiFlags::SECTION_WINDOW & ActionFlags) {
            static int A = 0;
            static int Ixx = 0;
            static int Iyy = 0;
            static int Izz = 0;
            static int E = 0;
            static int G = 0;
            static bool A_Active = false;
            static bool Ixx_Active = false;
            static bool Iyy_Active = false;
            static bool Izz_Active = false;
            static bool E_Active = false;
            static bool G_Active = false;

            static int prevSectionId = -1;
            static int sectionId = 0;
            static int ListViewTop = 0;
            auto& sections = model.getSections();
            bool ReadSectionInfo = prevSectionId == sectionId;
            if (!ReadSectionInfo) prevSectionId = sectionId;
            if (!ReadSectionInfo && sections.find(static_cast<int>(sectionId)) != sections.end()) {
                A = sections.find(static_cast<size_t>(sectionId))->second.Area;
                Ixx = sections.find(static_cast<size_t>(sectionId))->second.Ixx;
                Iyy = sections.find(static_cast<size_t>(sectionId))->second.Iyy;
                Izz = sections.find(static_cast<size_t>(sectionId))->second.Izz;
                E = sections.find(static_cast<size_t>(sectionId))->second.Modulus;
                G = sections.find(static_cast<size_t>(sectionId))->second.G;

            }





            std::string listNames;
            for (auto section : sections) {
                listNames += std::to_string(section.first);
                listNames += ";";
            }
            if (listNames.size() > 0) listNames.erase(listNames.size() - 1);


            if (GuiWindowBox(windowPos, "Sections")) ActionFlags &= ~GuiFlags::SECTION_WINDOW;
            GuiListView(listPos, listNames.data(), &ListViewTop, &sectionId);
            if (GuiValueBox(secArea_InputPos, "Area", &A, 0, 1000000, A_Active)) A_Active = !A_Active;
            if (GuiValueBox(secIyy_InputPos, "Iyy", &Iyy, 0, 1000000, Iyy_Active)) Iyy_Active = !Iyy_Active;
            if (GuiValueBox(secE_InputPos, "E", &E, 0, 1000000, E_Active)) E_Active = !E_Active;
            if (GuiValueBox(secIxx_InputPos, "Ixx", &Ixx, 0, 1000000, Ixx_Active)) Ixx_Active = !Ixx_Active;
            if (GuiValueBox(secIzz_InputPos, "Izz", &Izz, 0, 1000000, Izz_Active)) Izz_Active = !Izz_Active;
            if (GuiValueBox(secG_InputPos, "G", &G, 0, 1000000, G_Active)) G_Active = !G_Active;
            if (GuiButton(OkButPos, "ADD")) {
                model.addSection(A, E, G, Ixx, Iyy, Izz);

            }
            if (GuiButton(sec_SaveButPos, "SAVE") && sections.size() > 0) {
                model.modifySection(sectionId, A, E, G, Ixx, Iyy, Izz);
            }


        }

        if (GuiFlags::FORCE_ADD_ACTIVE & ActionFlags)
        {

            static float xNew;
            static float yNew;
            static float zNew;
            static char xText[33];
            static char yText[33];
            static char zText[33];
            static bool xActive = false, yActive = false, zActive = false;


            if (GuiWindowBox(windowPos, "Add Force")) ActionFlags &= ~GuiFlags::FORCE_ADD_ACTIVE;
            if (v_GuiValueBoxFloat(xInputPos, "X", &xText[0], &xNew, xActive)) {
                xActive = !xActive;
                if (IsKeyPressed(KEY_TAB)) yActive = true;
            }
            else if (v_GuiValueBoxFloat(yInputPos, "Y", &yText[0], &yNew, yActive)) {
                yActive = !yActive;
                if (IsKeyPressed(KEY_TAB) && IsKeyDown(KEY_LEFT_SHIFT)) xActive = true;
                else if (IsKeyPressed(KEY_TAB)) zActive = true;
            }
            else if (v_GuiValueBoxFloat(zInputPos, "Z", &zText[0], &zNew, zActive)) {
                zActive = !zActive;
                if (IsKeyPressed(KEY_TAB) && IsKeyDown(KEY_LEFT_SHIFT)) yActive = true;
            }
            else if (IsKeyPressed(KEY_TAB)) xActive = true;

            GuiLabel(textIndentedPos, "Pick Nodes and insert Force vector coordinates\n\nADD FORCE (or rClick) adds the force to selected Nodes\nCLEAR clears selected Nodes");
            if ((GuiButton(OkButPos, "ADD FORCE") || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))) {
                for (auto nodePos : selectedNodes) {
                    
                    model.addForce(nodePos, 0, xNew);
                    model.addForce(nodePos, 1, yNew);
                    model.addForce(nodePos, 2, zNew);

                }
                selectedNodes.clear();
            }
            if (GuiButton(clearButIndentedPos, "CLEAR")) {
                selectedNodes.clear();
            }

        }

        if (GuiFlags::FORCE_REMOVE_ACTIVE & ActionFlags) {
            {

                static bool toggleActive = false;
                if (GuiWindowBox(windowPos, "Remove Force")) ActionFlags &= ~GuiFlags::FORCE_REMOVE_ACTIVE;
                GuiLabel(textPos, "Pick nodes with left click. Unpick with LeftCtrl + click.\nClick CLEAR (or rClick) to clear selection.\nClick REMOVE to remove selected Forces.\n ");

                if (GuiButton(OkButPos, "REMOVE") || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                    for (size_t selected : selectedNodes) {
                        model.removeForce(selected);
                    }
                    selectedNodes.clear();
                };

                if (GuiButton(clearButPos, "CLEAR")) {
                    selectedNodes.clear();
                }
            }
        }

        if (GuiFlags::BC_ADD_ACTIVE & ActionFlags)
        {

            if (GuiWindowBox(windowPos, "Add Force")) ActionFlags &= ~GuiFlags::BC_ADD_ACTIVE;

            GuiLabel(textPos, "Pick Nodes with left click, unpick with LeftCtrl + click\nADD BC (or rClick) adds the BC(fixed) to selected Nodes\nCLEAR clears selected Nodes\n");
            if ((GuiButton(OkButPos, "ADD BC") || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))) {
                for (auto nodePos : selectedNodes) {
                    model.addBCfixed(nodePos);
                    selectedNodes.clear();
                }
            }
            if (GuiButton(clearButPos, "CLEAR")) {
                selectedNodes.clear();
            }

        }

        if (GuiFlags::BC_REMOVE_ACTIVE & ActionFlags) {
            {

                static bool toggleActive = false;
                if (GuiWindowBox(windowPos, "Remove Force")) ActionFlags &= ~GuiFlags::BC_REMOVE_ACTIVE;
                GuiLabel(textPos, "Pick nodes with left click. Unpick with LeftCtrl + click.\nClick CLEAR (or rClick) to clear selection.\nClick REMOVE to remove selected Forces.\n ");

                if (GuiButton(OkButPos, "REMOVE") || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                    for (size_t selected : selectedNodes) {
                        model.removeBCfixed(selected);
                    }
                    selectedNodes.clear();
                };

                if (GuiButton(clearButPos, "CLEAR")) {
                    selectedNodes.clear();
                }
            }
        }

        if (GuiFlags::EL_COPY_ACTIVE & ActionFlags) {
            static float xNew;
            static float yNew;
            static float zNew;
            static char xText[33];
            static char yText[33];
            static char zText[33];
            static bool xActive = false, yActive = false, zActive = false;
            


            if (GuiWindowBox(windowPos, "Element Copy")) ActionFlags &= ~GuiFlags::NODE_ADD_ACTIVE;
            if (v_GuiValueBoxFloat(xInputPos, "X", &xText[0], &xNew, xActive)) {
                xActive = !xActive;
                if (IsKeyPressed(KEY_TAB)) yActive = true;
            }
            else if (v_GuiValueBoxFloat(yInputPos, "Y", &yText[0], &yNew, yActive)) {
                yActive = !yActive;
                if (IsKeyPressed(KEY_TAB) && IsKeyDown(KEY_LEFT_SHIFT)) xActive = true;
                else if (IsKeyPressed(KEY_TAB)) zActive = true;
            }
            else if (v_GuiValueBoxFloat(zInputPos, "Z", &zText[0], &zNew, zActive)) {
                zActive = !zActive;
                if (IsKeyPressed(KEY_TAB) && IsKeyDown(KEY_LEFT_SHIFT)) yActive = true;
            }
            else if (IsKeyPressed(KEY_TAB)) xActive = true;

            GuiLabel(textIndentedPos, "Pick Elements to copy and add offsets in the dialog\n\n COPY (or rClick) performs the operation\n");


            if (GuiButton(OkButPos, "COPY") || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                //NodeAddActive = false;


                model.copyElements(selectedElems, Vector3{ (float)xNew,(float)yNew,(float)zNew });
                selectedElems.clear();
            }
            if (GuiButton(clearButIndentedPos, "CLEAR")) {
                selectedElems.clear();
            }

        }
    }

    void drawInfo(uint32_t& ActionFlags, const Camera& camera, Beams::Model& model,modelDrawState& modelState )
    {
        const Beams::NodeContainer& nodes = model.getNodes();
        const std::vector<Beams::vBeam>& elements = model.getElements();
        bool& deformed = modelState.deformed;
        std::vector<size_t>& infoNodes = modelState.infoNodes;
        std::vector<size_t>& infoElems = modelState.infoElems;

        if (GuiFlags::SHOW_NODE_INFO & ActionFlags) {
            for (auto& notDeleted : nodes) {
                const Beams::Node& node = notDeleted;
                Vector3 nodeCoords{ node.xRender,node.yRender,node.zRender };

                if (deformed) {

                    nodeCoords += model.getDeflectionRender(node.matrixPos);
                }

                Vector2 nodeScreenPos = GetWorldToScreen(nodeCoords, camera);
                char a[20];
                int matrixPos = (node.free_flag) ? -1 : (int)node.matrixPos;
                sprintf_s(a, "ID %d\nDoF Group: %d",node.pos, matrixPos);
                DrawText(a, (int)nodeScreenPos.x - MeasureText(a, 20) / 2, (int)nodeScreenPos.y, 20, BLACK);
            }
        }
        else {
            for (auto infoPos : infoNodes) {
                const Beams::Node& node = nodes.get_byPos(infoPos);
                Vector3 nodeCoords{ node.xRender,node.yRender,node.zRender };
                Vector3 deforms = model.getDeflectionRender(node.matrixPos);
                if (deformed) {
                    nodeCoords += deforms;
                }
                Vector3 deformsCorrect = model.getDeflection(node.matrixPos);
                Vector2 nodeScreenPos = GetWorldToScreen(nodeCoords, camera);
                char a[64];
                sprintf_s(a, "ID %d\nDeformations:\nX: %f\nY: %f\nZ: %f", (int)node.pos, deformsCorrect.x, deformsCorrect.y, deformsCorrect.z);
                DrawText(a, (int)nodeScreenPos.x - MeasureText(a, 20) / 2, (int)nodeScreenPos.y, 20, BLACK);
            }
        }

        if (GuiFlags::SHOW_ELEM_INFO & ActionFlags) {   
            for (auto& element : elements) {
                const Beams::Node& node1 = nodes.get_byPos(element.node1Pos);
                const Beams::Node& node2 = nodes.get_byPos(element.node2Pos);


                Vector3 n1{ node1.xRender,node1.yRender,node1.zRender };
                Vector3 n2{ node2.xRender,node2.yRender,node2.zRender };
                if (deformed) {
                    n1 += model.getDeflectionRender(node1.matrixPos);
                    n2 += model.getDeflectionRender(node2.matrixPos);
                }
                Vector3 middlePosition = Vector3Add(n1, Vector3Subtract(n2, n1) / 2);



                Vector2 elScreenPos = GetWorldToScreen(middlePosition, camera);
                char a[20];
                sprintf_s(a, "NIDs %d | %d", (int)node1.matrixPos, (int)node2.matrixPos);
                DrawText(a, (int)elScreenPos.x - MeasureText(a, 20) / 2, (int)elScreenPos.y, 20, BLACK);

                //Draw Local Axes
                auto localVectors = element.getLocalUnitVectors();
                BeginMode3D(camera); 
                //DrawLine3D(middlePosition, Vector3Add(Vector3{ (float)localVectors[0][0], (float)localVectors[0][1], (float)localVectors[0][2] }, middlePosition), RED); //X axis is not visible
                DrawLine3D(middlePosition, Vector3Add(Vector3{ (float)localVectors[1][0], (float)localVectors[1][1], (float)localVectors[1][2] }, middlePosition), GREEN);
                DrawLine3D(middlePosition, Vector3Add(Vector3{ (float)localVectors[2][0], (float)localVectors[2][1], (float)localVectors[2][2] }, middlePosition), BLUE);
                EndMode3D();
            }
        }
        else {
            for (auto infoPos : infoElems) {
                const Beams::Node& node1 = nodes.get_byPos(elements[infoPos].node1Pos);
                const Beams::Node& node2 = nodes.get_byPos(elements[infoPos].node2Pos);


                Vector3 n1{ node1.xRender,node1.yRender,node1.zRender };
                Vector3 n2{ node2.xRender,node2.yRender,node2.zRender };
                if (deformed) {
                    n1 += model.getDeflectionRender(node1.matrixPos);
                    n2 += model.getDeflectionRender(node2.matrixPos);
                }
                Vector3 middlePosition = Vector3Add(n1, Vector3Subtract(n2, n1) / 2);

                Vector2 elScreenPos = GetWorldToScreen(middlePosition, camera);
                
                char a[20];
                sprintf_s(a, "NIDs %d | %d", (int)node1.matrixPos, (int)node2.matrixPos);
                DrawText(a, (int)elScreenPos.x - MeasureText(a, 20) / 2, (int)elScreenPos.y, 20, BLACK);

                //Draw Local Axes
                auto localVectors = elements[infoPos].getLocalUnitVectors();
                BeginMode3D(camera);
                //DrawLine3D(middlePosition, Vector3Add(Vector3{ (float)localVectors[0][0], (float)localVectors[0][1], (float)localVectors[0][2] }, middlePosition), RED); //X axis is not visible
                DrawLine3D(middlePosition, Vector3Add(Vector3{ (float)localVectors[1][0], (float)localVectors[1][1], (float)localVectors[1][2] }, middlePosition), GREEN);
                DrawLine3D(middlePosition, Vector3Add(Vector3{ (float)localVectors[2][0], (float)localVectors[2][1], (float)localVectors[2][2] }, middlePosition), BLUE);
                EndMode3D();
            }
        }
        
        if (GuiFlags::SHOW_BC_INFO & ActionFlags) {
            {
                for (auto& bc : model.getBCfixed()) {
                    const Beams::Node& node = nodes.get_byPos(bc);
                    Vector3 nodeCoords{ node.xRender,node.yRender,node.zRender };

                    if (deformed) {

                        nodeCoords += model.getDeflectionRender(node.matrixPos);
                    }

                    Vector2 nodeScreenPos = GetWorldToScreen(nodeCoords, camera);
                    char a[64];
                    sprintf_s(a, "Fixed");
                    DrawText(a, (int)nodeScreenPos.x - MeasureText(a, 20) / 2, (int)nodeScreenPos.y, 20, BLACK);
                }
            }
        }

        if (GuiFlags::SHOW_FORCE_INFO & ActionFlags) {
            {
                for (auto& force : model.getForces()) {
                    const Beams::Node& node = nodes.get_byPos(force.first);
                    Vector3 nodeCoords{ node.xRender,node.yRender,node.zRender };

                    if (deformed) {

                        nodeCoords += model.getDeflectionRender(node.matrixPos);
                    }

                    Vector2 nodeScreenPos = GetWorldToScreen(nodeCoords, camera);
                    char a[64];
                    sprintf_s(a, "Force:\nX: %f\nY: %f\nZ: %f", force.second[0], force.second[1], force.second[2]);
                    DrawText(a, (int)nodeScreenPos.x - MeasureText(a, 20) / 2, (int)nodeScreenPos.y, 20, BLACK);
                }
            }
        }

    }

    //draws Elements, element picking points(when elements menu is active) and overlays on selected elements.
    void RenderElements(Beams::Model& model, modelDrawState& modelState)
    {
        const std::vector<Beams::vBeam>& elements = model.getElements();
        const Beams::NodeContainer& nodes = model.getNodes();
        bool deformed = modelState.deformed;
        int activeDropdownMenu = modelState.activeDropdownMenu;
        std::vector<size_t>& selectedElems = modelState.selectedElems;

        for (auto& element : elements) {
            const Beams::Node& node1 = nodes.get_byPos(element.node1Pos);
            const Beams::Node& node2 = nodes.get_byPos(element.node2Pos);

            Vector3 n1{ node1.xRender,node1.yRender,node1.zRender };
            Vector3 n2{ node2.xRender,node2.yRender,node2.zRender };

            if (deformed) {
                n1 += model.getDeflectionRender(node1.matrixPos);
                n2 += model.getDeflectionRender(node2.matrixPos);
            }

            if (activeDropdownMenu == 1) {
                Vector3 middlePosition = Vector3Add(n1, (Vector3Subtract(n2, n1) / 2));
                DrawSphere(middlePosition, 0.15f, BLUE);
                
                

            }
            DrawCapsule(n1, n2, 0.1f, 2, 2, BLUE);
        }

        for (size_t pos : selectedElems) {
            const Beams::Node& node1 = nodes.get_byPos(elements[pos].node1Pos);
            const Beams::Node& node2 = nodes.get_byPos(elements[pos].node2Pos);

            Vector3 n1{ node1.xRender,node1.yRender,node1.zRender };
            Vector3 n2{ node2.xRender,node2.yRender,node2.zRender };
            DrawSphereWires(Vector3Add(n1, (Vector3Subtract(n2, n1) / 2)), 0.2, 5, 5, DARKGREEN);
        }
    }

    //TODO: Remove code duplication in pick Nodes/Elements
    void getInput_pickNode(const Camera3D& camera,  Beams::Model& model, modelDrawState& modelState) {
        const Beams::NodeContainer& nodes = model.getNodes();
        std::vector<size_t>& selectedNodes = modelState.selectedNodes;
        bool deformed = modelState.deformed;
        SelectionBox& selectionBox = modelState.selectionBox;
        
        
        //if Click, set the first box selection point (if no box selection happens, this changes later)
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectionBox.set_point1(GetMousePosition());
        }

        //else if draging with lmb down to create box, set box selection active and update the second box selection point every frame
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && Vector2Length(GetMouseDelta()) > 0.1f) {
            selectionBox.active = true;
            selectionBox.set_point2(GetMousePosition());
        }

        //else if releasing the lmb with ctrl pressed, do proper action (box or point unselect)
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {



            for (auto& notDeleted : nodes) {
                const Beams::Node& node = notDeleted;
                Vector3 nodeCenter{ node.xRender,node.yRender,node.zRender };
                Vector2 nodeCenter_ScreenPos = GetWorldToScreen(nodeCenter, camera);

                if (deformed) {
                    nodeCenter += model.getDeflectionRender(node.matrixPos);
                }

                bool isSelected = false;
                if (selectionBox.active) {
                    selectionBox.set_point2(GetMousePosition()); //Moving during release -> get the latest pos
                    

                    Vector2 selectionBottomLeft = selectionBox.get_BottomLeft();
                    Vector2 selectionTopRight = selectionBox.get_TopRight();

                    bool in_xRange = (selectionBottomLeft.x < nodeCenter_ScreenPos.x && nodeCenter_ScreenPos.x < selectionTopRight.x);
                    bool in_yRange = (selectionBottomLeft.y < nodeCenter_ScreenPos.y && nodeCenter_ScreenPos.y < selectionTopRight.y);

                    isSelected = in_xRange && in_yRange;

                    if (isSelected && IsKeyDown(KEY_LEFT_CONTROL)) {
                        auto it = std::find(selectedNodes.begin(), selectedNodes.end(), node.pos);
                        if (it != selectedNodes.end()) selectedNodes.erase(it);
                    }
                    else if (isSelected && (std::find(selectedNodes.begin(), selectedNodes.end(), node.pos) == selectedNodes.end())) {
                        selectedNodes.push_back(node.pos);
                    }
                }
                else {//not Box Selecting
                    //TODO: Pick closest if they overlap
                    //TODO: Dynamic Pick Size
                    Vector2 mousePos = GetMousePosition();
                    float distance2d = Vector2Length(nodeCenter_ScreenPos - mousePos);
                    
                    //node is selected if mouse release is close enough to node 
                    isSelected = distance2d < 20.0f;

                    if (isSelected && IsKeyDown(KEY_LEFT_CONTROL)) {
                        auto it = std::find(selectedNodes.begin(), selectedNodes.end(), node.pos);
                        if (it != selectedNodes.end()) selectedNodes.erase(it);
                        return;
                    }
                    else if (isSelected&& (std::find(selectedNodes.begin(), selectedNodes.end(), node.pos) == selectedNodes.end())) {
                        selectedNodes.push_back(node.pos);
                        return;
                    }
                }
            }
            selectionBox.active = false;//always turn off box selection on mouse release


        }

        
        return ;
    }

    void getInput_pickElem(const Camera3D& camera, Beams::Model& model, modelDrawState& modelState) {
        const std::vector<Beams::vBeam>& elements = model.getElements();
        std::vector<size_t>& selectedElems = modelState.selectedElems;
        bool deformed = modelState.deformed;
        SelectionBox& selectionBox = modelState.selectionBox;


        //if Click, set the first box selection point (if no box selection happens, this changes later)
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectionBox.set_point1(GetMousePosition());
        }

        //else if draging with lmb down to create box, set box selection active and update the second box selection point every frame
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && Vector2Length(GetMouseDelta()) > 0.1f) {
            selectionBox.active = true;
            selectionBox.set_point2(GetMousePosition());
        }

        //else if releasing the lmb with ctrl pressed, do proper action (box or point unselect)
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {

            for (size_t i = 0; i < elements.size(); i++) {
                const Beams::Node& node1 = model.getNodes().get_byPos(elements[i].node1Pos);
                const Beams::Node& node2 = model.getNodes().get_byPos(elements[i].node2Pos);



                Vector3 n1{ node1.xRender,node1.yRender,node1.zRender };
                Vector3 n2{ node2.xRender,node2.yRender,node2.zRender };

                if (deformed) {
                    n1+= model.getDeflectionRender(node1.matrixPos);
                    n2 += model.getDeflectionRender(node2.matrixPos);
                }

                Vector3 elemCenter = Vector3Add(n1, (Vector3Subtract(n2, n1) / 2));
                Vector2 elemCenter_ScreenPos = GetWorldToScreen(elemCenter,camera);

                bool isSelected = false;
                if (selectionBox.active) {
                    selectionBox.set_point2(GetMousePosition()); //Moving during release -> get the latest pos


                    Vector2 selectionBottomLeft = selectionBox.get_BottomLeft();
                    Vector2 selectionTopRight = selectionBox.get_TopRight();

                    bool in_xRange = (selectionBottomLeft.x < elemCenter_ScreenPos.x && elemCenter_ScreenPos.x < selectionTopRight.x);
                    bool in_yRange = (selectionBottomLeft.y < elemCenter_ScreenPos.y && elemCenter_ScreenPos.y < selectionTopRight.y);

                    isSelected = in_xRange && in_yRange;

                    if (isSelected && IsKeyDown(KEY_LEFT_CONTROL)) {
                        auto it = std::find(selectedElems.begin(), selectedElems.end(), i);
                        if (it != selectedElems.end()) selectedElems.erase(it);
                    }
                    else if (isSelected && (std::find(selectedElems.begin(), selectedElems.end(), i) == selectedElems.end())) {
                        selectedElems.push_back(i);
                    }
                }
                else {//not Box Selecting
                    //TODO: Pick closest if they overlap
                    //TODO: Dynamic Pick Size
                    Vector2 mousePos = GetMousePosition();
                    float distance2d = Vector2Length(elemCenter_ScreenPos - mousePos);

                    //node is selected if mouse release is close enough to node 
                    isSelected = distance2d < 20.0f;

                    if (isSelected && IsKeyDown(KEY_LEFT_CONTROL)) {
                        auto it = std::find(selectedElems.begin(), selectedElems.end(), i);
                        if (it != selectedElems.end()) selectedElems.erase(it);
                        return;
                    }
                    else if (isSelected && (std::find(selectedElems.begin(), selectedElems.end(), i) == selectedElems.end())) {
                        selectedElems.push_back(i);
                        return;
                    }
                }
            }
            selectionBox.active = false;//always turn off box selection on mouse release


        }

        return;
    }

    //draws Nodes,Forces,BCs & overlays on selected nodes
    void RenderNodes(Beams::Model& model, modelDrawState& modelState)
    {
        
        const Beams::NodeContainer& nodes = model.getNodes();
        bool& deformed  = modelState.deformed;
        std::vector<size_t>& selected = modelState.selectedNodes;
        

        for (auto& notDeleted : nodes) {
            const Beams::Node& node = notDeleted;
            Vector3 nodeCoords{ node.xRender ,node.yRender ,node.zRender };


            if (deformed) {
                nodeCoords += model.getDeflectionRender(node.matrixPos);
            }
            DrawSphere(nodeCoords, 0.15f, RED);
        }

        for (auto forceIt : model.getForces()) {
            const Beams::Node& node = nodes.get_byPos(forceIt.first);
            Vector3 nodeCoords{ node.xRender ,node.yRender ,node.zRender };
            if (deformed) {
                nodeCoords += model.getDeflectionRender(node.matrixPos);
            }


            Vector3 force{ forceIt.second[0],forceIt.second[1],forceIt.second[2] };
            if (std::abs(force.x) + std::abs(force.y) + std::abs(force.z) > 0.001) {
                force = Vector3Normalize(force);
                Vector3 forceEndpoint = Vector3Add(nodeCoords, force);
                DrawLine3D(nodeCoords, forceEndpoint, DARKBROWN);
                DrawCylinderEx(Vector3Add(nodeCoords, force / 2), forceEndpoint, 0.1f, 0.0f, 10, DARKBROWN);
            }
        }

        for (auto BC : model.getBCfixed()) {
            const Beams::Node& node = nodes.get_byPos(BC);
            Vector3 nodeCoords{ node.xRender ,node.yRender ,node.zRender };
            DrawCubeWires(nodeCoords, 1.0f, 1.0f, 1.0f, MAROON);
        }

        for (size_t pos : selected) {
            const Beams::Node& node = nodes.get_byPos(pos);
            Vector3 nodeCoords{ node.xRender ,node.yRender ,node.zRender };
            if (deformed) {
                nodeCoords += model.getDeflectionRender(node.matrixPos);
            }
            DrawSphereWires(nodeCoords, 0.2, 5, 5, DARKGREEN);
        }

        //DrawCube(Vector3Zero(), 2.0f, 2.0f, 2.0f, RED);
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

        camera.fovy = 5.0f; // Camera field-of-view Y
        camera.projection = CAMERA_PERSPECTIVE; //Hackia - orthographic camera zoom does not work properly. FoV~ 5 is close enough




        SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

        // Tell the window to use vsync and work on high DPI displays
        //SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);


        // Create the window and OpenGL context
        InitWindow(screenWidth, screenHeight, "VBeams");
        SetWindowMinSize(1000, 692);
    }

    void getInput_CameraControl(Camera3D& camera,Beams::Model& model)
    {
        //Move Camera to 0,0,0
        if (IsKeyPressed(KEY_Z)) { 
            camera.target = Vector3Zero();
        }
        //Pan camera
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            UpdateCameraProAroundTarget(&camera, Vector3{ 0,-GetMouseDelta().x * 0.03f,GetMouseDelta().y * 0.03f}, Vector3{0,0,0}, 0.0f);
           
        }
        //Orbit Camera
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
        //Center Camera to Node
        else if (IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE)) {
            Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);
            //for (size_t i = 0; i < nodeSize; i++) {
            auto& nodes = model.getNodes();
            for (auto& notDeletedIt : nodes) {
                const Beams::Node& node = notDeletedIt;
                Vector3 nodeCenter{ node.xRender,node.yRender,node.zRender };
                RayCollision collision = GetRayCollisionSphere(ray, nodeCenter, 0.15f);
                if (collision.hit) {
                    camera.target = nodeCenter;
                }
            }
        }
        //Zoom
        else {
            UpdateCameraProAroundTarget(&camera, Vector3{ 0,0,0 }, Vector3{ 0,0,0 }, GetMouseWheelMove() * 10.0f);
            SetMouseCursor(MOUSE_CURSOR_ARROW);

        }
    }

}