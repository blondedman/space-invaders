#include <GL/glut.h>
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <ctime>

// game constants
const int WIDTH = 800;
const int HEIGHT = 600;
const int ALIEN_ROWS = 4;
const int ALIEN_COLS = 8;

// game state
struct game {

    // player
    float playerX = WIDTH / 2;
    const float playerSpeed = 5.0f;
    bool leftPressed = false;
    bool rightPressed = false;

    // aliens
    float alienX = 50;
    float alienY = 400;
    float alienSpeed = 0.5f;
    bool aliensRight = true;

    // aliens alive state
    bool aliens[ALIEN_ROWS][ALIEN_COLS];

    // bullets
    struct bullet { float x, y, speed; };
    std::vector<bullet> playerBullets;
    std::vector<bullet> alienBullets;
    
    // accuracy
    int totalShots = 0;
    int hits = 0;

    // game state
    int round = 1;
    int score = 0;
    bool gameOver = false;
} game;

// utility function: drawing a string using GLUT bitmap font
void drawString(void* font, const char* string, float x, float y) {
    glRasterPos2f(x, y);
    for (const char* c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void init() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // initialize all aliens to alive
    for (int y = 0; y < ALIEN_ROWS; y++)
        for (int x = 0; x < ALIEN_COLS; x++)
            game.aliens[y][x] = true;

    std::srand((unsigned int)time(0));
}

void drawPlayer() {
    glColor3f(0.0f, 1.0f, 0.0f); // Green
    glBegin(GL_QUADS);
    glVertex2f(game.playerX - 20, 20);
    glVertex2f(game.playerX + 20, 20);
    glVertex2f(game.playerX + 20, 50);
    glVertex2f(game.playerX - 20, 50);
    glEnd();
}

void drawAliens() {
    glColor3f(1.0f, 0.0f, 0.0f); // Red
    for (int y = 0; y < ALIEN_ROWS; y++) {
        for (int x = 0; x < ALIEN_COLS; x++) {
            if (!game.aliens[y][x]) continue; // Skip dead aliens
            glBegin(GL_QUADS);
            float ax = game.alienX + x * 60;
            float ay = game.alienY - y * 40;
            glVertex2f(ax - 15, ay - 15);
            glVertex2f(ax + 15, ay - 15);
            glVertex2f(ax + 15, ay + 15);
            glVertex2f(ax - 15, ay + 15);
            glEnd();
        }
    }
}

void drawBullets() {
    // Player bullets (white)
    glColor3f(1.0f, 1.0f, 1.0f);
    for (auto& b : game.playerBullets) {
        glBegin(GL_QUADS);
        glVertex2f(b.x - 2, b.y - 8);
        glVertex2f(b.x + 2, b.y - 8);
        glVertex2f(b.x + 2, b.y + 8);
        glVertex2f(b.x - 2, b.y + 8);
        glEnd();
    }
    // Alien bullets (yellow)
    glColor3f(1.0f, 1.0f, 0.0f);
    for (auto& b : game.alienBullets) {
        glBegin(GL_QUADS);
        glVertex2f(b.x - 2, b.y - 8);
        glVertex2f(b.x + 2, b.y - 8);
        glVertex2f(b.x + 2, b.y + 8);
        glVertex2f(b.x - 2, b.y + 8);
        glEnd();
    }
}

void keyboard(unsigned char key, int, int) {
    if (key == 'a' || key == 'A') game.leftPressed = true;
    if (key == 'd' || key == 'D') game.rightPressed = true;
    if (key == ' ' && !game.gameOver) { // Shoot
        game.playerBullets.push_back({ game.playerX, 50, 8.0f });
    }

    // accurcay handler
    if (key == ' ' && !game.gameOver) {
        game.playerBullets.push_back({ game.playerX, 50, 8.0f });
        game.totalShots = game.totalShots + 2;
    }
    // restart game on enter after game over
    if (key == 13 && game.gameOver) {
        // reset game state
        game.playerX = WIDTH / 2;
        game.alienX = 50;
        game.alienY = 400;
        game.aliensRight = true;
        game.playerBullets.clear();
        game.alienBullets.clear();
        game.score = 0;
        game.gameOver = false;
        for (int y = 0; y < ALIEN_ROWS; y++)
            for (int x = 0; x < ALIEN_COLS; x++)
                game.aliens[y][x] = true;
        game.round = 1;
        game.totalShots = 0;
        game.hits = 0;
    }
}

void keyboardUp(unsigned char key, int, int) {
    if (key == 'a' || key == 'A') game.leftPressed = false;
    if (key == 'd' || key == 'D') game.rightPressed = false;
}

void checkCollisions() {
    for (auto it = game.playerBullets.begin(); it != game.playerBullets.end(); ) {
        bool bulletUsed = false;

        // Check collision with all aliens
        for (int y = 0; y < ALIEN_ROWS && !bulletUsed; y++) {
            for (int x = 0; x < ALIEN_COLS && !bulletUsed; x++) {
                if (!game.aliens[y][x]) continue;

                // Alien bounding box
                float ax = game.alienX + x * 60;
                float ay = game.alienY - y * 40;
                float alienLeft = ax - 15;
                float alienRight = ax + 15;
                float alienBottom = ay - 15;
                float alienTop = ay + 15;

                // Bullet bounding box
                float bulletLeft = it->x - 2;
                float bulletRight = it->x + 2;
                float bulletBottom = it->y - 8;
                float bulletTop = it->y + 8;

                // AABB collision check
                if (bulletLeft < alienRight &&
                    bulletRight > alienLeft &&
                    bulletBottom < alienTop &&
                    bulletTop > alienBottom) {

                    // Destroy alien and bullet
                    game.aliens[y][x] = false;
                    game.score += 100;
                    game.hits++;
                    it = game.playerBullets.erase(it);
                    bulletUsed = true; // Exit both loops
                }
            }
        }

        if (!bulletUsed) ++it; // Only increment if bullet wasn't used
    }

    // alien bullets vs player (unchanged)
    float px1 = game.playerX - 20, px2 = game.playerX + 20;
    float py1 = 20, py2 = 50;
    for (auto it = game.alienBullets.begin(); it != game.alienBullets.end(); ) {
        if (it->x > px1 && it->x < px2 && it->y > py1 && it->y < py2) {
            game.gameOver = true;
            break;
        }
        ++it;
    }
}

void specialInput(int key, int, int) {
    if (key == GLUT_KEY_LEFT)  game.leftPressed = true;
    if (key == GLUT_KEY_RIGHT) game.rightPressed = true;
}

void specialUpInput(int key, int, int) {
    if (key == GLUT_KEY_LEFT)  game.leftPressed = false;
    if (key == GLUT_KEY_RIGHT) game.rightPressed = false;
}

void update(int) {
    if (!game.gameOver) {
        
        // player movement
        if (game.leftPressed) game.playerX -= game.playerSpeed;
        if (game.rightPressed) game.playerX += game.playerSpeed;
        game.playerX = std::max(20.0f, std::min((float)WIDTH - 20, game.playerX));

        // round system check
        bool allDead = true;
        for (int y = 0; y < ALIEN_ROWS; y++) {
            for (int x = 0; x < ALIEN_COLS; x++) {
                if (game.aliens[y][x]) {
                    allDead = false;
                    break;
                }
            }
            if (!allDead) break;
        }

        if (allDead) {
            game.round++;
            // reset aliens
            for (int y = 0; y < ALIEN_ROWS; y++)
                for (int x = 0; x < ALIEN_COLS; x++)
                    game.aliens[y][x] = true;
            // reset alien position
            game.alienX = 50;
            game.alienY = 400;
            game.aliensRight = true;
            game.alienSpeed *= 1.5f;
            // clear bullets
            game.playerBullets.clear();
            game.alienBullets.clear();
        }
        // alien movement
        int aliveLeft = ALIEN_COLS, aliveRight = -1;
        for (int y = 0; y < ALIEN_ROWS; y++)
            for (int x = 0; x < ALIEN_COLS; x++)
                if (game.aliens[y][x]) {
                    aliveLeft = std::min(aliveLeft, x);
                    aliveRight = std::max(aliveRight, x);
                }
        float leftEdge = game.alienX + aliveLeft * 60 - 15;
        float rightEdge = game.alienX + aliveRight * 60 + 15;
        if (game.aliensRight) {
            game.alienX += game.alienSpeed;
            if (rightEdge > WIDTH - 10) {
                game.aliensRight = false;
                game.alienY -= 20;
            }
        }
        else {
            game.alienX -= game.alienSpeed;
            if (leftEdge < 10) {
                game.aliensRight = true;
                game.alienY -= 20;
            }
        }

        // update bullets
        for (auto& b : game.playerBullets) b.y += b.speed;
        for (auto& b : game.alienBullets) b.y -= b.speed;

        // remove off-screen bullets
        game.playerBullets.erase(
            std::remove_if(game.playerBullets.begin(), game.playerBullets.end(),
                [](const game::bullet& b) { return b.y > HEIGHT; }),
            game.playerBullets.end()
        );
        game.alienBullets.erase(
            std::remove_if(game.alienBullets.begin(), game.alienBullets.end(),
                [](const game::bullet& b) { return b.y < 0; }),
            game.alienBullets.end()
        );

        // alien shooting (random alive alien)
        if (rand() % 25 == 0) { // adjust for more/less frequent shooting
            std::vector<std::pair<int, int>> aliveAliens;
            for (int y = 0; y < ALIEN_ROWS; y++)
                for (int x = 0; x < ALIEN_COLS; x++)
                    if (game.aliens[y][x])
                        aliveAliens.push_back({ y, x });
            if (!aliveAliens.empty()) {
                int idx = rand() % aliveAliens.size();
                int ay = aliveAliens[idx].first;
                int ax = aliveAliens[idx].second;
                game.alienBullets.push_back({
                    game.alienX + ax * 60,
                    game.alienY - ay * 40,
                    4.0f
                    });
            }
        }

        // check collisions
        checkCollisions();

        // game over condition: aliens reach player area
        for (int y = 0; y < ALIEN_ROWS; y++)
            for (int x = 0; x < ALIEN_COLS; x++)
                if (game.aliens[y][x]) {
                    float ay = game.alienY - y * 40;
                    if (ay - 15 < 60) // Approaching player
                        game.gameOver = true;
                }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // ~60 FPS
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawPlayer();
    drawAliens();
    drawBullets();

    //  game score
    glColor3f(1.0f, 1.0f, 1.0f);
    char scoreStr[64];
    sprintf_s(scoreStr, "SCORE: %d", game.score);
    drawString(GLUT_BITMAP_HELVETICA_18, scoreStr, 10, HEIGHT - 30);
    
    char roundStr[32];
    sprintf_s(roundStr, "ROUND: %d", game.round);
    drawString(GLUT_BITMAP_HELVETICA_18, roundStr, WIDTH / 2 - 50, HEIGHT - 30);

    // shot accuracy
    float accuracy = game.totalShots > 0 ?
        (static_cast<float>(game.hits) / game.totalShots) * 100.0f : 0.0f;
    char accStr[32];
    sprintf_s(accStr, "ACCURACY: %.1f%%", accuracy);
    drawString(GLUT_BITMAP_HELVETICA_18, accStr,WIDTH - 190, HEIGHT - 30);
    
    // game over
    if (game.gameOver) {
        drawString(GLUT_BITMAP_HELVETICA_18, "GAME OVER", WIDTH / 2 - 60, HEIGHT / 2);
        drawString(GLUT_BITMAP_HELVETICA_12, "PRESS ENTER TO RESTART", WIDTH / 2 - 70, HEIGHT / 2 - 30);
    }

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("SPACE INVADERS");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialInput);
    glutSpecialUpFunc(specialUpInput);
    glutTimerFunc(0, update, 0);
    glutMainLoop();
    return 0;
}