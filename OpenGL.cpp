// OpenGL.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

// This is a compiler directive that includes libraries (For Visual Studio)
// You can manually include the libraries in the "Project->settings" menu under
// the "Link" tab.  You need these libraries to compile this program.
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <gl\gl.h>										// Header File For The OpenGL32 Library
#include <gl\glu.h>										// Header File For The GLu32 Library

#define SCREEN_WIDTH 800								// We want our screen width 800 pixels
#define SCREEN_HEIGHT 600								// We want our screen height 600 pixels
#define SCREEN_DEPTH 16									// We want 16 bits per pixel

bool g_bFullScreen = true; // Set full screen as default
HWND g_hWnd; // This is the handle for the window
RECT g_rRect; // This holds the window dimensions
HDC g_hDC; // General HDC - (handle to device context)
HGLRC g_hRC; // General OpenGL_DC - Our Rendering Context for OpenGL
HINSTANCE g_hInstance; // This holds the global hInstance for UnregisterClass() in DeInit()

// The window proc which handles all of window's messages.
LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// This is our main rendering function prototype.  It's up here for now so MainLoop() can call it.
void RenderScene();

///////////////////////////////// CHANGE TO FULL SCREEN \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This changes the screen to FULL SCREEN
/////
///////////////////////////////// CHANGE TO FULL SCREEN \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void ChangeToFullScreen() {
	DEVMODE dmSettings; // Device Mode variable

	memset(&dmSettings, 0, sizeof (dmSettings)); // Makes Sure Memory's Cleared

	// Get current settings -- This function fills our the settings
	// This makes sure NT and Win98 machines change correctly
	if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmSettings)) {
		// Display error message if we couldn't get display settings
		MessageBox(NULL, "Could Not Enum Display Settings", "Error", MB_OK);
		return;
	}

	dmSettings.dmPelsWidth = SCREEN_WIDTH; // Selected Screen Width
	dmSettings.dmPelsHeight = SCREEN_HEIGHT; // Selected Screen Height

	// This function actually changes the screen to full screen
	// CDS_FULLSCREEN Gets Rid Of Start Bar.
	// We always want to get a result from this function to check if we failed
	int result = ChangeDisplaySettings(&dmSettings, CDS_FULLSCREEN);

	// Check if we didn't recieved a good return message From the function
	if (result != DISP_CHANGE_SUCCESSFUL) {
		// Display the error message and quit the program
		MessageBox(NULL, "Display Mode Not Compatible", "Error", MB_OK);
		PostQuitMessage(0);
	}
}

///////////////////////////////// CREATE MY WINDOW \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function creates a window, but doesn't have a message loop
/////
///////////////////////////////// CREATE MY WINDOW \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

HWND CreateMyWindow(LPSTR strWindowName, int width, int height, DWORD dwStyle, bool bFullScreen, HINSTANCE hInstance) {
	HWND hWnd;
	WNDCLASS wndclass;

	memset(&wndclass, 0, sizeof (WNDCLASS)); // Init the size of the class
	wndclass.style = CS_HREDRAW | CS_VREDRAW; // Regular drawing capabilities
	wndclass.lpfnWndProc = WinProc; // Pass our function pointer as the window procedure
	wndclass.hInstance = hInstance; // Assign our hInstance
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION); // General icon
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW); // An arrow for the cursor
	wndclass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1); // A white window
	wndclass.lpszClassName = "GameTutorials"; // Assign the class name

	RegisterClass(&wndclass); // Register the class

	if (bFullScreen && !dwStyle) // Check if we wanted full screen mode
	{ // Set the window properties for full screen mode
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		ChangeToFullScreen(); // Go to full screen
		ShowCursor(FALSE); // Hide the cursor
	} else if (!dwStyle) // Assign styles to the window depending on the choice
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	g_hInstance = hInstance; // Assign our global hInstance to the window's hInstance

	// Below, we need to adjust the window to it's true requested size.  If we say we
	// want a window that is 800 by 600, that means we want the client rectangle to
	// be that big, not the entire window.  If we go into window mode, it will cut off
	// some of the client rect and stretch the remaining which causes slow down.  We fix this below.

	RECT rWindow;
	rWindow.left = 0; // Set Left Value To 0
	rWindow.right = width; // Set Right Value To Requested Width
	rWindow.top = 0; // Set Top Value To 0
	rWindow.bottom = height; // Set Bottom Value To Requested Height

	AdjustWindowRect(&rWindow, dwStyle, false); // Adjust Window To True Requested Size

	// Create the window
	hWnd = CreateWindow("GameTutorials", strWindowName, dwStyle, 0, 0,
			rWindow.right - rWindow.left, rWindow.bottom - rWindow.top,
			NULL, NULL, hInstance, NULL);

	if (!hWnd) return NULL; // If we could get a handle, return NULL

	ShowWindow(hWnd, SW_SHOWNORMAL); // Show the window
	UpdateWindow(hWnd); // Draw the window

	SetFocus(hWnd); // Sets Keyboard Focus To The Window	

	return hWnd;
}

///////////////////////////////// MAIN GAME LOOP \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function Handles the main game loop
/////
///////////////////////////////// MAIN GAME LOOP \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

WPARAM MainLoop() {
	MSG msg;

	while (1) // Do our infinate loop
	{ // Check if there was a message
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) // If the message wasnt to quit
				break;
			TranslateMessage(&msg); // Find out what the message does
			DispatchMessage(&msg); // Execute the message
		} else // if there wasn't a message
		{
			// Do computationally expensive things here.  We want to render the scene
			// every frame, so we call our rendering function here.  Even though the scene
			// doesn't change, it will bottle neck the message queue if we don't do something.
			// Usually WaitMessage() is used to make sure the app doesn't eat up the CPU.
			RenderScene();
		}
	}

	return (msg.wParam); // Return from the program
}

///////////////////////////////// SET UP PIXEL FORMAT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function sets the pixel format for OpenGL.
/////
///////////////////////////////// SET UP PIXEL FORMAT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool bSetupPixelFormat(HDC hdc) {
	PIXELFORMATDESCRIPTOR pfd = {0};
	int pixelformat;

	pfd.nSize = sizeof (PIXELFORMATDESCRIPTOR); // Set the size of the structure
	pfd.nVersion = 1; // Always set this to 1
	// Pass in the appropriate OpenGL flags
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.dwLayerMask = PFD_MAIN_PLANE; // We want the standard mask (this is ignored anyway)
	pfd.iPixelType = PFD_TYPE_RGBA; // We want RGB and Alpha pixel type
	pfd.cColorBits = SCREEN_DEPTH; // Here we use our #define for the color bits
	pfd.cDepthBits = SCREEN_DEPTH; // Depthbits is ignored for RGBA, but we do it anyway
	pfd.cAccumBits = 0; // No special bitplanes needed
	pfd.cStencilBits = 0; // We desire no stencil bits

	// This gets us a pixel format that best matches the one passed in from the device
	if ((pixelformat = ChoosePixelFormat(hdc, &pfd)) == FALSE) {
		MessageBox(null, "ChoosePixelFormat failed", "Error", MB_OK);
		return false;
	}

	// This sets the pixel format that we extracted from above
	if (SetPixelFormat(hdc, pixelformat, &pfd) == FALSE) {
		MessageBox(null, "SetPixelFormat failed", "Error", MB_OK);
		return false;
	}

	return true; // Return a success!
}

//////////////////////////// RESIZE OPENGL SCREEN \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function resizes the viewport for OpenGL.
/////
//////////////////////////// RESIZE OPENGL SCREEN \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void SizeOpenGLScreen(int width, int height) // Initialize The GL Window
{
	if (height == 0) // Prevent A Divide By Zero error
	{
		height = 1; // Make the Height Equal One
	}

	glViewport(0, 0, width, height); // Make our viewport the whole window
	// We could make the view smaller inside
	// Our window if we wanted too.
	// The glViewport takes (x, y, width, height)
	// This basically means, what our our drawing boundries

	glMatrixMode(GL_PROJECTION); // Select The Projection Matrix
	glLoadIdentity(); // Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	// The parameters are:
	// (view angle, aspect ration of the width to the height, 
	//  The closest distance to the camera before it clips, 
	// FOV		// Ratio				//  The farthest distance before it stops drawing)
	gluPerspective(45.0f, (GLfloat) width / (GLfloat) height, 1, 150.0f);

	// * Note * - The farthest distance should be at least 1 if you don't want some
	// funny artifacts when dealing with lighting and distance polygons.  This is a special
	// thing that not many people know about.  If it's less than 1 it creates little flashes
	// on far away polygons when lighting is enabled.

	glMatrixMode(GL_MODELVIEW); // Select The Modelview Matrix
	glLoadIdentity(); // Reset The Modelview Matrix
}

///////////////////////////////// INITIALIZE GL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function handles all the initialization for OpenGL.
/////
///////////////////////////////// INITIALIZE GL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void InitializeOpenGL(int width, int height) {
	g_hDC = GetDC(g_hWnd); // This sets our global HDC
	// We don't free this hdc until the end of our program
	if (!bSetupPixelFormat(g_hDC)) // This sets our pixel format/information
		PostQuitMessage(0); // If there's an error, quit

	// We need now to create a rendering context AFTER we setup the pixel format.
	// A rendering context is different that a device context (hdc), but that is
	// What openGL uses to draw/render to.  Because openGL can be used on
	// Macs/Linux/Windows/etc.. It has it's on type of rendering context that is
	// The same for EACH operating system, but it piggy backs our HDC information
	g_hRC = wglCreateContext(g_hDC); // This creates a rendering context from our hdc
	wglMakeCurrent(g_hDC, g_hRC); // This makes the rendering context we just created the one we want to use

	SizeOpenGLScreen(width, height); // Setup the screen translations and viewport
}

///////////////////////////////// INIT GAME WINDOW \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function initializes the game window.
/////
///////////////////////////////// INIT GAME WINDOW \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void Init(HWND hWnd) {
	g_hWnd = hWnd; // Assign the window handle to a global window handle
	GetClientRect(g_hWnd, &g_rRect); // Assign the windows rectangle to a global RECT
	InitializeOpenGL(g_rRect.right, g_rRect.bottom); // Init OpenGL with the global rect

	// *Hint* We will put all our game init stuff here
	// Some things include loading models, textures and network initialization
}

///////////////////////////////// RENDER SCENE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function renders the entire scene.
/////
///////////////////////////////// RENDER SCENE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear The Screen And The Depth Buffer
	glLoadIdentity(); // Reset The View

	// 	  Position      View	   Up Vector
	gluLookAt(0, 0, 6, 0, 0, 0, 0, 1, 0); // This determines where the camera's position and view is

	// The position has an X Y and Z.  Right now, we are standing at (0, 0, 6)
	// The view also has an X Y and Z.  We are looking at the center of the axis (0, 0, 0)
	// The up vector is 3D too, so it has an X Y and Z.  We say that up is (0, 1, 0)
	// Unless you are making a game like Descent(TM), the up vector can stay the same.

	// Below we say that we want to draw triangles		
	glBegin(GL_TRIANGLES); // This is our BEGIN to draw
	glVertex3f(0, 1, 0); // Here is the top point of the triangle
	glVertex3f(-1, 0, 0);
	glVertex3f(1, 0, 0); // Here are the left and right points of the triangle
	glEnd(); // This is the END of drawing

	// I arranged the functions like that in code so you could visualize better
	// where they will be on the screen.  Usually they would each be on their own line
	// The code above draws a triangle to those points and fills it in.
	// You can have as many points inside the BEGIN and END, but it must be in three's.
	// Try GL_LINES or GL_QUADS.  Lines are done in 2's and Quads done in 4's.

	SwapBuffers(g_hDC); // Swap the backbuffers to the foreground
}

///////////////////////////////// DE INIT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function cleans up and then posts a quit message to the window
/////
///////////////////////////////// DE INIT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void DeInit() {
	if (g_hRC) {
		wglMakeCurrent(NULL, NULL); // This frees our rendering memory and sets everything back to normal
		wglDeleteContext(g_hRC); // Delete our OpenGL Rendering Context	
	}

	if (g_hDC)
		ReleaseDC(g_hWnd, g_hDC); // Release our HDC from memory

	if (g_bFullScreen) // If we were in full screen
	{
		ChangeDisplaySettings(NULL, 0); // If So Switch Back To The Desktop
		ShowCursor(TRUE); // Show Mouse Pointer
	}

	UnregisterClass("GameTutorials", g_hInstance); // Free the window class

	PostQuitMessage(0); // Post a QUIT message to the window
}


///////////////////////////////// WIN MAIN \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function handles registering and creating the window.
/////
///////////////////////////////// WIN MAIN \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hprev, PSTR cmdline, int ishow) {
	HWND hWnd;

	// Check if we want full screen or not
	if (MessageBox(NULL, "Click Yes to go to full screen (Recommended)", "Options", MB_YESNO | MB_ICONQUESTION) == IDNO)
		g_bFullScreen = FALSE;

	// Create our window with our function we create that passes in the:
	// Name, width, height, any flags for the window, if we want fullscreen of not, and the hInstance
	hWnd = CreateMyWindow("www.GameTutorials.com - First OpenGL Program", SCREEN_WIDTH, SCREEN_HEIGHT, 0, g_bFullScreen, hInstance);

	// If we never got a valid window handle, quit the program
	if (hWnd == NULL) return TRUE;

	// INIT OpenGL
	Init(hWnd);

	// Run our message loop and after it's done, return the result
	return MainLoop();
}

///////////////////////////////// WIN PROC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function handles the window messages.
/////
///////////////////////////////// WIN PROC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LONG lRet = 0;
	PAINTSTRUCT ps;

	switch (uMsg) {
		case WM_SIZE: // If the window is resized
			if (!g_bFullScreen) // Don't worry about this if we are in full screen (otherwise may cause problems)
			{ // LoWord=Width, HiWord=Height
				SizeOpenGLScreen(LOWORD(lParam), HIWORD(lParam));
				GetClientRect(hWnd, &g_rRect); // Get the window rectangle
			}
			break;

		case WM_PAINT: // If we need to repaint the scene
			BeginPaint(hWnd, &ps); // Init the paint struct		
			EndPaint(hWnd, &ps); // EndPaint, Clean up
			break;

		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE) DeInit(); // Quit if we pressed ESCAPE
			break;

		case WM_DESTROY: // If the window is destroyed
			DeInit(); // Release memory and restore settings
			break;

		default: // Return by default
			lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
			break;
	}

	return lRet; // Return by default
}
