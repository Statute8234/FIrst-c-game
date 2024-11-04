#include <cmath>
#include <string>
#include <vector>

// Define wire element
struct Wire {
    int x, y;
    int width, height;
    u32 color;
};

// Define different electronic components
struct Component {
    int x, y;
    int width, height;
    std::string label;
    u32 color;
};

global_variable std::vector<Wire> wires;
global_variable std::vector<Component> components;
global_variable float render_scale = 0.01f;
global_variable bool running = true;
global_variable bool dragging = false;
global_variable int dragOffsetX = 0;
global_variable int dragOffsetY = 0;
global_variable Component* selecetedComponent = nullptr;
global_variable Wire* selecetedWire = nullptr;

HDC backBufferDC;
HBITMAP backBufferBitmap;
HBITMAP oldBitmap;
void* backBufferMemory;

internal
void create_back_buffer(int width, int height) {
    HDC hdc = GetDC(GetConsoleWindow());
    backBufferDC = CreateCompatibleDC(hdc);
    backBufferBitmap = CreateCompatibleBitmap(hdc, width, height);
    oldBitmap = (HBITMAP)SelectObject(backBufferDC, backBufferBitmap);
    backBufferMemory = backBufferDC;
    ReleaseDC(GetConsoleWindow(), hdc);
}

internal
void render_background(HDC hdc, u32 color) {
    HBRUSH brush = CreateSolidBrush(color);
    RECT rect;
    GetClientRect(GetConsoleWindow(), &rect);
    FillRect(backBufferDC, &rect, brush);
    DeleteObject(brush);
}

internal void
draw_rect_in_pixels(int x1, int y1, int x2, int y2, u32 color) {
    x1 = clamp(0, x1, render_state.width);
    x2 = clamp(0, x2, render_state.width);
    y1 = clamp(0, y1, render_state.height);
    y2 = clamp(0, y2, render_state.height);

    HBRUSH brush = CreateSolidBrush(color);
    RECT rect = { x1, y1, x2, y2 };
    FillRect(backBufferDC, &rect, brush);
    DeleteObject(brush);
}

// Utility for resistors, batteries
internal
void draw_component(Component component) {
    int x1 = component.x - component.width / 2;
    int x2 = component.x + component.width / 2;
    int y1 = component.y - component.height / 2;
    int y2 = component.y + component.height / 2;

    draw_rect_in_pixels(x1, y1, x2, y2, component.color);
}

internal
void draw_line(int x1, int y1, int x2, int y2, u32 color) {
    x1 = clamp(0, x1, render_state.width);
    x2 = clamp(0, x2, render_state.width);
    y1 = clamp(0, y1, render_state.height);
    y2 = clamp(0, y2, render_state.height);

    HBRUSH brush = CreateSolidBrush(color);
    RECT rect = { x1, y1, x2, y2 };
    FillRect(backBufferDC, &rect, brush);
    DeleteObject(brush);
}

internal
void draw_wire(Wire wire) {
    int x1 = wire.x - wire.width / 2;
    int x2 = wire.x + wire.width / 2;
    int y1 = wire.y - wire.height / 2;
    int y2 = wire.y + wire.height / 2;

    draw_rect_in_pixels(x1, y1, x2, y2, wire.color);
}

// Create levels
internal
void setup_level() {
    components.clear();
    Component battery = { 10, 10, 5, 5, "Battery", 1015149};
    Wire wire_2v = { 100, 100, 100, 5, 16776447 };
    components.push_back(battery);
    wires.push_back(wire_2v);
}

// Draw level
internal
void render_components(HDC hdc) {
    for (const auto& component : components) {
        draw_component(component);
    }

    for (const auto& wire : wires) {
        draw_wire(wire);
    }

    // Display namess
    for (const auto& component : components) {
        RECT rect;
        rect.left = component.x;
        rect.top = component.y + component.height / 2 + 5;
        rect.right = component.x + 100;
        SetTextColor(hdc, component.color);

        int len = MultiByteToWideChar(CP_ACP, 0, component.label.c_str(), -1, NULL, 0);
        std::wstring wlabel(len, L'\0');
        MultiByteToWideChar(CP_ACP, 0, component.label.c_str(), -1, &wlabel[0], len);
        DrawText(hdc, wlabel.c_str(), -1, &rect, DT_SINGLELINE | DT_NOCLIP);
    }
}