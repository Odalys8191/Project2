#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <vector>

// 游戏状态
enum GameState { PLAYING, GAME_OVER, LEVEL_UP };
const int mapWidth = 800 / 25;  // 25
const int mapHeight = 600 / 25; // 18
int mapData[mapHeight][mapWidth] = { 0 }; // 0表示空地，1表示砖块
// 玩家结构体
struct Player {
    int x, y;
    int hp;
    int maxHp;
    int attack;
    int defense;
    int level;
    int exp;
} player = { 400, 300, 100, 100, 10, 5, 1, 0 };

// 敌人结构体
struct Enemy {
    int x, y;
    int hp;
    int attack;
    bool active;
};

// 子弹结构体
struct Bullet {
    int x, y;
    int dir; // 0-7表示8个方向
    bool active;
};

std::vector<Enemy> enemies;
std::vector<Bullet> bullets;
GameState gameState = PLAYING;
const int tileSize = 32;

// 初始化游戏
void GameInit() {
    srand(time(0)); 

    // 生成固定地图（只在游戏开始时生成一次）
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            // 边缘生成墙壁
            if (x == 0 || y == 0 || x == mapWidth - 1 || y == mapHeight - 1) {
                mapData[y][x] = 1;
            }
            else {
                // 30%概率生成砖块
                mapData[y][x] = (rand() % 100 < 30) ? 1 : 0;
            }
        }
    }
    // 生成初始敌人
    for (int i = 0; i < 5; i++) {
        enemies.push_back({
            rand() % 800,
            rand() % 600,
            50,
            5,
            true
            });
    }
}

// 绘制地图
void DrawMap() {
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            if (mapData[y][x] == 1) {
                setfillcolor(RGB(150, 150, 150)); // 灰色砖块
                fillrectangle(x * tileSize, y * tileSize,
                    (x + 1) * tileSize, (y + 1) * tileSize);
            }
        }
    }
}

// 玩家控制
void PlayerControl() {
    int newX = player.x;
    int newY = player.y;

    // 获取输入
    if (GetAsyncKeyState('W') & 0x8000) newY -= 3;
    if (GetAsyncKeyState('S') & 0x8000) newY += 3;
    if (GetAsyncKeyState('A') & 0x8000) newX -= 3;
    if (GetAsyncKeyState('D') & 0x8000) newX += 3;

    // 转换为地图格子坐标
    int tileX = newX / tileSize;
    int tileY = newY / tileSize;

    // 碰撞检测（检测周围3x3区域）
    bool canMove = true;
    RECT rcIntersect; // 增加这个变量用于接收交集区域

    for (int y = tileY - 1; y <= tileY + 1; y++) {
        for (int x = tileX - 1; x <= tileX + 1; x++) {
            if (x >= 0 && y >= 0 && x < mapWidth && y < mapHeight) {
                if (mapData[y][x] == 1) {
                    // 正确创建墙体的RECT（left, top, right, bottom）
                    RECT wallRect = {
                        x * tileSize,
                        y * tileSize,
                        x * tileSize + tileSize,
                        y * tileSize + tileSize
                    };

                    // 正确创建玩家RECT（注意使用中心点坐标转换）
                    RECT playerRect = {
                        newX - 10,  // left
                        newY - 10,  // top
                        newX + 10,  // right
                        newY + 10   // bottom
                    };

                    // 正确的函数调用（三个参数）
                    if (IntersectRect(&rcIntersect, &playerRect, &wallRect)) {
                        canMove = false;
                        break;
                    }
                }
            }
        }
        if (!canMove) break;
    }
    // 更新位置
    if (canMove) {
        player.x = newX;
        player.y = newY;
    }


}

// 升级选择界面
void LevelUpMenu() {
    settextcolor(WHITE);
    settextstyle(24, 0, _T("宋体"));
    outtextxy(300, 200, _T("选择升级项："));
    outtextxy(300, 250, _T("1. 攻击力+5"));
    outtextxy(300, 300, _T("2. 生命值+50"));
    outtextxy(300, 350, _T("3. 防御力+3"));
}

// 游戏主循环
void GameLoop() {
    BeginBatchDraw();

    cleardevice();
    DrawMap();

    // 绘制玩家
    setfillcolor(BLUE);
    fillcircle(player.x, player.y, 10);

    // 更新敌人
    for (auto& e : enemies) {
        if (!e.active) continue;

        // 敌人AI
        int dx = player.x - e.x;
        int dy = player.y - e.y;
        e.x += dx / 10;
        e.y += dy / 10;

        // 绘制敌人
        setfillcolor(RED);
        fillcircle(e.x, e.y, 8);

        // 碰撞检测
        if (abs(player.x - e.x) < 20 && abs(player.y - e.y) < 20) {
            player.hp -= e.attack - player.defense;
        }
    }

    // 更新子弹
    for (auto& b : bullets) {
        if (!b.active) continue;

        // 移动子弹
        switch (b.dir) {
        case 0: b.y -= 5; break;
        case 1: b.x += 5; b.y -= 5; break;
        case 2: b.x += 5; break;
        case 3: b.x += 5; b.y += 5; break;
            // ...其他方向
        }

        // 绘制子弹
        setfillcolor(YELLOW);
        fillcircle(b.x, b.y, 3);

        // 碰撞检测
        for (auto& e : enemies) {
            if (e.active && abs(b.x - e.x) < 15 && abs(b.y - e.y) < 15) {
                e.hp -= player.attack;
                if (e.hp <= 0) {
                    e.active = false;
                    player.exp += 10;
                }
                b.active = false;
            }
        }
    }

    // 显示状态信息
    settextcolor(WHITE);
    TCHAR str[64];
    swprintf_s(str, _T("HP: %d/%d  ATK: %d  DEF: %d  LV: %d"),
        player.hp, player.maxHp, player.attack, player.defense, player.level);
    outtextxy(10, 10, str);

    // 升级检测
    if (player.exp >= 100) {
        player.level++;
        player.exp -= 100;
        gameState = LEVEL_UP;
    }

    // 处理游戏状态
    switch (gameState) {
    case LEVEL_UP:
        LevelUpMenu();
        if (GetAsyncKeyState('1')) { player.attack += 5; gameState = PLAYING; }
        if (GetAsyncKeyState('2')) { player.maxHp += 50; gameState = PLAYING; }
        if (GetAsyncKeyState('3')) { player.defense += 3; gameState = PLAYING; }
        break;
    case GAME_OVER:
        outtextxy(350, 300, _T("Game Over!"));
        break;
    }

    EndBatchDraw();
}

int main() {
    initgraph(800, 600);
    GameInit();

    while (true) {
        if (player.hp <= 0) gameState = GAME_OVER;

        if (gameState == PLAYING) {
            PlayerControl();
        }

        GameLoop();
        Sleep(10);
    }

    closegraph();
    return 0;
}
