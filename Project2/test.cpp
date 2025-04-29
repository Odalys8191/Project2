#include <graphics.h>      // EasyXͼ�ο�
#include <conio.h>         // ��������
#include <vector>          // ��̬����
#include <ctime>           // �������

// ��Ϸ����
const int TILE_SIZE = 32;  // �������ش�С
const int MAP_W = 20;      // ��ͼ�������
const int MAP_H = 15;      // ��ͼ�߸�����

// ��ͼ��������
enum TileType {
    FLOOR,   // �ذ�
    WALL,    // ǽ
    DOOR     // ��
};

// ��ɫ��
class Character {
public:
    int x, y;          // ��ͼ����
    int hp;            // ����ֵ
    int attack;        // ������
    COLORREF color;    // ��ʾ��ɫ

    Character(int x, int y, COLORREF c) : x(x), y(y), hp(10), attack(2), color(c) {}

    // ���ƽ�ɫ
    void draw() const {
        setfillcolor(color);
        solidcircle(x * TILE_SIZE + TILE_SIZE / 2, y * TILE_SIZE + TILE_SIZE / 2, TILE_SIZE / 3);
    }
};

// ��Ϸ״̬
struct Game {
    TileType map[MAP_W][MAP_H];  // ��ά��ͼ����
    Character player{ 1, 1, RED }; // ��ҽ�ɫ
    std::vector<Character> enemies; // �����б�
    bool isRunning = true;       // ��Ϸ���б�־
};

// ���������ͼ����ʾ����
void generateMap(Game& game) {
    srand(time(0)); // �������

    for (int x = 0; x < MAP_W; x++) {
        for (int y = 0; y < MAP_H; y++) {
            // �߽�Ϊǽ���ڲ��������
            if (x == 0 || y == 0 || x == MAP_W - 1 || y == MAP_H - 1) {
                game.map[x][y] = WALL;
            }
            else {
                game.map[x][y] = (rand() % 10 > 7) ? WALL : FLOOR;
            }
        }
    }
}

// ���Ƶ�ͼ
void drawMap(const Game& game) {
    for (int x = 0; x < MAP_W; x++) {
        for (int y = 0; y < MAP_H; y++) {
            // ���ݸ�������������ɫ
            COLORREF color = DARKGRAY;
            switch (game.map[x][y]) {
            case WALL:  color = BROWN; break;
            case FLOOR: color = RGB(50, 50, 50); break;
            case DOOR:  color = YELLOW; break;
            }

            // ���Ƹ���
            setfillcolor(color);
            solidrectangle(x * TILE_SIZE, y * TILE_SIZE,
                (x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE);
        }
    }
}

// �����������
void processInput(Game& game) {
    if (!_kbhit()) return; // �ް���ʱֱ�ӷ���

    int dx = 0, dy = 0;
    switch (_getch()) {
    case 'W': case 'w': dy = -1; break;
    case 'S': case 's': dy = 1; break;
    case 'A': case 'a': dx = -1; break;
    case 'D': case 'd': dx = 1; break;
    case 27: game.isRunning = false; break; // ESC�˳�
    }

    // ������λ��
    int newX = game.player.x + dx;
    int newY = game.player.y + dy;

    // ��ײ���
    if (newX >= 0 && newX < MAP_W &&
        newY >= 0 && newY < MAP_H &&
        game.map[newX][newY] != WALL) {
        game.player.x = newX;
        game.player.y = newY;
    }
}

// ������
int main() {
    initgraph(MAP_W * TILE_SIZE, MAP_H * TILE_SIZE); // ��������
    Game game;
    generateMap(game);

    // ���ɲ��Ե���
    game.enemies.emplace_back(5, 5, GREEN);
    game.enemies.emplace_back(8, 3, BLUE);

    // ��Ϸ��ѭ��
    while (game.isRunning) {
        BeginBatchDraw(); // ��ʼ�������ƣ�����˸��

        cleardevice();    // ��ջ���
        drawMap(game);    // ���Ƶ�ͼ
        game.player.draw(); // �������

        // �������е���
        for (const auto& enemy : game.enemies) {
            enemy.draw();
        }

        processInput(game); // ��������
        EndBatchDraw();   // ������������
        Sleep(50);        // ����ѭ���ٶ�
    }

    closegraph(); // �ر�ͼ�δ���
    return 0;
}