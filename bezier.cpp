#include <SDL.h>

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include <vector>
#include <float.h>

#define SCREEN_WIDTH  900
#define SCREEN_HEIGHT 600

#define MAX_POINTS 10
#define CONTROL_POINT_WIDTH 15
#define CONTROL_POINT_HEIGHT 15
#define CONTROL_POINT_RADIUS 10


typedef struct {
    bool is_held;
} Mouse_state;
Mouse_state ms = { .is_held = false };

typedef struct {
    int r;
    int g;
    int b;
    int a;
} Color;
Color point_add_color = { .r = 0xff, .g = 0xff, .b = 0xff, .a = 0xff };
Color point_held_color = { .r = 0x00, .g = 0xff, .b = 0xff, .a = 0xff };
Color point_strip_color = { .r = 0xff, .g = 0x00, .b = 0xff, .a = 0xff };
Color vector_color = { .r = 0xff, .g = 0x00, .b = 0x00, .a = 0xff };

typedef struct {
    float x;
    float y;
    bool is_held;
} Point; // IT IS ACTUALLY RECT

bool is_inside(Point& p, const float in_x, const float in_y) {
    return p.x-CONTROL_POINT_RADIUS <= in_x && in_x <= p.x+CONTROL_POINT_RADIUS
             && p.y-CONTROL_POINT_RADIUS <= in_y && in_y <= p.y+CONTROL_POINT_RADIUS;
}
bool is_hovered(std::vector<Point>& points, const float in_x, const float in_y, int* index) {
    for (int i = 0; i < points.size(); i++) {
        if (is_inside(points[i], in_x, in_y)) {
            *index = i;
            return true;
        }
    }
    *index = -1;
    return false;
}

bool is_mouse_inside(int px, int py) {
    return !(px-CONTROL_POINT_RADIUS < 0 || px+CONTROL_POINT_RADIUS > SCREEN_WIDTH || py-CONTROL_POINT_RADIUS < 0 || py+CONTROL_POINT_RADIUS > SCREEN_HEIGHT);
}

void add_point(SDL_Event& e, std::vector<Point>& points) {
    int px, py;
    SDL_GetMouseState(&px, &py); 
    int i;
    if (points.size() < MAX_POINTS && !is_hovered(points, px, py, &i) && is_mouse_inside(px, py)) {
        Point p = { .x = (float)px, .y = (float)py, .is_held = false };
        points.push_back(p);
    }
}

void update_points(std::vector<Point>& points) {
    for (int i = 0; i < points.size(); i++) {
        if (points[i].is_held) {
            if (ms.is_held) {
                int px, py;
                SDL_GetMouseState(&px, &py); 
                points[i].x = (float)px;
                points[i].y = (float)py;
                if (px-CONTROL_POINT_RADIUS < 0.f) points[i].x = 0.f+CONTROL_POINT_RADIUS;
                if (px+CONTROL_POINT_RADIUS > SCREEN_WIDTH) points[i].x = (float)SCREEN_WIDTH-CONTROL_POINT_RADIUS;
                if (py-CONTROL_POINT_RADIUS < 0.f) points[i].y = 0.f+CONTROL_POINT_RADIUS;
                if (py+CONTROL_POINT_RADIUS > SCREEN_HEIGHT) points[i].y = (float)SCREEN_HEIGHT-CONTROL_POINT_RADIUS;
            }
            else {
                points[i].is_held = false;
            }
        }
    }
}

void draw_control_point(SDL_Renderer* r, const Point& p) { // this is centre
    if (!p.is_held) SDL_SetRenderDrawColor(r, point_add_color.r, point_add_color.g, point_add_color.b, point_add_color.a);
    else SDL_SetRenderDrawColor(r, point_held_color.r, point_held_color.g, point_held_color.b, point_held_color.a);

    for (int y = p.y - CONTROL_POINT_RADIUS; y < p.y + CONTROL_POINT_RADIUS; y++) {
        int dy = y-p.y;
        for (int x = p.x - CONTROL_POINT_RADIUS; x < p.x + CONTROL_POINT_RADIUS; x++) {
            int dx = x-p.x;
            if (dx*dx + dy*dy <= CONTROL_POINT_RADIUS*CONTROL_POINT_RADIUS) {
                SDL_RenderDrawPoint(r, x, y);
            }
        }
    }


//    SDL_FRect target = {p.x, p.y, (float)CONTROL_POINT_WIDTH, (float)CONTROL_POINT_HEIGHT};
//    SDL_RenderFillRectF(r, &target);
}

#define STRIP_STEP 5.F
void draw_control_points(SDL_Renderer* r, std::vector<Point>& points) {
    for (int i = 0; i < points.size(); i++) {
        draw_control_point(r, points[i]);
    }

    SDL_SetRenderDrawColor(r, point_strip_color.r, point_strip_color.g, point_strip_color.b, point_strip_color.a);
    if (!points.empty()) {
        for (int i = 0; i < points.size() - 1; i++) {
            SDL_RenderDrawLineF(r, points[i].x, points[i].y, points[i+1].x, points[i+1].y);

//            float x0 = points[i].x+CONTROL_POINT_WIDTH, y0 = points[i].y+CONTROL_POINT_HEIGHT/2, x = points[i+1].x+CONTROL_POINT_WIDTH/2, y = points[i+1].y+CONTROL_POINT_HEIGHT;
//            float l = sqrtf((x-x0)*(x-x0) + (y-y0)*(y-y0));
//            float vx = (x-x0)/l, vy = (y-y0)/l;
//            float stepx = STRIP_STEP*vx;
//            float stepy = STRIP_STEP*vy;
//            float lv = sqrtf(stepx*stepx + stepy*stepy);
//            float temp = lv;
//            while (lv < l) {
//                lv += temp;
//                x0 += stepx;
//                y0 += stepy;
//
//                if (!(is_inside(points[i], x0, y0) || (is_inside(points[i+1], x0, y0)))) {
//                    SDL_RenderDrawPointF(r, x0, y0);
//                }
//            }
        }
    }
}

Point lerp(float t, Point& p1, Point& p2) {
    float x = (1.f - t)*p1.x + t*p2.x;
    float y = (1.f - t)*p1.y + t*p2.y;
    return { x, y, false };
}

std::vector<Point> t_points;
Point compute_t_point(float t, std::vector<Point>& points) {
    t_points.clear();

    t_points.assign(points.begin(), points.end());
    for (int i = 1; i < points.size(); i++) {
        for (int j = 0; j < points.size() - i; j++) {
            t_points[j] = lerp(t, t_points[j], t_points[j+1]);
        }
    } 

    return t_points[0];
}

#define T_STEP 0.05f
void draw_bezier(SDL_Renderer* r, std::vector<Point>& points) {
    if (points.size() > 1) {

        Point prev_p = points[0];
        bool debug = false;
        for (float t = 0.f; t <= 1.f+FLT_EPSILON; t+=T_STEP) {
            SDL_SetRenderDrawColor(r, 0x33, 0x58, 0xbb, SDL_ALPHA_OPAQUE);
            Point next_p = compute_t_point(t, points);
            
            SDL_RenderDrawLineF(r, prev_p.x, prev_p.y, next_p.x, next_p.y);

            SDL_SetRenderDrawColor(r, 0x00, 0x00, 0xff, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawPointF(r, next_p.x, next_p.y);
            prev_p = next_p;
        }
    }
}

int main(int argc, char* argv[]) {

    SDL_Window* window = NULL;
    SDL_Renderer* render = NULL;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize. SDL error: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Bezier curve", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (window == NULL) {
        printf("SDL could not create a window. SDL error: %s\n", SDL_GetError());
        return 1;
    }

    render = SDL_CreateRenderer(window, -1, 0); 
    if (render == NULL) {
        printf("SDL could not create a renderer. SDL error: %s\n", SDL_GetError());
        return 1;
    }
    

    std::vector<Point> points{};

//    int32_t start_time = SDL_GetTicks();
//    int32_t dt = 0;

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    add_point(e, points);
                }
                else if (e.button.button == SDL_BUTTON_RIGHT) {
                    int px, py, i;
                    SDL_GetMouseState(&px, &py); 
                    if (is_hovered(points, px, py, &i)) {
                        ms.is_held = true;
                        points[i].is_held = true;
                    }
                }
            }
            if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_RIGHT) ms.is_held = false;
            update_points(points);
        }

//        int32_t current_time = SDL_GetTicks();
//        dt = current_time - start_time;

        SDL_SetRenderDrawColor(render, 18, 18, 18, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(render);

        draw_control_points(render, points);
        draw_bezier(render, points);

        SDL_RenderPresent(render);
    }

    SDL_DestroyRenderer(render);
    render = NULL;
    SDL_DestroyWindow(window);
    window = NULL;
    SDL_Quit();
    return 0;
}
