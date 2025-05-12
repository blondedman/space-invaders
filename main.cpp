#include <GL/glut.h>
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <ctime>

// game constants
const int WIDTH = 1000;
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
    struct bullet {
        float x, y, speed;
        float dx = 0, dy = 0; // direction for homing
        bool homing = false;
    };

    std::vector<bullet> playerBullets;
    std::vector<bullet> alienBullets;
    
    // accuracy
    int totalShots = 0;
    int hits = 0;

    // game state
    int round = 1;
    int score = 0;
    bool paused = false;
    bool gameOver = false;

    bool slowAlienBulletsActive = false;
    int slowAlienBulletsTimer = 0; // frames or ms

    bool homingBulletsActive = false;
    int homingBulletsTimer = 0; // frames or ms

    bool shieldActive = false;
    int shieldTimer = 0; // frames or ms


    // power-ups
    struct powerup { float x, y; int type; int timer; };
    std::vector<powerup> powerups;

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void drawPlayer() {
    float cx = game.playerX;
    float baseY = 20.0f;
    
    // draw shield if active
    if (game.shieldActive) {
        float cx = game.playerX;
        float baseY = 20.0f;
        glColor4f(0.3f, 0.7f, 1.0f, 0.28f); // blue, semi-transparent
        glBegin(GL_POLYGON);
        for (int i = 0; i < 32; ++i) {
            float theta = 2.0f * 3.14159f * i / 32;
            glVertex2f(cx + cos(theta) * 28, baseY + 16 + sin(theta) * 28);
        }
        glEnd();
    }

    // main body
    glColor3f(0.2f, 0.8f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2f(cx - 12, baseY + 16);
    glVertex2f(cx - 18, baseY + 12);
    glVertex2f(cx - 12, baseY + 4);
    glVertex2f(cx + 12, baseY + 4);
    glVertex2f(cx + 18, baseY + 12);
    glVertex2f(cx + 12, baseY + 16);
    glEnd();

    // cockpit
    glColor3f(0.3f, 0.3f, 1.0f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 32; ++i) {
        float theta = i / 32.0f * 3.14159f;
        float x = cx + cos(theta) * 6;
        float y = baseY + 16 + sin(theta) * 8;
        glVertex2f(x, y);
    }
    glEnd();

    // nose
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(cx, baseY + 34);
    glVertex2f(cx - 8, baseY + 16);
    glVertex2f(cx + 8, baseY + 16);
    glEnd();

    // left wing
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_POLYGON);
    glVertex2f(cx - 12, baseY + 12);
    glVertex2f(cx - 28, baseY + 6);
    glVertex2f(cx - 24, baseY + 2);
    glVertex2f(cx - 12, baseY + 4);
    glEnd();

    // right wing
    glBegin(GL_POLYGON);
    glVertex2f(cx + 12, baseY + 12);
    glVertex2f(cx + 28, baseY + 6);
    glVertex2f(cx + 24, baseY + 2);
    glVertex2f(cx + 12, baseY + 4);
    glEnd();

    // left engine
    glColor3f(0.8f, 0.4f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 22, baseY + 0);
    glVertex2f(cx - 18, baseY + 0);
    glVertex2f(cx - 18, baseY + 4);
    glVertex2f(cx - 22, baseY + 4);
    glEnd();

    // right engine
    glBegin(GL_QUADS);
    glVertex2f(cx + 18, baseY + 0);
    glVertex2f(cx + 22, baseY + 0);
    glVertex2f(cx + 22, baseY + 4);
    glVertex2f(cx + 18, baseY + 4);
    glEnd();

    // engine flames (flicker effect)
    glColor3f(1.0f, 0.8f, 0.1f);
    glBegin(GL_TRIANGLES);
    glVertex2f(cx - 20, baseY + 0);
    glVertex2f(cx - 19, baseY - 8 - rand() % 4);
    glVertex2f(cx - 21, baseY - 8 - rand() % 4);
    glVertex2f(cx + 20, baseY + 0);
    glVertex2f(cx + 19, baseY - 8 - rand() % 4);
    glVertex2f(cx + 21, baseY - 8 - rand() % 4);
    glEnd();

    // highlights on body
    glColor3f(0.8f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex2f(cx, baseY + 4);
    glVertex2f(cx, baseY + 16);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(cx - 6, baseY + 8);
    glVertex2f(cx - 6, baseY + 14);
    glVertex2f(cx + 6, baseY + 8);
    glVertex2f(cx + 6, baseY + 14);
    glEnd();
}

void drawEvilAlienShip(float x, float y) {
    // main hull (dark, spiky, compact)
    glColor3f(0.35f, 0.05f, 0.15f);
    glBegin(GL_POLYGON);
    glVertex2f(x, y + 10);
    glVertex2f(x - 10, y + 4);
    glVertex2f(x - 13, y - 4);
    glVertex2f(x - 6, y - 12);
    glVertex2f(x, y - 16);
    glVertex2f(x + 6, y - 12);
    glVertex2f(x + 13, y - 4);
    glVertex2f(x + 10, y + 4);
    glEnd();

    // cockpit
    glColor3f(0.9f, 0.1f, 0.2f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 14; ++i) {
        float theta = 2.0f * 3.14159f * i / 14;
        glVertex2f(x + cos(theta) * 3.5f, y - 1 + sin(theta) * 3.5f);
    }
    glEnd();

    // side spikes
    glColor3f(0.6f, 0.0f, 0.2f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 13, y - 4);
    glVertex2f(x - 18, y - 8);
    glVertex2f(x - 6, y - 12);

    glVertex2f(x + 13, y - 4);
    glVertex2f(x + 18, y - 8);
    glVertex2f(x + 6, y - 12);
    glEnd();

    // glowing spoons
    glColor3f(1.0f, 0.9f, 0.2f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 8; ++i) {
        float theta = 2.0f * 3.14159f * i / 8;
        glVertex2f(x - 4.5f + cos(theta) * 1.0f, y - 7 + sin(theta) * 1.0f);
    }
    glEnd();
    glBegin(GL_POLYGON);
    for (int i = 0; i < 8; ++i) {
        float theta = 2.0f * 3.14159f * i / 8;
        glVertex2f(x + 4.5f + cos(theta) * 1.0f, y - 7 + sin(theta) * 1.0f);
    }
    glEnd();

    // lower mandibles (dark)
    glColor3f(0.25f, 0.0f, 0.1f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 3, y - 14);
    glVertex2f(x - 1, y - 19);
    glVertex2f(x, y - 16);

    glVertex2f(x + 3, y - 14);
    glVertex2f(x + 1, y - 19);
    glVertex2f(x, y - 16);
    glEnd();
}

void drawAliens() {
    for (int y = 0; y < ALIEN_ROWS; y++) {
        for (int x = 0; x < ALIEN_COLS; x++) {
            if (!game.aliens[y][x]) continue;
            float ax = game.alienX + x * 60;
            float ay = game.alienY - y * 40;
            drawEvilAlienShip(ax, ay);
        }
    }
}

void drawBullets() {

    // player bullets (thin laser)
    /*
    for (auto& b : game.playerBullets) {
        // outer glow (optional, for effect)
        glColor4f(0.2f, 1.0f, 1.0f, 0.18f);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 16; ++i) {
            float theta = 2.0f * 3.14159f * i / 16;
            glVertex2f(b.x + cos(theta) * 3, b.y + sin(theta) * 12);
        }
        glEnd();

        // thin core
        glColor3f(0.8f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(b.x - 1, b.y - 10);
        glVertex2f(b.x + 1, b.y - 10);
        glVertex2f(b.x + 1, b.y + 14);
        glVertex2f(b.x - 1, b.y + 14);
        glEnd();
    }
    */
    for (auto& b : game.playerBullets) {
        float angle = atan2(b.dy, b.dx);

        glPushMatrix();
        glTranslatef(b.x, b.y, 0);
        glRotatef(angle * 180.0f / 3.14159f - 90.0f, 0, 0, 1); // -90 so tip points along (dx,dy)

        // outer glow (optional)
        glColor4f(0.2f, 1.0f, 1.0f, 0.18f);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 16; ++i) {
            float theta = 2.0f * 3.14159f * i / 16;
            glVertex2f(cos(theta) * 5, sin(theta) * 16);
        }
        glEnd();

        // bullet tip (triangle)
        glColor3f(0.8f, 1.0f, 1.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(0, 14);    // tip
        glVertex2f(-3, -8);   // left base
        glVertex2f(3, -8);    // right base
        glEnd();

        glPopMatrix();
    }

    // alien bullets (yellow)
    for (auto& b : game.alienBullets) {
        float pulse = 1.0f + 0.3f * sin(b.y * 0.25f);
        glColor3f(0.6f + 0.3f * pulse, 1.0f - 0.4f * pulse, 0.1f);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 18; ++i) {
            float theta = 2.0f * 3.14159f * i / 18;
            glVertex2f(b.x + cos(theta) * 7 * pulse, b.y + sin(theta) * 7 * pulse);
        }
        glEnd();
    }
}

void drawPowerups() {
    for (auto& p : game.powerups) {

        // color based on power-up type
        float r = 0.3f, g = 1.0f, b = 0.7f; // default: green (slow bullets)
        if (p.type == 1) { // slow alien bullets
            r = 0.3f; g = 1.0f; b = 0.7f; // green/cyan
        }
        else if (p.type == 2) { // homing bullets
            r = 1.0f; g = 0.0f; b = 0.2f; // orange/red
        }
        else if (p.type == 3) { // shield
            r = 0.5f; g = 0.7f; b = 1.0f; // blue
        }
        
        // outer glow
        glColor4f(r, g, b, 0.18f);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 24; ++i) {
            float theta = 2.0f * 3.14159f * i / 24;
            glVertex2f(p.x + cos(theta) * 14, p.y + sin(theta) * 14);
        }
        glEnd();

        // main orb
        glColor3f(r, g, b);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 20; ++i) {
            float theta = 2.0f * 3.14159f * i / 20;
            glVertex2f(p.x + cos(theta) * 8, p.y + sin(theta) * 8);
        }
        glEnd();
        // inner highlight
        glColor4f(1.0f, 1.0f, 1.0f, 0.28f);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 12; ++i) {
            float theta = 2.0f * 3.14159f * i / 12;
            glVertex2f(p.x + cos(theta) * 4, p.y + sin(theta) * 4);
        }
        glEnd();
    }
}

void keyboard(unsigned char key, int, int) {
    
    if (key == 27) { // ESC key
        game.paused = !game.paused; // toggle pause on/off
        return;
    }

    if (key == 'a' || key == 'A') game.leftPressed = true;
    if (key == 'd' || key == 'D') game.rightPressed = true;
    
    if (key == ' ' && !game.gameOver) {
        game.totalShots++;
        if (game.homingBulletsActive) {
            game.playerBullets.push_back({ game.playerX, 50, 8.0f, 0, 1.0f, true });
        }
        else {
            game.playerBullets.push_back({ game.playerX, 50, 8.0f, 0, 1.0f, false });
        }
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
        game.alienSpeed = 0.5f;
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

// glutSpecialFunc(specialInput); 
// tells GLUT to call specialInput function when a special key is pressed

//glutSpecialUpFunc(specialUpInput); 
// tells GLUT to call specialUpInput function when a special key is released

// both of these are used in update() 
// lets us use the LEFT and RIGHT arrow keys

void specialInput(int key, int, int) {
    if (key == GLUT_KEY_LEFT)  game.leftPressed = true;
    if (key == GLUT_KEY_RIGHT) game.rightPressed = true;
}

void specialUpInput(int key, int, int) {
    if (key == GLUT_KEY_LEFT)  game.leftPressed = false;
    if (key == GLUT_KEY_RIGHT) game.rightPressed = false;
}

void checkCollisions() {
    for (auto it = game.playerBullets.begin(); it != game.playerBullets.end(); ) {
        bool bulletUsed = false;

        // check collision with all aliens
        for (int y = 0; y < ALIEN_ROWS && !bulletUsed; y++) {
            for (int x = 0; x < ALIEN_COLS && !bulletUsed; x++) {
                if (!game.aliens[y][x]) continue;

                // alien bounding box
                float ax = game.alienX + x * 60;
                float ay = game.alienY - y * 40;
                float alienLeft = ax - 15;
                float alienRight = ax + 15;
                float alienBottom = ay - 15;
                float alienTop = ay + 15;

                // bullet bounding box
                float bulletLeft = it->x - 2;
                float bulletRight = it->x + 2;
                float bulletBottom = it->y - 8;
                float bulletTop = it->y + 8;

                // AABB collision check
                if (bulletLeft < alienRight &&
                    bulletRight > alienLeft &&
                    bulletBottom < alienTop &&
                    bulletTop > alienBottom) {

                    // destroy alien and bullet
                    game.aliens[y][x] = false;

                    // power-ups logic
                    if (rand() % 10 == 0) {
                        game.powerups.push_back({ ax, ay, 1, 0 }); // type 1 = slow bullets
                    }

                    if (rand() % 10 == 0) {
                        game.powerups.push_back({ ax, ay, 2, 0 }); // type 2 = homing bullets
                    }

                    if (rand() % 20 == 0) {
                        game.powerups.push_back({ ax, ay, 3, 0 }); // type 3 = shield
                    }

                    game.score += 100;
                    game.hits++;
                    it = game.playerBullets.erase(it);
                    bulletUsed = true;
                }
            }
        }

        if (!bulletUsed) ++it; // only increment if bullet wasn't used
    }

    // alien bullets vs player
    float px1 = game.playerX - 20, px2 = game.playerX + 20;
    float py1 = 20, py2 = 50;
    for (auto it = game.alienBullets.begin(); it != game.alienBullets.end(); ) {
        if (it->x > px1 && it->x < px2 && it->y > py1 && it->y < py2) {
            if (game.shieldActive) {
                // shield active
                it = game.alienBullets.erase(it);
                continue;
            }
            else {
                // no shield active
                game.gameOver = true;
                break;
            }
        }

        ++it;
    }

    // power-up collection
    for (auto it = game.powerups.begin(); it != game.powerups.end(); ) {
        // check if player collects the powerup (e.g., overlap with player)
        if (fabs(it->x - game.playerX) < 20 && it->y < 60) { // adjust bounds as needed
            if (it->type == 1) { // 1 = slow alien bullets
                game.slowAlienBulletsActive = true;
                game.slowAlienBulletsTimer = 200; // 3s at 60 FPS
            }
            if (it->type == 2) { // 2 = homing bullets
                game.homingBulletsActive = true;
                game.homingBulletsTimer = 200; // 3s at 60 FPS
            }
            if (it->type == 3) { // 3 = shield
                game.shieldActive = true;
                game.shieldTimer = 600; // 10s 60 FPS
            }

            it = game.powerups.erase(it); // remove collected powerup
        }
        else {
            ++it;
        }
    }
}

void update(int)     {
    
    if (game.round > 15) {
        game.round--;
        game.gameOver = true;
    }

    if (game.paused) {
        glutTimerFunc(16, update, 0); // timer running for input
        return;
    }

    if (!game.gameOver) {
        // power-up
        if (game.slowAlienBulletsActive) {
            game.slowAlienBulletsTimer--;
            if (game.slowAlienBulletsTimer <= 0)
                game.slowAlienBulletsActive = false;
        }

        if (game.homingBulletsActive) {
            game.homingBulletsTimer--;
            if (game.homingBulletsTimer <= 0)
                game.homingBulletsActive = false;
        }

        if (game.shieldActive) {
            game.shieldTimer--;
            if (game.shieldTimer <= 0)
                game.shieldActive = false;
        }

        for (auto& p : game.powerups) {
            p.y -= 2.0f; // adjust speed as desired
        }
        game.powerups.erase(
            std::remove_if(game.powerups.begin(), game.powerups.end(),
                [](const game::powerup& p) { return p.y < 0; }),
            game.powerups.end()
        );

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
            game.alienSpeed *= 1.25f;
            
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

        // update playerBullets
        for (auto& b : game.playerBullets) {
            if (b.homing) {
                // find nearest alive alien
                float bestDist = 1e9, tx = 0, ty = 0;
                bool found = false;
                for (int y = 0; y < ALIEN_ROWS; y++) {
                    for (int x = 0; x < ALIEN_COLS; x++) {
                        if (!game.aliens[y][x]) continue;
                        float ax = game.alienX + x * 60;
                        float ay = game.alienY - y * 40;
                        float dx = ax - b.x, dy = ay - b.y;
                        float dist = dx * dx + dy * dy;
                        if (dist < bestDist) {
                            bestDist = dist;
                            tx = ax;
                            ty = ay;
                            found = true;
                        }
                    }
                }
                if (found) {
                    float vx = tx - b.x, vy = ty - b.y;
                    float len = sqrt(vx * vx + vy * vy);
                    if (len > 1e-2) {
                        vx /= len; vy /= len;
                        b.dx = b.dx * 0.85f + vx * 0.15f;
                        b.dy = b.dy * 0.85f + vy * 0.15f;
                        float dlen = sqrt(b.dx * b.dx + b.dy * b.dy);
                        b.dx /= dlen; b.dy /= dlen;
                    }
                }
                b.x += b.dx * b.speed;
                b.y += b.dy * b.speed;
            }
            else {
                b.y += b.speed;
            }
        }

        // update alienBullets
        for (auto& b : game.alienBullets) {
            if (game.round > 5 && (b.dx != 0.0f || b.dy != 0.0f)) {
                b.x += b.dx * b.speed;
                b.y -= b.dy * b.speed;
            }
            else {
                b.y -= b.speed;
            }
        }


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
                float alienBulletSpeed = game.slowAlienBulletsActive ? 1.5f : 4.0f;
                int idx = rand() % aliveAliens.size();
                int ay = aliveAliens[idx].first;
                int ax = aliveAliens[idx].second;

                // axfter round 5, allow diagonal bullets
                float dx = 0.0f;
                float dy = 1.0f;
                if (game.round > 5) {
                    int dir = rand() % 3; // 0: left-diagonal, 1: straight, 2: right-diagonal
                    if (dir == 0) dx = -0.7f;
                    else if (dir == 2) dx = 0.7f;
                    // dy stays 1.0f
                    float norm = sqrt(dx * dx + dy * dy);
                    dx /= norm; dy /= norm; // normalize so speed stays consistent
                }
                game.alienBullets.push_back({
                    game.alienX + ax * 60,
                    game.alienY - ay * 40,
                    alienBulletSpeed,
                    dx, dy
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
                    if (ay - 15 < 60) // approaching player
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
    drawPowerups();

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