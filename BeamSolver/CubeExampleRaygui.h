//#pragma once 
//#include "raylib.h"
//#include "rlgl.h"
//
//#include <stddef.h>     // Required for: NULL
//#include <math.h>       // Required for: sinf()
//
//
//int drawfunc() {
//    const int screenWidth = 800;
//    const int screenHeight = 450;
//
//    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
//    InitWindow(screenWidth, screenHeight, "raylib [text] example - draw 2D text in 3D");
//
//    bool spin = true;        // Spin the camera?
//    bool multicolor = false; // Multicolor mode
//
//    // Define the camera to look into our 3d world
//    Camera3D camera = { 0 };
//    camera.position = (Vector3){ -10.0f, 15.0f, -10.0f };   // Camera position
//    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };          // Camera looking at point
//    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };              // Camera up vector (rotation towards target)
//    camera.fovy = 45.0f;                                    // Camera field-of-view Y
//    camera.projection = CAMERA_ORTHOGRAPHIC;                 // Camera projection type
//
//    int camera_mode = CAMERA_ORBITAL;
//
//    Vector3 cubePosition = { 0.0f, 1.0f, 0.0f };
//    Vector3 cubeSize = { 2.0f, 2.0f, 2.0f };
//
//    // Use the default font
//    Font font = GetFontDefault();
//    float fontSize = 8.0f;
//    float fontSpacing = 0.5f;
//    float lineSpacing = -1.0f;
//
//    // Set the text (using markdown!)
//    char text[64] = "Hello ~~World~~ in 3D!";
//    Vector3 tbox = { 0 };
//    int layers = 1;
//    int quads = 0;
//    float layerDistance = 0.01f;
//
//    WaveTextConfig wcfg;
//    wcfg.waveSpeed.x = wcfg.waveSpeed.y = 3.0f; wcfg.waveSpeed.z = 0.5f;
//    wcfg.waveOffset.x = wcfg.waveOffset.y = wcfg.waveOffset.z = 0.35f;
//    wcfg.waveRange.x = wcfg.waveRange.y = wcfg.waveRange.z = 0.45f;
//
//    float time = 0.0f;
//
//    // Setup a light and dark color
//    Color light = MAROON;
//    Color dark = RED;
//
//    // Load the alpha discard shader
//    Shader alphaDiscard = LoadShader(NULL, "resources/shaders/glsl330/alpha_discard.fs");
//
//    // Array filled with multiple random colors (when multicolor mode is set)
//    Color multi[TEXT_MAX_LAYERS] = { 0 };
//
//    DisableCursor();                    // Limit cursor to relative movement inside the window
//
//    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
//    //--------------------------------------------------------------------------------------
//
//    // Main game loop
//    while (!WindowShouldClose())        // Detect window close button or ESC key
//    {
//        // Update
//        //----------------------------------------------------------------------------------
//        UpdateCamera(&camera, camera_mode);
//
//        // Handle font files dropped
//        if (IsFileDropped())
//        {
//            FilePathList droppedFiles = LoadDroppedFiles();
//
//            // NOTE: We only support first ttf file dropped
//            if (IsFileExtension(droppedFiles.paths[0], ".ttf"))
//            {
//                UnloadFont(font);
//                font = LoadFontEx(droppedFiles.paths[0], (int)fontSize, 0, 0);
//            }
//            else if (IsFileExtension(droppedFiles.paths[0], ".fnt"))
//            {
//                UnloadFont(font);
//                font = LoadFont(droppedFiles.paths[0]);
//                fontSize = (float)font.baseSize;
//            }
//
//            UnloadDroppedFiles(droppedFiles);    // Unload filepaths from memory
//        }
//
//        // Handle Events
//        if (IsKeyPressed(KEY_F1)) SHOW_LETTER_BOUNDRY = !SHOW_LETTER_BOUNDRY;
//        if (IsKeyPressed(KEY_F2)) SHOW_TEXT_BOUNDRY = !SHOW_TEXT_BOUNDRY;
//        if (IsKeyPressed(KEY_F3))
//        {
//            // Handle camera change
//            spin = !spin;
//            // we need to reset the camera when changing modes
//            camera = (Camera3D){ 0 };
//            camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };          // Camera looking at point
//            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };              // Camera up vector (rotation towards target)
//            camera.fovy = 45.0f;                                    // Camera field-of-view Y
//            camera.projection = CAMERA_PERSPECTIVE;                 // Camera mode type
//
//            if (spin)
//            {
//                camera.position = (Vector3){ -10.0f, 15.0f, -10.0f };   // Camera position
//                camera_mode = CAMERA_ORBITAL;
//            }
//            else
//            {
//                camera.position = (Vector3){ 10.0f, 10.0f, -10.0f };   // Camera position
//                camera_mode = CAMERA_FREE;
//            }
//        }
//
//        // Handle clicking the cube
//        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
//        {
//            Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);
//
//            // Check collision between ray and box
//            RayCollision collision = GetRayCollisionBox(ray,
//                (BoundingBox) {
//                (Vector3) {
//                cubePosition.x - cubeSize.x / 2, cubePosition.y - cubeSize.y / 2, cubePosition.z - cubeSize.z / 2
//            },
//                    (Vector3) {
//                    cubePosition.x + cubeSize.x / 2, cubePosition.y + cubeSize.y / 2, cubePosition.z + cubeSize.z / 2
//                }
//            });
//            if (collision.hit)
//            {
//                // Generate new random colors
//                light = GenerateRandomColor(0.5f, 0.78f);
//                dark = GenerateRandomColor(0.4f, 0.58f);
//            }
//        }
//
//        // Handle text layers changes
//        if (IsKeyPressed(KEY_HOME)) { if (layers > 1) --layers; }
//        else if (IsKeyPressed(KEY_END)) { if (layers < TEXT_MAX_LAYERS) ++layers; }
//
//        // Handle text changes
//        if (IsKeyPressed(KEY_LEFT)) fontSize -= 0.5f;
//        else if (IsKeyPressed(KEY_RIGHT)) fontSize += 0.5f;
//        else if (IsKeyPressed(KEY_UP)) fontSpacing -= 0.1f;
//        else if (IsKeyPressed(KEY_DOWN)) fontSpacing += 0.1f;
//        else if (IsKeyPressed(KEY_PAGE_UP)) lineSpacing -= 0.1f;
//        else if (IsKeyPressed(KEY_PAGE_DOWN)) lineSpacing += 0.1f;
//        else if (IsKeyDown(KEY_INSERT)) layerDistance -= 0.001f;
//        else if (IsKeyDown(KEY_DELETE)) layerDistance += 0.001f;
//        else if (IsKeyPressed(KEY_TAB))
//        {
//            multicolor = !multicolor;   // Enable /disable multicolor mode
//
//            if (multicolor)
//            {
//                // Fill color array with random colors
//                for (int i = 0; i < TEXT_MAX_LAYERS; ++i)
//                {
//                    multi[i] = GenerateRandomColor(0.5f, 0.8f);
//                    multi[i].a = GetRandomValue(0, 255);
//                }
//            }
//        }
//
//        // Handle text input
//        int ch = GetCharPressed();
//        if (IsKeyPressed(KEY_BACKSPACE))
//        {
//            // Remove last char
//            int len = TextLength(text);
//            if (len > 0) text[len - 1] = '\0';
//        }
//        else if (IsKeyPressed(KEY_ENTER))
//        {
//            // handle newline
//            int len = TextLength(text);
//            if (len < sizeof(text) - 1)
//            {
//                text[len] = '\n';
//                text[len + 1] = '\0';
//            }
//        }
//        else
//        {
//            // append only printable chars
//            int len = TextLength(text);
//            if (len < sizeof(text) - 1)
//            {
//                text[len] = ch;
//                text[len + 1] = '\0';
//            }
//        }
//
//        // Measure 3D text so we can center it
//        tbox = MeasureTextWave3D(font, text, fontSize, fontSpacing, lineSpacing);
//
//        quads = 0;                      // Reset quad counter
//        time += GetFrameTime();         // Update timer needed by `DrawTextWave3D()`
//        //----------------------------------------------------------------------------------
//
//        // Draw
//        //----------------------------------------------------------------------------------
//        BeginDrawing();
//
//        ClearBackground(RAYWHITE);
//
//        BeginMode3D(camera);
//        DrawCubeV(cubePosition, cubeSize, dark);
//        DrawCubeWires(cubePosition, 2.1f, 2.1f, 2.1f, light);
//
//        DrawGrid(10, 2.0f);
//
//        // Use a shader to handle the depth buffer issue with transparent textures
//        // NOTE: more info at https://bedroomcoders.co.uk/posts/198
//        BeginShaderMode(alphaDiscard);
//
//        // Draw the 3D text above the red cube
//        rlPushMatrix();
//        rlRotatef(90.0f, 1.0f, 0.0f, 0.0f);
//        rlRotatef(90.0f, 0.0f, 0.0f, -1.0f);
//
//        for (int i = 0; i < layers; ++i)
//        {
//            Color clr = light;
//            if (multicolor) clr = multi[i];
//            DrawTextWave3D(font, text, (Vector3) { -tbox.x / 2.0f, layerDistance* i, -4.5f }, fontSize, fontSpacing, lineSpacing, true, & wcfg, time, clr);
//        }
//
//        // Draw the text boundry if set
//        if (SHOW_TEXT_BOUNDRY) DrawCubeWiresV((Vector3) { 0.0f, 0.0f, -4.5f + tbox.z / 2 }, tbox, dark);
//        rlPopMatrix();
//
//        // Don't draw the letter boundries for the 3D text below
//        bool slb = SHOW_LETTER_BOUNDRY;
//        SHOW_LETTER_BOUNDRY = false;
//
//        // Draw 3D options (use default font)
//        //-------------------------------------------------------------------------
//        rlPushMatrix();
//        rlRotatef(180.0f, 0.0f, 1.0f, 0.0f);
//        char* opt = (char*)TextFormat("< SIZE: %2.1f >", fontSize);
//        quads += TextLength(opt);
//        Vector3 m = MeasureText3D(GetFontDefault(), opt, 8.0f, 1.0f, 0.0f);
//        Vector3 pos = { -m.x / 2.0f, 0.01f, 2.0f };
//        DrawText3D(GetFontDefault(), opt, pos, 8.0f, 1.0f, 0.0f, false, BLUE);
//        pos.z += 0.5f + m.z;
//
//        opt = (char*)TextFormat("< SPACING: %2.1f >", fontSpacing);
//        quads += TextLength(opt);
//        m = MeasureText3D(GetFontDefault(), opt, 8.0f, 1.0f, 0.0f);
//        pos.x = -m.x / 2.0f;
//        DrawText3D(GetFontDefault(), opt, pos, 8.0f, 1.0f, 0.0f, false, BLUE);
//        pos.z += 0.5f + m.z;
//
//        opt = (char*)TextFormat("< LINE: %2.1f >", lineSpacing);
//        quads += TextLength(opt);
//        m = MeasureText3D(GetFontDefault(), opt, 8.0f, 1.0f, 0.0f);
//        pos.x = -m.x / 2.0f;
//        DrawText3D(GetFontDefault(), opt, pos, 8.0f, 1.0f, 0.0f, false, BLUE);
//        pos.z += 1.0f + m.z;
//
//        opt = (char*)TextFormat("< LBOX: %3s >", slb ? "ON" : "OFF");
//        quads += TextLength(opt);
//        m = MeasureText3D(GetFontDefault(), opt, 8.0f, 1.0f, 0.0f);
//        pos.x = -m.x / 2.0f;
//        DrawText3D(GetFontDefault(), opt, pos, 8.0f, 1.0f, 0.0f, false, RED);
//        pos.z += 0.5f + m.z;
//
//        opt = (char*)TextFormat("< TBOX: %3s >", SHOW_TEXT_BOUNDRY ? "ON" : "OFF");
//        quads += TextLength(opt);
//        m = MeasureText3D(GetFontDefault(), opt, 8.0f, 1.0f, 0.0f);
//        pos.x = -m.x / 2.0f;
//        DrawText3D(GetFontDefault(), opt, pos, 8.0f, 1.0f, 0.0f, false, RED);
//        pos.z += 0.5f + m.z;
//
//        opt = (char*)TextFormat("< LAYER DISTANCE: %.3f >", layerDistance);
//        quads += TextLength(opt);
//        m = MeasureText3D(GetFontDefault(), opt, 8.0f, 1.0f, 0.0f);
//        pos.x = -m.x / 2.0f;
//        DrawText3D(GetFontDefault(), opt, pos, 8.0f, 1.0f, 0.0f, false, DARKPURPLE);
//        rlPopMatrix();
//        //-------------------------------------------------------------------------
//
//        // Draw 3D info text (use default font)
//        //-------------------------------------------------------------------------
//        opt = "All the text displayed here is in 3D";
//        quads += 36;
//        m = MeasureText3D(GetFontDefault(), opt, 10.0f, 0.5f, 0.0f);
//        pos = (Vector3){ -m.x / 2.0f, 0.01f, 2.0f };
//        DrawText3D(GetFontDefault(), opt, pos, 10.0f, 0.5f, 0.0f, false, DARKBLUE);
//        pos.z += 1.5f + m.z;
//
//        opt = "press [Left]/[Right] to change the font size";
//        quads += 44;
//        m = MeasureText3D(GetFontDefault(), opt, 6.0f, 0.5f, 0.0f);
//        pos.x = -m.x / 2.0f;
//        DrawText3D(GetFontDefault(), opt, pos, 6.0f, 0.5f, 0.0f, false, DARKBLUE);
//        pos.z += 0.5f + m.z;
//
//        opt = "press [Up]/[Down] to change the font spacing";
//        quads += 44;
//        m = MeasureText3D(GetFontDefault(), opt, 6.0f, 0.5f, 0.0f);
//        pos.x = -m.x / 2.0f;
//        DrawText3D(GetFontDefault(), opt, pos, 6.0f, 0.5f, 0.0f, false, DARKBLUE);
//        pos.z += 0.5f + m.z;
//
//        opt = "press [PgUp]/[PgDown] to change the line spacing";
//        quads += 48;
//        m = MeasureText3D(GetFontDefault(), opt, 6.0f, 0.5f, 0.0f);
//        pos.x = -m.x / 2.0f;
//        DrawText3D(GetFontDefault(), opt, pos, 6.0f, 0.5f, 0.0f, false, DARKBLUE);
//        pos.z += 0.5f + m.z;
//
//        opt = "press [F1] to toggle the letter boundry";
//        quads += 39;
//        m = MeasureText3D(GetFontDefault(), opt, 6.0f, 0.5f, 0.0f);
//        pos.x = -m.x / 2.0f;
//        DrawText3D(GetFontDefault(), opt, pos, 6.0f, 0.5f, 0.0f, false, DARKBLUE);
//        pos.z += 0.5f + m.z;
//
//        opt = "press [F2] to toggle the text boundry";
//        quads += 37;
//        m = MeasureText3D(GetFontDefault(), opt, 6.0f, 0.5f, 0.0f);
//        pos.x = -m.x / 2.0f;
//        DrawText3D(GetFontDefault(), opt, pos, 6.0f, 0.5f, 0.0f, false, DARKBLUE);
//        //-------------------------------------------------------------------------
//
//        SHOW_LETTER_BOUNDRY = slb;
//        EndShaderMode();
//
//        EndMode3D();
//
//        // Draw 2D info text & stats
//        //-------------------------------------------------------------------------
//        DrawText("Drag & drop a font file to change the font!\nType something, see what happens!\n\n"
//            "Press [F3] to toggle the camera", 10, 35, 10, BLACK);
//
//        quads += TextLength(text) * 2 * layers;
//        char* tmp = (char*)TextFormat("%2i layer(s) | %s camera | %4i quads (%4i verts)", layers, spin ? "ORBITAL" : "FREE", quads, quads * 4);
//        int width = MeasureText(tmp, 10);
//        DrawText(tmp, screenWidth - 20 - width, 10, 10, DARKGREEN);
//
//        tmp = "[Home]/[End] to add/remove 3D text layers";
//        width = MeasureText(tmp, 10);
//        DrawText(tmp, screenWidth - 20 - width, 25, 10, DARKGRAY);
//
//        tmp = "[Insert]/[Delete] to increase/decrease distance between layers";
//        width = MeasureText(tmp, 10);
//        DrawText(tmp, screenWidth - 20 - width, 40, 10, DARKGRAY);
//
//        tmp = "click the [CUBE] for a random color";
//        width = MeasureText(tmp, 10);
//        DrawText(tmp, screenWidth - 20 - width, 55, 10, DARKGRAY);
//
//        tmp = "[Tab] to toggle multicolor mode";
//        width = MeasureText(tmp, 10);
//        DrawText(tmp, screenWidth - 20 - width, 70, 10, DARKGRAY);
//        //-------------------------------------------------------------------------
//
//        DrawFPS(10, 10);
//
//        EndDrawing();
//        //----------------------------------------------------------------------------------
//    }
//
//    // De-Initialization
//    //--------------------------------------------------------------------------------------
//    UnloadFont(font);
//    CloseWindow();        // Close window and OpenGL context
//    //--------------------------------------------------------------------------------------
//
//    return 0;
//}
//
//}