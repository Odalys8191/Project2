#include <graphics.h>      // EasyX图形库
#include <conio.h>         // 键盘输入
#include <vector>          // 动态数组
#include <ctime>           // 随机种子

// 游戏常量
const int TILE_SIZE = 32;  // 格子像素大小
const int MAP_W = 20;      // 地图宽格子数
const int MAP_H = 15;      // 地图高格子数

// 地图格子类型
enum TileType {
    FLOOR,   // 地板
    WALL,    // 墙
    DOOR     // 门
};

// 角色类
class Character {
public:
    int x, y;          // 地图坐标
    int hp;            // 生命值
    int attack;        // 攻击力
    COLORREF color;    // 显示颜色

    Character(int x, int y, COLORREF c) : x(x), y(y), hp(10), attack(2), color(c) {}

    // 绘制角色
    void draw() const {
        setfillcolor(color);
        solidcircle(x * TILE_SIZE + TILE_SIZE / 2, y * TILE_SIZE + TILE_SIZE / 2, TILE_SIZE / 3);
    }
};

// 游戏状态
struct Game {
    TileType map[MAP_W][MAP_H];  // 二维地图数组
    Character player{ 1, 1, RED }; // 玩家角色
    std::vector<Character> enemies; // 敌人列表
    bool isRunning = true;       // 游戏运行标志
};

// 生成随机地图（简单示例）
void generateMap(Game& game) {
    srand(time(0)); // 随机种子

    for (int x = 0; x < MAP_W; x++) {
        for (int y = 0; y < MAP_H; y++) {
            // 边界为墙，内部随机生成
            if (x == 0 || y == 0 || x == MAP_W - 1 || y == MAP_H - 1) {
                game.map[x][y] = WALL;
            }
            else {
                game.map[x][y] = (rand() % 10 > 7) ? WALL : FLOOR;
            }
        }
    }
}

// 绘制地图
void drawMap(const Game& game) {
    for (int x = 0; x < MAP_W; x++) {
        for (int y = 0; y < MAP_H; y++) {
            // 根据格子类型设置颜色
            COLORREF color = DARKGRAY;
            switch (game.map[x][y]) {
            case WALL:  color = BROWN; break;
            case FLOOR: color = RGB(50, 50, 50); break;
            case DOOR:  color = YELLOW; break;
            }

            // 绘制格子
            setfillcolor(color);
            solidrectangle(x * TILE_SIZE, y * TILE_SIZE,
                (x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE);
        }
    }
}

// 处理玩家输入
void processInput(Game& game) {
    if (!_kbhit()) return; // 无按键时直接返回

    int dx = 0, dy = 0;
    switch (_getch()) {
    case 'W': case 'w': dy = -1; break;
    case 'S': case 's': dy = 1; break;
    case 'A': case 'a': dx = -1; break;
    case 'D': case 'd': dx = 1; break;
    case 27: game.isRunning = false; break; // ESC退出
    }

    // 计算新位置
    int newX = game.player.x + dx;
    int newY = game.player.y + dy;

    // 碰撞检测
    if (newX >= 0 && newX < MAP_W &&
        newY >= 0 && newY < MAP_H &&
        game.map[newX][newY] != WALL) {
        game.player.x = newX;
        game.player.y = newY;
    }
}

// 主函数
int main() {
    initgraph(MAP_W * TILE_SIZE, MAP_H * TILE_SIZE); // 创建窗口
    Game game;
    generateMap(game);

    // 生成测试敌人
    game.enemies.emplace_back(5, 5, GREEN);
    game.enemies.emplace_back(8, 3, BLUE);

    // 游戏主循环
    while (game.isRunning) {
        BeginBatchDraw(); // 开始批量绘制（防闪烁）

        cleardevice();    // 清空画面
        drawMap(game);    // 绘制地图
        game.player.draw(); // 绘制玩家

        // 绘制所有敌人
        for (const auto& enemy : game.enemies) {
            enemy.draw();
        }

        processInput(game); // 处理输入
        EndBatchDraw();   // 结束批量绘制
        Sleep(50);        // 控制循环速度
    }

    closegraph(); // 关闭图形窗口
    return 0;
}