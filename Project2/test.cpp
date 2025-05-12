#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <vector>
#include <cmath>
using namespace std;

// 游戏状态
enum GameState { PLAYING, GAME_OVER, LEVEL_UP };

// 玩家结构体
struct Player {
    int x = 400, y = 300;
    int hp = 100, maxHp = 100;
    int attack = 10, defense = 5;
    int level = 1, exp = 0;
} player;

// 子弹结构体
struct Bullet {
    int x, y, dx, dy, damage;
    bool active;
};

// 敌人配置
enum AttackPattern { MELEE, RANGED, CHARGE };
struct EnemyConfig {
    int maxHp, damage, attackRange, attackInterval;
    float speed;
    AttackPattern pattern;
    COLORREF color;
};
const EnemyConfig ENEMY_TYPES[] = {
    {80,15,40,120,1.5f,MELEE,RGB(255,100,100)},
    {50,10,200,180,1.0f,RANGED,RGB(100,200,100)},
    {120,20,80,240,2.0f,CHARGE,RGB(100,100,255)}
};

// 敌人结构体
struct Enemy {
    int x, y, hp, attackTimer = 0;
    bool active = true;
    EnemyConfig config;
    int chargeX, chargeY;
};

// 游戏全局数据
vector<Enemy> enemies;
vector<Bullet> playerBullets, enemyBullets;
GameState gameState = PLAYING;
const int tileSize = 32, mapW = 25, mapH = 18;
int mapData[mapH][mapW] = { 0 };
int spawnTimer = 0;

// 碰撞检测
bool CheckCollision(int x1, int y1, int x2, int y2, int r) {
    int dx = x2 - x1, dy = y2 - y1;
    return dx * dx + dy * dy < r * r;
}

// 初始化游戏
void GameInit() {
    srand(time(0));
    // 生成随机地图
    for (int y = 0; y < mapH; y++)
        for (int x = 0; x < mapW; x++)
            mapData[y][x] = (x == 0 || y == 0 || x == mapW - 1 || y == mapH - 1) ? 1 : rand() % 10 < 3;
}

// 绘制地图
void DrawMap() {
    for (int y = 0; y < mapH; y++)
        for (int x = 0; x < mapW; x++)
            if (mapData[y][x]) {
                setfillcolor(RGB(150, 150, 150));
                fillrectangle(x * tileSize, y * tileSize, (x + 1) * tileSize, (y + 1) * tileSize);
            }
}

// 敌人移动逻辑
void EnemyMove(Enemy& e, const Player& p) {
    float dx = p.x - e.x, dy = p.y - e.y;
    float dist = sqrt(dx * dx + dy * dy);

    if (e.config.pattern == CHARGE && dist < e.config.attackRange) {
        e.x += e.chargeX * 3;
        e.y += e.chargeY * 3;
    }
    else {
        if (dist > 0) { dx /= dist; dy /= dist; }
        e.x += dx * e.config.speed;
        e.y += dy * e.config.speed;
        e.chargeX = dx;
        e.chargeY = dy;
    }
}

// 敌人攻击逻辑
void EnemyAttack(Enemy& e, Player& p) {
    if (--e.attackTimer > 0) return;

    float dx = p.x - e.x, dy = p.y - e.y;
    float dist = sqrt(dx * dx + dy * dy);

    if (dist > e.config.attackRange) return;

    switch (e.config.pattern) {
    case MELEE:
        p.hp -= e.config.damage;
        e.attackTimer = e.config.attackInterval;
        break;
    case RANGED:
        dx /= dist; dy /= dist;
        //enemyBullets.push_back({ e.x,e.y,dx * 5,dy * 5,e.config.damage,true });
        enemyBullets.push_back({
        e.x,                    // int x
        e.y,                    // int y
        static_cast<int>(dx * 5), // int dx (确保类型正确)
        static_cast<int>(dy * 5), // int dy
        e.config.damage,        // int damage
        true                    // bool active
            });
        e.attackTimer = e.config.attackInterval;
        break;
    case CHARGE:
        p.hp -= e.config.damage;
        e.attackTimer = e.config.attackInterval * 2;
        break;
    }
}

// 玩家控制
void PlayerControl() {
    // 移动
    int nx = player.x, ny = player.y;
    if (GetAsyncKeyState('W') & 0x8000) ny -= 3;
    if (GetAsyncKeyState('S') & 0x8000) ny += 3;
    if (GetAsyncKeyState('A') & 0x8000) nx -= 3;
    if (GetAsyncKeyState('D') & 0x8000) nx += 3;

    // 碰撞检测
    bool canMove = true;
    RECT rc;
    for (int y = ny / tileSize - 1; y <= ny / tileSize + 1; y++)
        for (int x = nx / tileSize - 1; x <= nx / tileSize + 1; x++)
            if (x >= 0 && y >= 0 && x < mapW && y < mapH && mapData[y][x]) {
                RECT wall = { x * tileSize, y * tileSize, (x + 1) * tileSize, (y + 1) * tileSize };
                RECT player = { nx - 10, ny - 10, nx + 10, ny + 10 };
                if (IntersectRect(&rc, &player, &wall)) canMove = false;
            }
    if (canMove) { player.x = nx; player.y = ny; }

    // 射击
    static int shootTimer = 0;
    if (GetAsyncKeyState(VK_SPACE) && shootTimer <= 0) {
        for (int i = 0; i < 8; i++) {
            float a = i * 3.1416f / 4;
           // playerBullets.push_back({ player.x,player.y,cos(a) * 5,sin(a) * 5,5,true });
            playerBullets.push_back({
        player.x,                    // int x
        player.y,                    // int y
        static_cast<int>(cos(a) * 5), // int dx (确保类型正确)
        static_cast<int>(sin(a) * 5), // int dy
        5,        // int damage
        true                    // bool active
                });
        }
        shootTimer = 20;
    }
    shootTimer--;
}

// 游戏主循环
void GameLoop() {
    BeginBatchDraw();
    cleardevice();

    // 绘制地图和玩家
    DrawMap();
    setfillcolor(BLUE);
    fillcircle(player.x, player.y, 10);

    // 更新敌人
    auto e = enemies.begin();
    while (e != enemies.end()) {
        if (!e->active) { e = enemies.erase(e); continue; }

        EnemyMove(*e, player);
        EnemyAttack(*e, player);

        // 绘制敌人
        setfillcolor(e->config.color);
        fillcircle(e->x, e->y, e->config.pattern == MELEE ? 12 : 8);

        // 玩家子弹碰撞
        auto b = playerBullets.begin();
        while (b != playerBullets.end()) {
            if (b->active && CheckCollision(b->x, b->y, e->x, e->y, 15)) {
                e->hp -= player.attack;
                b->active = false;
                b = playerBullets.erase(b);
                if (e->hp <= 0) { player.exp += 50; e->active = false; }
            }
            else ++b;
        }
        ++e;
    }

    // 敌人生成
    if (--spawnTimer <= 0) {
        enemies.push_back({
            rand() % 800, rand() % 600,
            ENEMY_TYPES[rand() % 3].maxHp,
            0, true,
            ENEMY_TYPES[rand() % 3]
            });
        spawnTimer = 120;
    }

    // 更新子弹
    auto UpdateBullets = [](vector<Bullet>& bs, bool isPlayer) {
        auto b = bs.begin();
        while (b != bs.end()) {
            if (!b->active) { b = bs.erase(b); continue; }
            b->x += b->dx; b->y += b->dy;

            if (b->x < 0 || b->x>800 || b->y < 0 || b->y>600)
                b->active = false;
            else if (isPlayer && CheckCollision(b->x, b->y, player.x, player.y, 10))
                player.hp -= b->damage;

            setfillcolor(isPlayer ? YELLOW : RED);
            fillcircle(b->x, b->y, 3);
            ++b;
        }
        };
    UpdateBullets(playerBullets, false);
    UpdateBullets(enemyBullets, true);

    // 显示状态
    settextcolor(WHITE);
    TCHAR str[64];
    swprintf_s(str, L"HP: %d/%d  ATK: %d  LV: %d",
        player.hp, player.maxHp, player.attack, player.level);
    outtextxy(10, 10, str);

    // 升级系统
    if (player.exp >= 100) {
        gameState = LEVEL_UP;
        player.level++;
        player.exp -= 100;
    }
    if (gameState == LEVEL_UP) {
        settextstyle(24, 0, L"宋体");
        outtextxy(300, 200, L"1. 攻击+5   2. 生命+50");
        if (GetAsyncKeyState('1')) { player.attack += 5; gameState = PLAYING; }
        if (GetAsyncKeyState('2')) { player.maxHp += 50; gameState = PLAYING; }
    }

    EndBatchDraw(); 
}

int main() {
    initgraph(800, 600);
    GameInit();

    while (true) {
        if (player.hp <= 0) gameState = GAME_OVER;
        if (gameState == PLAYING) PlayerControl();
        GameLoop();
        Sleep(10);
    }

    closegraph();
    return 0;
}
