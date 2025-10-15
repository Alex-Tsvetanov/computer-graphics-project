#include <iostream>
#include <cmath>
#include <vector>
#include <utility>
#include <set>
#include <stdexcept>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

constexpr int ESCAPE = 27;

struct point {
    float x, y;
    point(): x(0.0f), y(0.0f) {}
    point(float a, float b): x(a), y(b) {}
};

ostream& operator<<(ostream& out, point& a) {
    out << "(" << a.x / 4 << ", " << a.y / 4 << ")";
    return out;
}
istream& operator>>(istream& in, point& a) { in >> a.x >> a.y; return in; }

point operator-(point a, point b) { return point(a.x - b.x, a.y - b.y); }

bool operator==(point a, point b) {
    return (fabs(a.y - b.y) <= 0.01 && fabs(a.x - b.x) <= 0.01);
}
bool operator>(point a, point b) {
    if (a.y == b.y) return a.x > b.x; return (a.y > b.y);
}
bool operator<(point a, point b) {
    if (a.y == b.y) return a.x < b.x; return (a.y < b.y);
}

vector<point> polygon;
const string colinear = "colinear", clockwise = "clock wise", counterclockwise = "counterclock wise";

double CrossProduct(point a, point b) { return a.x * b.y - a.y * b.x; }

string orientation(point A, point B, point C) {
    point AC = C - B, AB = A - B;
    double val = CrossProduct(AB, AC);
    if (fabs(val) <= 0.01) return colinear;
    if (val > 0.01) return clockwise;
    return counterclockwise;
}

bool inAngle (int A, int K) {
    point C = polygon[A],
          D = polygon[(A - 1 + (int)polygon.size()) % (int)polygon.size()],
          B = polygon[(A + 1 + (int)polygon.size()) % (int)polygon.size()],
          E = polygon[K];

    if (orientation(D, C, B) == clockwise) {
        return ((orientation(C, D, E) == counterclockwise) &&
                (orientation(C, E, B) == counterclockwise));
    } else {
        std::swap(B, D);
        return ((orientation(C, D, E) != counterclockwise) ||
                (orientation(C, E, B) != counterclockwise));
    }
}

point* intersection(float x1,float y1,float x2,float y2, float x3, float y3, float x4, float y4) {
    float d = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4);
    if (d == 0) return nullptr;
    float xi = ((x3-x4)*(x1*y2-y1*x2)-(x1-x2)*(x3*y4-y3*x4))/d;
    float yi = ((y3-y4)*(x1*y2-y1*x2)-(y1-y2)*(x3*y4-y3*x4))/d;
    point* p = new point(xi,yi);
    if (xi < min(x1,x2) || xi > max(x1,x2)) return nullptr;
    if (xi < min(x3,x4) || xi > max(x3,x4)) return nullptr;
    return p;
}

bool Intersect(point p1, point q1, point p2, point q2) {
    if (p1 == q2 && q1 == p2) return true;
    if (p1 == p2 && q1 == q2) return true;
    point* p = intersection(p1.x,p1.y,q1.x,q1.y,p2.x,p2.y,q2.x,q2.y);
    if (p == nullptr) return false;
    if (*p == p1 || *p == q1 || *p == p2 || *p == q2) return false;
    return true;
}

vector<pair<int,int>> edges;

bool Intersect(int a, int b, int c, int d) {
    return Intersect(polygon[a], polygon[b], polygon[c], polygon[d]);
}
bool Intersect(int a, int b) {
    return Intersect(edges[a].first, edges[a].second, edges[b].first, edges[b].second);
}
bool Intersect(vector<int> arr) {
    for (int i = 0; i < (int)arr.size(); i++)
        for (int j = i + 1; j < (int)arr.size(); j++)
            if (arr[i] == arr[j]) return true;
            else if (Intersect(arr[i], arr[j])) return true;
    return false;
}

vector< set<int> > ln;

void gen_LN() {
    for (int i = 0; i < (int)polygon.size(); i++)
        for (int j = i + 1; j < (int)polygon.size(); j++) {
            for (int k = 0; k < (int)polygon.size(); k++)
                if (Intersect(polygon[i], polygon[j], polygon[k], polygon[(k + 1) % polygon.size()])) {
                    goto BREAK;
                }
            if (inAngle(i, j)) edges.push_back({i, j});
            BREAK:{}
        }

    ln.resize(edges.size());
    for (int i = 0; i < (int)edges.size(); i++)
        for (int k = i + 1; k < (int)edges.size(); k++) {
            if (!Intersect(polygon[edges[i].first], polygon[edges[i].second],
                           polygon[edges[k].first], polygon[edges[k].second])) {
                ln[i].insert(k);
                ln[k].insert(i);
            }
        }
}

float sleep_sec = 1.0f;

vector< set<int> > all1;
vector< set<int> > all2;
set<int> curr2;
vector<int> curr22;
vector< set<pair<int,int>> > allSimply;

int all1Counter = 0;
int all2Counter = 0;
int allSimplyCounter = 0;

void gen1() {
    if (polygon.size() <= 3) return;

    if (polygon.size() == 4)
        for (int a = 0; a < (int)edges.size(); a++) all1.push_back({a});

    if (polygon.size() == 5)
        for (int a = 0; a < (int)edges.size(); a++)
            for (auto& b : ln[a])
                if (!Intersect({a, b})) all1.push_back({a, b});

    if (polygon.size() == 6)
        for (int a = 0; a < (int)edges.size(); a++)
            for (auto& b : ln[a])
                for (auto& c : ln[b])
                    if (!Intersect({a, b, c})) all1.push_back({a, b, c});

    if (polygon.size() == 7)
        for (int a = 0; a < (int)edges.size(); a++)
            for (auto& b : ln[a])
                for (auto& c : ln[b])
                    for (auto& d : ln[c])
                        if (!Intersect({a, b, c, d})) all1.push_back({a, b, c, d});

    if (polygon.size() == 8)
        for (int a = 0; a < (int)edges.size(); a++)
            for (auto& b : ln[a])
                for (auto& c : ln[b])
                    for (auto& d : ln[c])
                        for (auto& e : ln[d])
                            if (!Intersect({a, b, c, d, e})) all1.push_back({a, b, c, d, e});
}

void gen2 (int i = 0, int last = -1)
{
    if (i == (int)polygon.size()) {
        if (!Intersect (curr22)) all2.push_back (curr2);
        return;
    }

    if (last == -1) {
        for (int start = 0; start < (int)ln.size(); ++start) {
            curr22.push_back(start);
            curr2.insert(start);
            gen2(i + 1, start);
            curr2.erase(start);
            curr22.pop_back();
        }
    } else {
        if (last < 0 || last >= (int)ln.size()) return;

        for (int x : ln[last]) {
            curr22.push_back(x);
            curr2.insert(x);
            gen2(i + 1, x);
            curr2.erase(x);
            curr22.pop_back();
        }
    }
}

void gen_simple() {
    for (int start = 0; start < (int)polygon.size(); start++) {
        set<pair<int,int>> a;
        for (int i = start + 2; i < (int)polygon.size() + start - 1; i++)
            a.insert({start, i % (int)polygon.size()});
        allSimply.push_back(a);
    }
}

enum class Mode { Gen1, Gen2, Simple };
Mode g_mode = Mode::Gen1;

void gen1_printer() {
    if (!all1.empty()) all1Counter = (all1Counter + 1) % (int)all1.size();
}
void gen2_printer() {
    if (!all2.empty()) all2Counter = (all2Counter + 1) % (int)all2.size();
}
void gen_simple_printer() {
    if (!allSimply.empty()) allSimplyCounter = (allSimplyCounter + 1) % (int)allSimply.size();
}

static void glfw_err(int code, const char* desc) {
    std::fprintf(stderr, "GLFW error %d: %s\n", code, desc);
}

static GLuint compile(GLenum type, const char* src) {
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok = 0; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if(!ok){
        char log[4096]; glGetShaderInfoLog(sh, sizeof log, nullptr, log);
        throw std::runtime_error(std::string("shader compile failed: ")+log);
    }
    return sh;
}

static GLuint link(GLuint vs, GLuint fs) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs); glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok = 0; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if(!ok){
        char log[4096]; glGetProgramInfoLog(prog, sizeof log, nullptr, log);
        throw std::runtime_error(std::string("program link failed: ")+log);
    }
    glDetachShader(prog, vs); glDetachShader(prog, fs);
    glDeleteShader(vs); glDeleteShader(fs);
    return prog;
}

static const char* kVert = R"(#version 330 core
layout(location=0) in vec2 inPos;
layout(location=1) in vec3 inColor;
uniform mat4 uMVP;
out vec3 vColor;
void main(){
  vColor = inColor;
  gl_Position = uMVP * vec4(inPos, 0.0, 1.0);
}
)";

static const char* kFrag = R"(#version 330 core
in vec3 vColor;
out vec4 fragColor;
void main(){
  fragColor = vec4(vColor, 1.0);
}
)";

GLuint g_prog = 0;
GLint  g_uMVP = -1;
GLuint g_vao = 0;
GLuint g_vbo = 0;

struct Vtx { float x,y, r,g,b; };

static vector<Vtx> build_frame_vertices() {
    vector<Vtx> v;

    auto push_segment = [&](point a, point b, float r, float g, float bcol){
        v.push_back({a.x, a.y, r,g,bcol});
        v.push_back({b.x, b.y, r,g,bcol});
    };

    if (g_mode == Mode::Gen1 && !all1.empty()) {
        const auto& S = all1[all1Counter];
        for (auto idx : S) {
            auto e = edges[idx];
            push_segment(polygon[e.first], polygon[e.second], 1.0f, 1.0f, 0.0f); // yellow
        }
    } else if (g_mode == Mode::Gen2 && !all2.empty()) {
        const auto& S = all2[all2Counter];
        for (auto idx : S) {
            auto e = edges[idx];
            push_segment(polygon[e.first], polygon[e.second], 1.0f, 1.0f, 0.0f);
        }
    } else if (g_mode == Mode::Simple && !allSimply.empty()) {
        const auto& S = allSimply[allSimplyCounter];
        for (auto pr : S) {
            push_segment(polygon[pr.first], polygon[pr.second], 1.0f, 1.0f, 0.0f);
        }
    }

    for (int i = 0; i < (int)polygon.size(); i++) {
        point a = polygon[i];
        point b = polygon[(i+1)%(int)polygon.size()];
        push_segment(a, b, 0.0f, 1.0f, 0.0f);
    }

    return v;
}

int main() {
    std::ios::sync_with_stdio(false);

    std::cout << "How long each triangulation stays on screen? (seconds): ";
    std::cin >> sleep_sec;
    int n;
    std::cout << "How many vertices does your polygon have?: ";
    std::cin >> n;
    for (int i = 0; i < n; i++) {
        float x,y;
        std::cout << "Input coordinates of point " << char('A'+i) << " (x y): ";
        std::cin >> x >> y;
        polygon.push_back(point(x*4.0f, y*4.0f));
    }

    gen_LN();
    gen1();
    gen2();
    gen_simple();

    {
        std::cout << "Choose mode: 1=gen1, 2=gen2, 3=simple : ";
        int m=1; std::cin >> m;
        g_mode = (m==2? Mode::Gen2 : m==3 ? Mode::Simple : Mode::Gen1);
    }

    glfwSetErrorCallback(glfw_err);
    if(!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow* win = glfwCreateWindow(960, 600, "Triangulation of Polygon (OpenGL 3.3 Core)", nullptr, nullptr);
    if(!win){ glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to init GLAD\n");
        return 1;
    }
    std::printf("GL: %s\nGLSL: %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

    glfwSetWindowUserPointer(win, nullptr);
    glfwSetKeyCallback(win, [](GLFWwindow* w, int key, int sc, int action, int mods){
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) glfwSetWindowShouldClose(w, true);
            if (key == GLFW_KEY_1) g_mode = Mode::Gen1;
            if (key == GLFW_KEY_2) g_mode = Mode::Gen2;
            if (key == GLFW_KEY_3) g_mode = Mode::Simple;
        }
    });

    g_prog = link(compile(GL_VERTEX_SHADER, kVert), compile(GL_FRAGMENT_SHADER, kFrag));
    g_uMVP = glGetUniformLocation(g_prog, "uMVP");

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    glGenBuffers(1, &g_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, 1024 * 1024, nullptr, GL_DYNAMIC_DRAW); // reserve ~1MB

    glEnableVertexAttribArray(0); // pos
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)offsetof(Vtx, x));
    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)offsetof(Vtx, r));

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glLineWidth(2.0f);

    double lastAdvance = glfwGetTime();

    while(!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        int w, h; glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (h==0) ? 1.0f : (float)w/(float)h;
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-5.5f, 0.0f, -30.0f));
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 mvp = proj * view * model;

        double now = glfwGetTime();
        if (now - lastAdvance >= (double)sleep_sec) {
            switch (g_mode) {
                case Mode::Gen1:   gen1_printer(); break;
                case Mode::Gen2:   gen2_printer(); break;
                case Mode::Simple: gen_simple_printer(); break;
            }
            lastAdvance = now;
        }

        vector<Vtx> verts = build_frame_vertices();

        glUseProgram(g_prog);
        glUniformMatrix4fv(g_uMVP, 1, GL_FALSE, glm::value_ptr(mvp));

        glBindVertexArray(g_vao);
        glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
        GLsizeiptr bytes = (GLsizeiptr)(verts.size() * sizeof(Vtx));
        if (bytes > 0) {
            glBufferData(GL_ARRAY_BUFFER, bytes, verts.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_LINES, 0, (GLsizei)verts.size());
        }


        glfwSwapBuffers(win);
    }

    // Cleanup
    glDeleteBuffers(1, &g_vbo);
    glDeleteVertexArrays(1, &g_vao);
    glDeleteProgram(g_prog);
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
