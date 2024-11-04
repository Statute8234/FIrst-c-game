#include <windows.h>
#include "utils.cpp"

static bool needs_render = true;

struct Render_State {
    int height, width;
    void* memory;
    BITMAPINFO bitmap_info;
};
global_variable Render_State render_state;

#include "renderer.cpp"

// check if a point is within a component
bool is_point_in_component(int x, int y, const Component& component) {
    int halfWidth = component.width / 2;
    int halfHeight = component.height / 2;
    return x >= component.x - halfWidth && x <= component.x + halfHeight && y >= component.y && y <= component.y + halfHeight;
}
    
LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    switch (uMsg) {
    case WM_CLOSE:
    case WM_DESTROY: {
        running = false;
    } break;
    case WM_SIZE: {
        RECT rect;
        GetClientRect(hwnd, &rect);
        render_state.width = rect.right - rect.left;
        render_state.height = rect.bottom - rect.top;
        int buffer_size = render_state.width * render_state.height * sizeof(unsigned int);
        if (render_state.memory) VirtualFree(render_state.memory, 0, MEM_RELEASE);
        render_state.memory = VirtualAlloc(0, buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        render_state.bitmap_info.bmiHeader.biSize = sizeof(render_state.bitmap_info.bmiHeader);
        render_state.bitmap_info.bmiHeader.biWidth = render_state.width;
        render_state.bitmap_info.bmiHeader.biHeight = render_state.height;
        render_state.bitmap_info.bmiHeader.biPlanes = 1;
        render_state.bitmap_info.bmiHeader.biBitCount = 32;
        render_state.bitmap_info.bmiHeader.biCompression = BI_RGB;

        needs_render = true;
    } break;

    // test mouses
    case WM_LBUTTONDOWN: {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);

        for (auto& component : components) {
            if (is_point_in_component(mouseX, mouseY, component)) {
                dragging = true;
                selecetedComponent = &component;
                dragOffsetX = mouseX - component.x;
                dragOffsetY = mouseY - component.y;
                break;
            }
        }
    } break;

    case WM_LBUTTONUP: {
        dragging = false;
        selecetedComponent = nullptr;
    } break;
    
    case WM_MOUSEMOVE: {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);
        if (dragging && selecetedComponent) {
            selecetedComponent->x = mouseX - dragOffsetX; //?
            selecetedComponent->y = mouseY - dragOffsetY; //?
            needs_render = true;
        }

        for (auto& wire : wires) {
            selecetedWire = &wire;
            selecetedWire->x = mouseX;
            selecetedWire->width = mouseY;
            needs_render = true;
        }
    }

    case WM_PAINT: {
        if (needs_render) {
            render_background(backBufferDC, 0x00000);
            render_components(backBufferDC);

            BitBlt(GetDC(hwnd), 0, 0, render_state.width, render_state.height, backBufferDC, 0, 0, SRCCOPY);
            ValidateRect(hwnd, NULL);  // Prevents constant redraws, stopping flickering
            needs_render = false;
        }
    } break;

    default: {
        result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    }
    return result;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    ShowCursor(TRUE);
    WNDCLASS window_class = {};
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpszClassName = L"Game Window Class";
    window_class.lpfnWndProc = window_callback;
    RegisterClass(&window_class);

    HWND window = CreateWindow(window_class.lpszClassName, L"Game", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, 0, 0, hInstance, 0);
    HDC hdc = GetDC(window);
    create_back_buffer(1280, 720);

    setup_level();

    MSG message;
    while (running) {
        if (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

    SelectObject(backBufferDC, oldBitmap);
    DeleteObject(backBufferBitmap);
    DeleteDC(backBufferDC);

    ShowCursor(TRUE);
    return 0;
}