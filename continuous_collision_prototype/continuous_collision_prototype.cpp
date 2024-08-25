// continuous_collision_prototype.cpp : Defines the entry point for the application.
//


#include <memory>
#include <cstdint>
#include <cstdlib> // For rand() and RAND_MAX

#include "framework.h"

#include "DebugWindowsDrawHelper/debug_draw_interface.h"
#include "misc_utilities/delata_time_util.h"

#include "continuous_collision_prototype.h"
//#include "array_utilities/WideNodeLinkedList/UnitTests/wide_node_linked_list_unit_tests.h"
//#include "array_utilities/UnitTests/unit_test_manager.h"
//#include "continuous_collision_library/UnitTests/OverlapTrackingUnitTests/OverlapTrackingUnitTest.h"


#include "continuous_collision_library/2d_physics_main.h"

#include "continuous_collision_library/UnitTests/PhysicsMain/phyisics_2d_main_unit_test.h"

#define MAX_LOADSTRING 100

const char WINDOW_WIDTH = 400;
const char WINDOW_HEIGHT = 400;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

debug_draw_interface draw_interface;
delata_time_util delta_time_tracker;

using physics_main_type = ContinuousCollisionLibrary::phyisics_2d_main<std::numeric_limits<ContinuousCollisionLibrary::uint16>::max() - 1, 32>;
//using physics_main_type = phyisics_2d_main<254, 16>;

std::unique_ptr<physics_main_type> physics_main = std::make_unique<physics_main_type>();

enum class KEY_PRESS : uint8_t
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    ZOOM_IN,
    ZOOM_OUT,
    KEY_COUNT
};

std::array<bool, static_cast<uint32_t>(KEY_PRESS::KEY_COUNT)> key_down_array{};







// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    //ArrayUtilities::wide_node_linked_lists_unit_test::run_wide_node_linked_list_test();
    
    //unit test paged mem manager
    //ArrayUtilities::unit_test_manager::run_paged_memory_header_test();
    
    //unit test the virtual memory map
    //ArrayUtilities::unit_test_manager::run_virtual_memory_header_test();

    //unit test the paged 2d array
    //ArrayUtilities::unit_test_manager::run_paged_2d_array_test();

    //unit test paged hirachical linked list
    //ArrayUtilities::unit_test_manager::run_paged_wide_node_linked_list_unit_test();


    //unit test struct of arrays helper data structure 
    //ArrayUtilities::unit_test_manager::run_struct_of_arrays_test();

    //test metaprograming utility for converting tuples to other types
    //ArrayUtilities::unit_test_manager::run_tuple_converter_test();

    //test the tight packed array system
   //ArrayUtilities::unit_test_manager::run_tight_packed_paged_2d_array_test();

    //test the handle tracked packed data 
    //ArrayUtilities::unit_test_manager::run_handle_mapped_array_test();

    //overlap tracking test
    //ContinuousCollisionLibrary::overlap_tracking_unit_test::run_test();

    //test the physics system
    ContinuousCollisionLibrary::phyisics_2d_main_unit_test::run_test();


    //setup the physics library 
    //setup_physics_main();

    physics_main->setup_physics_random(65000, 200);


    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CONTINUOUSCOLLISIONPROTOTYPE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CONTINUOUSCOLLISIONPROTOTYPE));

    MSG msg;

   

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

  


    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CONTINUOUSCOLLISIONPROTOTYPE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CONTINUOUSCOLLISIONPROTOTYPE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, WINDOW_WIDTH, CW_USEDEFAULT, WINDOW_HEIGHT, nullptr, nullptr, hInstance, nullptr);

  //draw_interface.resize(WINDOW_WIDTH, WINDOW_HEIGHT);
  
  draw_interface.add_offset(math_2d_util::fvec2d( 255 / 2, 255 / 2));
  draw_interface.add_zoom(3.5);
  
  //draw_interface.clear_to(RGB(255, 255, 255));

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // Set up a timer to fire every 60th of a second (16 milliseconds)
   SetTimer(hWnd, 1, 100, NULL);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_ERASEBKGND:
    {
        return TRUE;
    }
    case WM_PAINT:
        {
        float dt = delta_time_tracker.update_delta_time();

        float view_pan_speed = 80.0f;

        float zoom_change_speed = 0.5f;

        //scale up pan speed with more zoom out
        view_pan_speed *= (1.0f / draw_interface.get_zoom());


        float zoom_change = 1.0f - (key_down_array[static_cast<uint32_t>(KEY_PRESS::ZOOM_OUT)] - key_down_array[static_cast<uint32_t>(KEY_PRESS::ZOOM_IN)]) * dt * zoom_change_speed;

        draw_interface.add_zoom(zoom_change);


        //handle any key pressses
        math_2d_util::fvec2d view_offset{};
        
        view_offset.y += (key_down_array[static_cast<uint32_t>(KEY_PRESS::UP)] - key_down_array[static_cast<uint32_t>(KEY_PRESS::DOWN)]) * dt * view_pan_speed;
        view_offset.x += (key_down_array[static_cast<uint32_t>(KEY_PRESS::LEFT)] - key_down_array[static_cast<uint32_t>(KEY_PRESS::RIGHT)]) * dt * view_pan_speed;

        draw_interface.add_offset(view_offset);

        //update the physics 
        physics_main->update_physics();


        draw_interface.clear_to(RGB(255, 255, 255));

            //PAINTSTRUCT ps;
            //HDC hdc = BeginPaint(hWnd, &ps);
            //
            //// Draw a rectangle (box)
            //RECT rect = { 100, 100, 300, 200 };
            //HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 255)); // Blue color
            //FillRect(hdc, &rect, hBrush);
            //DeleteObject(hBrush);
            //
            //
            //
            //EndPaint(hWnd, &ps);



       // draw_interface.draw_screen_space_box(math_2d_util::ivec2d(0, 0), math_2d_util::ivec2d(draw_interface.get_width() / 2, draw_interface.get_height() / 2), RGB(0, 255, 0));
       //
       // draw_interface.draw_screen_space_circle(math_2d_util::ivec2d(draw_interface.get_width() / 2, draw_interface.get_height() / 2), draw_interface.get_height() / 4, RGB(255, 255, 0));
       //
       // draw_interface.draw_sceen_space_circle_outline(math_2d_util::ivec2d(draw_interface.get_width() / 2, draw_interface.get_height() / 2), draw_interface.get_height() / 3, RGB(255, 255, 0));
       //
       // draw_interface.draw_screen_space_line(math_2d_util::ivec2d(0, 0), math_2d_util::ivec2d(draw_interface.get_width(), draw_interface.get_height()), RGB(0, 0, 255));
       //
       // draw_interface.draw_dotted_screen_space_line(math_2d_util::ivec2d(0, draw_interface.get_height()), math_2d_util::ivec2d(draw_interface.get_width(),0), RGB(255, 0, 0));
       //
       // draw_interface.draw_circle(math_2d_util::fvec2d(draw_interface.get_width() / 2, draw_interface.get_height() / 2), draw_interface.get_height() / 4, RGB(100, 100, 100));
      

        //debug draw all physics main
        physics_main->draw_debug(draw_interface);


            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            BITMAPINFO bmi;
            ZeroMemory(&bmi, sizeof(BITMAPINFO));
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = draw_interface.get_width();
            bmi.bmiHeader.biHeight = -static_cast<int32_t>(draw_interface.get_height()); // Negative height to ensure top-down DIB
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            SetDIBitsToDevice(
                hdc,
                0, 0, draw_interface.get_width(), draw_interface.get_height(),
                0, 0, 0, draw_interface.get_height(),
                draw_interface.get_data_ptr(),
                &bmi,
                DIB_RGB_COLORS
            );

            EndPaint(hWnd, &ps);
            ValidateRect(hWnd, NULL);  // or ValidateRgn(hwnd, NULL);
            
        }
        break;
    case WM_DESTROY:
    {
        PostQuitMessage(0);
    }
        break;
    
    case WM_SIZE:
    {
        // Handle window resizing here
        // The new width and height are in the LOWORD and HIWORD of lParam
        auto window_width = LOWORD(lParam);
        auto window_height = HIWORD(lParam);

        //resize the window structure
        draw_interface.resize(window_width, window_height);
    }
        break;

    case WM_KEYDOWN:
    {
        // Handle key down event
        // wParam contains the virtual key code
        switch (wParam)
        {
        case VK_LEFT:
            // Handle left arrow key
            key_down_array[static_cast<uint32_t>(KEY_PRESS::LEFT)] = true;
            break;
        case VK_RIGHT:
            // Handle right arrow key
            key_down_array[static_cast<uint32_t>(KEY_PRESS::RIGHT)] = true;
            break;
        case VK_UP:
            // Handle right arrow key
            key_down_array[static_cast<uint32_t>(KEY_PRESS::UP)] = true;
            break;

        case VK_DOWN:
            key_down_array[static_cast<uint32_t>(KEY_PRESS::DOWN)] = true;
            break;

        case VK_ADD:
            // Handle right arrow key
            key_down_array[static_cast<uint32_t>(KEY_PRESS::ZOOM_IN)] = true;
            break;

        case VK_SUBTRACT:
            // Handle right arrow key
            key_down_array[static_cast<uint32_t>(KEY_PRESS::ZOOM_OUT)] = true;
            break;
            // Add more cases for other keys as needed
        }

        break;
    }

    case WM_KEYUP:
    {
        // Handle key down event
        // wParam contains the virtual key code
        switch (wParam)
        {
        case VK_LEFT:
            // Handle left arrow key
            key_down_array[static_cast<uint32_t>(KEY_PRESS::LEFT)] = false;
            break;
        case VK_RIGHT:
            // Handle right arrow key
            key_down_array[static_cast<uint32_t>(KEY_PRESS::RIGHT)] = false;
            break;
        case VK_UP:
            // Handle right arrow key
            key_down_array[static_cast<uint32_t>(KEY_PRESS::UP)] = false;
            break;

        case VK_DOWN:
            key_down_array[static_cast<uint32_t>(KEY_PRESS::DOWN)] = false;
            break;

        case VK_ADD:
            // Handle right arrow key
            key_down_array[static_cast<uint32_t>(KEY_PRESS::ZOOM_IN)] = false;
            break;

        case VK_SUBTRACT:
            // Handle right arrow key
            key_down_array[static_cast<uint32_t>(KEY_PRESS::ZOOM_OUT)] = false;
            break;
            // Add more cases for other keys as needed
        }

        break;
    }

    case WM_TIMER:
    {
        // Handle timer events
        InvalidateRect(hWnd, NULL, TRUE);

        break;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
