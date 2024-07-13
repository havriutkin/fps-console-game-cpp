#include <stdlib.h>
#include <Windows.h>
#include <string>
#include <vector>
#include <math.h>
#include <chrono>
#include <geomerty.cpp>


// 1 map square is MAP_WORLD_RATION worlds' "squares"
const int N_MAP_WORLD_RATIO = 5;
const int N_SCREEN_WORLD_RATION = 8;

class GameObject 
{
protected:
    Vector3D v3CenterWorldPos;           // Center of the object 
    std::vector<Vector3D> vv3Points;     // Points that represent that object in space;
    std::vector<float> fMapPos;          // Position in map coordinates
    WORD wPixelColor;
    float fSpeed;

    // Fills up vv3Points array
    virtual void calculateObjectPoints();

public:
    GameObject()
    {
        v3CenterWorldPos = Vector3D();
        vv3Points = {};
        fMapPos = {-1, -1};
        wPixelColor = 0; // BLACK;
        this->fSpeed = 0;
    }

    GameObject(Vector3D centerPos, WORD color, float speed)
    {
        v3CenterWorldPos = centerPos;
        vv3Points = {};
        fMapPos = {centerPos[0] / N_MAP_WORLD_RATIO, centerPos[1] / N_MAP_WORLD_RATIO};
        wPixelColor = color;
        this->fSpeed = speed;
        this->calculateObjectPoints();
    }

    Vector3D getCenterPos() { return this->v3CenterWorldPos; }


    // Returns distance to the closest point that belongs to object and lies on the line, 
    //     returns -1 if there is not points on the line
    virtual float getIntersectionDistance(Line line);

    void move(Vector3D direction, float dt)
    {
        direction.normalize();

        this->v3CenterWorldPos = this->v3CenterWorldPos + direction * this->fSpeed * dt;
        fMapPos = {this->v3CenterWorldPos[0] / N_MAP_WORLD_RATIO, this->v3CenterWorldPos[1] / N_MAP_WORLD_RATIO};

        this->calculateObjectPoints();
    }


};

class Cube: GameObject
{
private:

    void calculateObjectPoints()
    {
        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < 5; j++)
            {
                for (int k = 0; k < 5; k++)
                {
                    Vector3D v3Point;
                    v3Point[0] = this->v3CenterWorldPos[0] + i;
                    v3Point[1] = this->v3CenterWorldPos[1] + j;
                    v3Point[2] = this->v3CenterWorldPos[2] + k;
                    this->vv3Points.push_back(v3Point);
                }
            }
        }
    };
public:
    Cube(Vector3D centerPos, WORD color, float speed): GameObject(centerPos, color, speed) {};

    float getIntersectionDistance(Line line)
    {
        float result = INT_MAX;
        for (Vector3D point: vv3Points)
        {
            float distance = line.getParameter(point);
            if (distance == 0)
            {
                continue;
            }

            if (distance < result)
            {
                result = distance;
            }
        }
    }
};

class Player: GameObject
{
private:
    float fAngle;
    float fRotationSpeed;

    // Fills up vv3Points array
    virtual void calculateObjectPoints()
    {
        this->vv3Points = {};
    }

public:
    Player(): GameObject()
    {
        this->fAngle = 0.0;
        this->fRotationSpeed = 0.0f;
    }

    Player(Vector3D centerPos, float speed): GameObject(centerPos, 0, speed)
    {
        this->fAngle = 0.0;
        this->fRotationSpeed = 0.5f;
    }

    float getAngle() { return this->fAngle; }

    void rotateAngle(int dir, float dt) { this->fAngle += dir * this->fRotationSpeed * dt; }
};

class Game
{
private:
    // Game settings
    int nScreenWidth = 120;
    int nScreenHeight = 40;
    int nScreenWorldWidth = 15;
    int nScreenWorldHeight = 5;
    float fFocalLength = 5.0f;  // Distance from player to console screen in world

    int nMapWidth = 16;
    int nMapHeight = 16;

    float fDepth = 16.0f;

    Player player;

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
    }

    void update()
    {

    }

    void render()
    {
        for (int y = 0; y < this->nScreenHeight; y++) 
        {
            for (int x = 0; x < this->nScreenWidth; x++)
            {

                // Cast ray for every column
                float fRayAngle = (player.getAngle() - fFOV / 2.0) + ((float)x / (float)nScreenWidth) * fFOV;
                Line lRay(this->player.getCenterPos(), Vector3D())
                std::pair<float, float> pPlayerPos = player.getPos();
                std::pair<float, float> pRayVector = { sinf(fRay), cosf(fRay) };
            }            
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
            update();
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