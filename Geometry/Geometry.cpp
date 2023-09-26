// Geometry.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Geometry.h"
#include "Simulator.h"
#include <vector>
#include <sstream>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

std::vector<GameState> gameStates;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GEOMETRY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GEOMETRY));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GEOMETRY));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GEOMETRY);
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
   std::ifstream inFile("input.txt");
   GameState gameState = readState(inFile);
   gameStates.push_back(gameState);
   while (gameState.balls.size()) {
       gameState.simulateStep();
       gameStates.push_back(gameState);
   }

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 500, 700, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

const double SCALING_FACTOR = 15;
const int BALL_RADIUS = 8;
const double RAY_LENGTH = 2;
const int TEXTBOX_SIZE = 50;

v2d TransformPoint(v2d pt) {
    return v2d{ 50+pt.x * SCALING_FACTOR, (500-pt.y * SCALING_FACTOR) };
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
    const int SCROLL_HEIGHT = 50;
    const int STATE_INDEX_LEFT_MARGIN = 10;
    const int STATE_INDEX_HEIGHT = 40;
    static HWND hwndScroll;
    static int stateIndex = 0;
    int maxScroll = (int)gameStates.size() - 1;
    int iTemp;
    switch (message)
    {
    case WM_CREATE:
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        hwndScroll = CreateWindow(TEXT("scrollbar"), NULL,
            WS_CHILD | WS_VISIBLE | SBS_HORZ,
            0, clientRect.bottom - SCROLL_HEIGHT, clientRect.right, SCROLL_HEIGHT, hWnd, (HMENU)1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        SetScrollRange(hwndScroll, SB_CTL, 0, maxScroll, FALSE);
        SetScrollPos(hwndScroll, SB_CTL, stateIndex, TRUE);
        break;
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
    case WM_HSCROLL:
        switch (LOWORD(wParam)) {
            case SB_LINELEFT:
                iTemp = stateIndex - 1;
                break;
            case SB_LINERIGHT:
                iTemp = stateIndex + 1;
                break;
            case SB_PAGELEFT:
                iTemp = stateIndex - 5;
                break;
            case SB_PAGERIGHT:
                iTemp = stateIndex + 5;
                break;
            case SB_THUMBTRACK:
            case SB_THUMBPOSITION:
                iTemp = HIWORD(wParam);
                break;
            default:
                iTemp = stateIndex;
        }

        iTemp = max(0, min(iTemp, maxScroll));
        if (iTemp != stateIndex) {
            stateIndex = iTemp;
            SetScrollPos(hwndScroll, SB_CTL, stateIndex, TRUE);
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_LBUTTONDOWN:  // Handle left mouse button down message
        break;

    case WM_PAINT:
        {
            GameState gameState = gameStates[stateIndex];
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);

            std::stringstream ss_stateIndx;
            ss_stateIndx << "State index: " << stateIndex;
            std::string label = ss_stateIndx.str();
            std::wstring wstr(label.begin(), label.end());
            const TCHAR* text = &wstr[0];
            RECT rect = { STATE_INDEX_LEFT_MARGIN, clientRect.bottom - SCROLL_HEIGHT - STATE_INDEX_HEIGHT, clientRect.right, clientRect.bottom - SCROLL_HEIGHT };
            // Draw the text
            DrawText(
                hdc,        // Handle to the device context
                text,       // Text string
                -1,         // Length of text string, -1 for null-terminated
                &rect,      // Bounding rectangle
                DT_SINGLELINE | DT_LEFT | DT_VCENTER  // Formatting options
            );

            // Set the starting point for the line
            std::vector<v2d> centers(gameState.amount_objects);
            std::vector<int> pcount(gameState.amount_objects);
            for (const Segment &segment : gameState.segments) {
                v2d p1 = TransformPoint(segment.p1);
                v2d p2 = TransformPoint(segment.p2);
                MoveToEx(hdc, (int)p1.x, (int)p1.y, nullptr);
                LineTo(hdc, (int)p2.x, (int)p2.y);
                if (segment.object_id < centers.size()) {
                    centers[segment.object_id] = centers[segment.object_id] + segment.p1;
                    ++pcount[segment.object_id];
                }
            }

            for (int i = 0; i < gameState.amount_objects;++i) {
                v2d coord = centers[i] * (1.0 / pcount[i]);
                coord = TransformPoint(coord);
                RECT rect = { (int)coord.x - TEXTBOX_SIZE/2, (int)coord.y - TEXTBOX_SIZE/2, (int)coord.x + TEXTBOX_SIZE/2, (int)coord.y + TEXTBOX_SIZE/2 };  // Define a rectangle in which to draw the text
                std::stringstream ss;
                ss << gameState.lives[i];
                std::string label = ss.str();
                std::wstring wstr(label.begin(), label.end());
                const TCHAR* text = &wstr[0];

                // Draw the text
                DrawText(
                    hdc,        // Handle to the device context
                    text,       // Text string
                    -1,         // Length of text string, -1 for null-terminated
                    &rect,      // Bounding rectangle
                    DT_SINGLELINE | DT_CENTER | DT_VCENTER  // Formatting options
                );
            }

            HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));  // Red fill
            HPEN hPen = CreatePen(PS_NULL, 5, RGB(0, 0, 255));  // no border

            // Select the brush and pen into the device context
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

            for (const Ball& ball : gameState.balls) {
                v2d pt = TransformPoint(ball.ori);
                Ellipse(hdc, (int)pt.x-BALL_RADIUS, (int)pt.y-BALL_RADIUS, (int)pt.x + BALL_RADIUS, (int)pt.y + BALL_RADIUS);
            }

            // Restore the original brush and pen
            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen);

            for (const Ball& ball : gameState.balls){
                v2d p1 = ball.ori;
                v2d p2 = p1 + RAY_LENGTH*ball.dir;
                
                p1 = TransformPoint(p1);
                p2 = TransformPoint(p2);
                MoveToEx(hdc, (int)p1.x, (int)p1.y, nullptr);
                LineTo(hdc, (int)p2.x, (int)p2.y);
            }

            DeleteObject(hBrush); // Don't forget to delete the brush
            DeleteObject(hPen);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
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
