#include <stdlib.h>
#include <Windows.h>
#include <string>
#include <vector>
#include <math.h>
#include <chrono>

/*
TODO:
1. Map in the upper left corner
2. Health in the upper right
3. Picking up apples restores health
4. Zombie that run to you and deal damage
5. Shooting
*/

class Player
{
private:
    float fPosX;
    float fPosY;
    float fAngle;
    float fSpeed;
    float fRotationSpeed;
public:
    Player()
    {
        this->fPosX = 0.0;
        this->fPosY = 0.0;
        this->fAngle = 0.0;
        this->fSpeed = 0.0f;
        this->fRotationSpeed = 0.0f;
    }

    Player(float x, float y)
    {
        this->fPosX = x;
        this->fPosY = y;
        this->fAngle = 0.0;
        this->fSpeed = 0.8f;
        this->fRotationSpeed = 0.5f;
    }

    std::pair<float, float> getPos() { return {this->fPosX, this->fPosY}; }
    float getPosX() { return this->fPosX; }
    float getPosY() { return this->fPosY; }
    float getAngle() { return this->fAngle; }

    void setPos(std::pair<float, float> newPos) 
    {
        this->fPosX = newPos.first;
        this->fPosY = newPos.second;
    }

    void setPosX(float x) { this->fPosX = x; }
    void setPosY(float y) { this->fPosY = y; }
    void setAngle(float a) { this->fAngle = a; }
    void rotateAngle(int dir, float dt) { this->fAngle += dir * this->fRotationSpeed * dt; }
    void moveForward(float dt)
    {
        this->fPosX += sinf(this->fAngle) * this->fSpeed * dt;
        this->fPosY += cosf(this->fAngle) * this->fSpeed * dt;
    }

    void moveBackward(float dt)
    {
        this->fPosX -= sinf(this->fAngle) * this->fSpeed * dt;
        this->fPosY -= cosf(this->fAngle) * this->fSpeed * dt;
    }
};

class Bullet
{
private:
    float fPosX;
    float fPosY;
    float fAngle;
    float fSpeed = 1.6f;
public:
    Bullet()
    {
        this->fPosX = -1;
        this->fPosY = -1;
        this->fAngle = -1;
    }

    Bullet(float x, float y, float a)
    {
        this->fPosX = x;
        this->fPosY = y;
        this->fAngle = a;
    }

    void move(float dt)
    {
        this->fPosX += sinf(this->fAngle) * this->fSpeed * dt;
        this->fPosY += cosf(this->fAngle) * this->fSpeed * dt;
    }

    std::pair<float, float> getPos() { return {this->fPosX, this->fPosY}; }
    float getPosX() { return this->fPosX; }
    float getPosY() { return this->fPosY; }
    float getAngle() { return this->fAngle; }
};

class Game
{
private:
    // Game settings
    int nScreenWidth = 120;
    int nScreenHeight = 40;

    int nMapWidth = 16;
    int nMapHeight = 16;

    float fFOV = 3.14159f / 4.0f;
    float fDepth = 16.0f;
    float fCheckStep = 0.1;

    Player player;
    Bullet bullet;
    boolean bBullet = false;

    std::wstring map;
    float fElapsedTime = 0.0f;

    // Screen
    wchar_t* screen;
    WORD* attributes;
    HANDLE hConsole;
    WORD floorColor = FOREGROUND_RED;
    WORD wallColor = FOREGROUND_INTENSITY;

    void setUpConsole()
    {
        this->screen = new wchar_t[nScreenWidth * nScreenHeight];
        this->attributes = new WORD[nScreenWidth * nScreenHeight];
        this->hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
        SetConsoleActiveScreenBuffer(this->hConsole);

        // Hide cursor
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(this->hConsole, &cursorInfo);
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(this->hConsole, &cursorInfo);
    }

    void setUpMap()
    {
        this->map += L"################";
        this->map += L"#####..........#";
        this->map += L"#..............#";
        this->map += L"#..........##..#";
        this->map += L"#..........##..#";
        this->map += L"#...###........#";
        this->map += L"#...###........#";
        this->map += L"#...###........#";
        this->map += L"#..............#";
        this->map += L"#.......###....#";
        this->map += L"#..............#";
        this->map += L"#..............#";
        this->map += L"#......##......#";
        this->map += L"#......##......#";
        this->map += L"#..............#";
        this->map += L"################";
    }

    wchar_t getWallCharByDistance(float distance)
    {
        if (distance <= this->fDepth / 4.0f)	    { return 0x2588; }	// Close	
        else if (distance < this->fDepth / 3.0f)	{ return 0x2593; }
        else if (distance < this->fDepth / 2.0f)	{ return 0x2592; }
        else if (distance < this->fDepth)			{ return 0x2591; }

        return ' ';
    }

    wchar_t getFloorCharByDistance(float distance) 
    {
        if (distance < 0.25)      { return '#'; }
        else if (distance < 0.5)  { return 'x'; }
        else if (distance < 0.75) { return '.'; }
        else if (distance < 0.9)  { return '-'; }
        
        return ' ';
    }

    void handleInput()
    {
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000) 
        {
            player.rotateAngle(-1, this->fElapsedTime);
        }

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
        {
            player.rotateAngle(1, this->fElapsedTime);
        }

        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            player.moveForward(this->fElapsedTime);
            
            int newX = (int)player.getPosX();
            int newY = (int)player.getPosY();
            
            if (this->map[newX * nMapWidth + newY] == '#') 
            {
                player.moveBackward(this->fElapsedTime);
            }
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            player.moveBackward(this->fElapsedTime);

            int newX = (int)player.getPosX();
            int newY = (int)player.getPosY();
            
            if (this->map[newX * nMapWidth + newY] == '#') 
            {
                player.moveForward(this->fElapsedTime);
            }
        }
    
        if (GetAsyncKeyState((unsigned short)'F') & 0x8000)
        {
            this->bullet = Bullet(this->player.getPosX(), this->player.getPosY(), this->player.getAngle());
            this->bBullet = true;
        }
    }

    void renderRow(float fDistance, int x)
    {
        int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / fDistance;
        int nFloor = nScreenHeight - nCeiling;
        wchar_t wallChar = this->getWallCharByDistance(fDistance);    

        for (int y = 0; y < nScreenHeight; y++) 
        {
            if (y < nCeiling)
            {
                this->screen[y * nScreenWidth + x] = ' ';
                this->attributes[y * nScreenWidth + x] = 0; // Black
            }
            else if (nCeiling <= y && y <= nFloor) 
            {
                this->screen[y * nScreenWidth + x] = wallChar;
                this->attributes[y * nScreenWidth + x] = this->wallColor;
            }
            else 
            {
                float fFloorDistance = 1.0f - (((float)y -nScreenHeight/2.0f) / ((float)nScreenHeight / 2.0f));
                wchar_t floorChar = getFloorCharByDistance(fFloorDistance);

                this->screen[y * nScreenWidth + x] = floorChar;
                this->attributes[y * nScreenWidth + x] = this->floorColor;
            }
        }
    }

    void displayMap()
    {
        for (int x = 0; x < this->nMapWidth; x++) 
        {
            for (int y = 0; y < this->nMapHeight; y++)
            {
                this->screen[(y + 1) * this->nScreenWidth + x] = this->map[y * this->nMapWidth + x]; 
                this->attributes[(y + 1) * this->nScreenWidth + x] = FOREGROUND_GREEN;
            }
        }
        this->screen[(int)this->player.getPosX() * this->nScreenWidth + (int)this->player.getPosY()] = 'P';
        this->attributes[(int)this->player.getPosX() * this->nScreenWidth + (int)this->player.getPosY()] = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    }

    void displayStats() 
    {
        std::wstring stats = L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f";
        swprintf_s(screen, stats.size(), L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f", this->player.getPosX(), this->player.getPosY(),
                    this->player.getAngle(), 1.0f/this->fElapsedTime);
        for (int i = 0; i < stats.size(); i++) {
            this->attributes[i] = FOREGROUND_GREEN;
        }
    }

    void render()
    {
        for (int x = 0; x < this->nScreenWidth; x++) 
        {
            // Cast ray for every column
            float fRay = (player.getAngle() - fFOV / 2.0) + ((float)x / (float)nScreenWidth) * fFOV;
            std::pair<float, float> pPlayerPos = player.getPos();
            std::pair<float, float> pRayVector = { sinf(fRay), cosf(fRay) };
            float fDistance = 0.0f;
            boolean bHitWall = false;

            // Find distance to wall in that direction
            while (!bHitWall && fDistance < fDepth) 
            {   
                fDistance += this->fCheckStep;
                
                // Cast to int to get int map coordinates
                std::pair<int, int> pMapCoord = 
                {
                    (int)(pPlayerPos.first + pRayVector.first * fDistance),
                    (int)(pPlayerPos.second + pRayVector.second * fDistance)
                };

                // Check if wall was hit
                if (0 <= pMapCoord.first && pMapCoord.first < nMapWidth && 0 <= pMapCoord.second && pMapCoord.second < nMapHeight)
                {
                    if (map[pMapCoord.first * nMapWidth + pMapCoord.second] == '#' || map[pMapCoord.first * nMapWidth + pMapCoord.second] == 'B') 
                    {
                        bHitWall = true;
                    }
                } 
                else 
                {
                    bHitWall = true;
                    fDistance = fDepth;
                }
            }

            this->renderRow(fDistance, x);
        }

        this->displayMap();
        this->displayStats();
    }

public:
    Game()
    {
        this->setUpConsole();
        this->setUpMap();
        this->player = Player(this->nMapWidth / 2.0, this->nMapHeight / 2.0);
    }

    ~Game()
    {
        delete this->screen;
        delete this->attributes;
    }

    void start()
    {
        auto time1 = std::chrono::system_clock::now();
        auto time2 = std::chrono::system_clock::now();

        while (true)
        {
            time2 = std::chrono::system_clock::now();
            std::chrono::duration<float> elapsedTime = time2 - time1;
            time1 = time2;
            this->fElapsedTime = elapsedTime.count();

            handleInput();

            if (this->bBullet) 
            {
                this->bullet.move(this->fElapsedTime);
                this->map[(int)this->bullet.getPosX() * this->nScreenWidth + (int)this->bullet.getPosY()] = 'B';
                this->attributes[(int)this->bullet.getPosX() * this->nScreenWidth + (int)this->bullet.getPosY()] = FOREGROUND_RED | FOREGROUND_BLUE;
            }

            render();

            // Draw
            DWORD dwBytesWritten;
            screen[nScreenWidth * nScreenHeight - 1] = '\0';
            WriteConsoleOutputCharacterW(this->hConsole, screen, nScreenWidth * nScreenHeight, {0, 0}, &dwBytesWritten);
            WriteConsoleOutputAttribute(this->hConsole, attributes, nScreenWidth * nScreenHeight, {0, 0}, &dwBytesWritten);
        }

    }
};

int main() 
{
    Game myGame;
    myGame.start();

    return 0;
}