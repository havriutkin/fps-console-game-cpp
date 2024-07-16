#include <stdlib.h>
#include <Windows.h>
#include <string>
#include <vector>
#include <math.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include "geometry.h"


// 1 map square is MAP_WORLD_RATION worlds' "squares"
const int N_MAP_WORLD_RATIO = 5;
const int N_SCREEN_WORLD_RATIO = 10;


// Dev
std::ofstream logFile("log.txt");

class GameObject 
{
protected:
    Vector3D v3CenterWorldPos;           // Center of the object 
    std::vector<float> fMapPos;          // Position in map coordinates
    WORD wChar;
    WORD wPixelColor;
    float fSpeed;

public:
    GameObject()
    {
        v3CenterWorldPos = Vector3D();
        fMapPos = {-1, -1};
        wChar = ' ';
        wPixelColor = 0; // BLACK;
        this->fSpeed = 0;
    }

    GameObject(Vector3D centerPos, WORD objectChar, WORD color, float speed)
    {
        v3CenterWorldPos = centerPos;
        fMapPos = {centerPos[0] / N_MAP_WORLD_RATIO, centerPos[1] / N_MAP_WORLD_RATIO};
        wChar = objectChar;
        wPixelColor = color;
        this->fSpeed = speed;
    }

    Vector3D getCenterPos() const { return this->v3CenterWorldPos; }

    WORD getChar() const { return this->wChar; }

    WORD getPixelColor() const { return this->wPixelColor; }

    void move(Vector3D direction, float dt)
    {
        direction.normalize();

        this->v3CenterWorldPos = this->v3CenterWorldPos + direction * this->fSpeed * dt;
        fMapPos = {this->v3CenterWorldPos[0] / N_MAP_WORLD_RATIO, this->v3CenterWorldPos[1] / N_MAP_WORLD_RATIO};
    }


    // Returns distance to the closest point that belongs to object and lies on the line, 
    //     returns -1 if there is not points on the line
    virtual std::pair<float, WORD> getIntersection(Line line, float depth) = 0;

    virtual WORD getCharByDistance(float depth, float distance) = 0;
};

class Cube: public GameObject
{
private:
    float size;
    std::vector<Plane*> planes;  // Planes define cube
    float fMinX, fMaxX;
    float fMinY, fMaxY;
    float fMinZ, fMaxZ;

public:
    Cube(Vector3D centerPos, WORD objectChar, WORD color, float speed): GameObject(centerPos, objectChar, color, speed) 
    {
        this->size = 2.5f;

        this->fMinX = centerPos[0] - size < centerPos[0] + size ? centerPos[0] - size : centerPos[0] + size;
        this->fMaxX = centerPos[0] - size > centerPos[0] + size ? centerPos[0] - size : centerPos[0] + size;
        this->fMinY = centerPos[1] - size < centerPos[1] + size ? centerPos[1] - size : centerPos[1] + size;
        this->fMaxY = centerPos[1] - size > centerPos[1] + size ? centerPos[1] - size : centerPos[1] + size;
        this->fMinZ = centerPos[2] - size < centerPos[2] + size ? centerPos[2] - size : centerPos[2] + size;
        this->fMaxZ = centerPos[2] - size > centerPos[2] + size ? centerPos[2] - size : centerPos[2] + size;

        planes.push_back(new Plane(Vector3D(centerPos[0] - size, centerPos[1], centerPos[2]), Vector3D(-1, 0, 0))); // Left
        planes.push_back(new Plane(Vector3D(centerPos[0] + size, centerPos[1], centerPos[2]), Vector3D(1, 0, 0)));  // Right
        planes.push_back(new Plane(Vector3D(centerPos[0], centerPos[1] - size, centerPos[2]), Vector3D(0, -1, 0))); // Bottom
        planes.push_back(new Plane(Vector3D(centerPos[0], centerPos[1] + size, centerPos[2]), Vector3D(0, 1, 0)));  // Top
        planes.push_back(new Plane(Vector3D(centerPos[0], centerPos[1], centerPos[2] - size), Vector3D(0, 0, -1))); // Front
        planes.push_back(new Plane(Vector3D(centerPos[0], centerPos[1], centerPos[2] + size), Vector3D(0, 0, 1)));  // Back
    };

    WORD getCharByDistance(float depth, float distance)
    {
        if (distance <= depth / 2.0f)	    { return 0x2588; }	// Close	
        else if (distance < depth / 1.75f)	{ return 0x2593; }
        else if (distance < depth / 1.5f)	{ return 0x2592; }
        else if (distance < depth)	{ return 0x2591; }
        return ' ';
    }

    std::pair<float, WORD> getIntersection(Line line, float depth = 16.0f)
    {
        float result = INT_MAX;
        WORD resChar = ' ';
        for(const Plane* plane: this->planes)
        {
            std::pair<Vector3D, float> localRes = plane->getLineIntersection(line);
            Vector3D point = localRes.first;
            float localDistance = localRes.second;

            // Check bounds
            if (point[0] < this->fMinX || point[0] > this->fMaxX)
            {
                continue;
            }

            if (point[1] < this->fMinY || point[1] > this->fMaxY)
            {
                continue;
            }

            if (point[2] < this->fMinZ || point[2] > this->fMaxZ)
            {
                continue;
            }
            
            if (localDistance > 0 && localDistance < result)
            {       
                resChar = this->getCharByDistance(depth, localDistance);
                result = localDistance;
            }
        }
        return { sqrtf(result), resChar };
    }
    
};

class Floor: public GameObject
{
private:
    Plane plane;

public:
    Floor(WORD color): GameObject(Vector3D(0, 0, 0), 'x', color, 0.0f) 
    {
        this->plane = Plane(Vector3D(0, 0, 0.0f), Vector3D(0, 0, 1.0f));
    };

    WORD getCharByDistance(float depth, float distance)
    {
        if (distance <= 4.0f)      { return '#'; }
        else if (distance < 8.0f)  { return 'x'; }
        else if (distance < 12.0f) { return '-'; }
        else if (distance < 16.0f)  { return '.'; }
        
        return '.';
    }

    std::pair<float, WORD> getIntersection(Line line, float depth = 16.0f)
    {
        WORD resChar = ' ';
        std::pair<Vector3D, float> localRes = this->plane.getLineIntersection(line);
        resChar = this->getCharByDistance(16.0f, localRes.second);
        return { sqrtf(localRes.second), resChar };
    }
};

class Ceiling: public GameObject
{
private:
    Plane plane;

public:
    Ceiling(WORD color): GameObject(Vector3D(0, 0, 0), 'x', color, 0.0f) 
    {
        this->plane = Plane(Vector3D(0, 0, 5.0f), Vector3D(0, 0, 1.0f));
    };

    WORD getCharByDistance(float depth, float distance)
    {
        if (distance <= 2.0f)      { return '#'; }
        else if (distance < 4.0f)  { return 'x'; }
        else if (distance < 6.0f) { return '-'; }
        else if (distance < 8.0f)  { return '.'; }
        
        return '.';
    }

    std::pair<float, WORD> getIntersection(Line line, float depth = 16.0f)
    {
        WORD resChar = ' ';
        std::pair<Vector3D, float> localRes = this->plane.getLineIntersection(line);
        resChar = this->getCharByDistance(16.0f, localRes.second);
        return { sqrtf(localRes.second), resChar };
    }
};

class Player: public GameObject
{
private:
    float fAngle;
    float fRotationSpeed;

public:
    Player(): GameObject()
    {
        this->fAngle = 0.0;
        this->fRotationSpeed = 0.0f;
    }

    Player(Vector3D centerPos, float speed): GameObject(centerPos, ' ', 0, speed)
    {
        this->fAngle = 0.0;
        this->fRotationSpeed = 0.5f;
    }

    float getAngle() { return this->fAngle; }

    void rotateAngle(int dir, float dt) { this->fAngle += dir * this->fRotationSpeed * dt; }

    std::pair<float, WORD> getIntersection(Line line, float depth = 16.0f)
    {
        return { -1, ' ' };
    }

    WORD getCharByDistance(float depth, float distance)
    {
        return ' ';
    }
};

class Game
{
private:
    // Game settings
    int nScreenWidth = 120;
    int nScreenHeight = 40;
    int nScreenWorldWidth = 12;
    int nScreenWorldHeight = 4;
    float fFocalLength = 1.0f;  // Distance from player to console screen in world

    int nMapWidth = 16;
    int nMapHeight = 16;

    float fDepth = 60.0f;

    Player player;
    std::vector<GameObject*> objects;

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
        this->map += L"#..............#";
        this->map += L"#..............#";
        this->map += L"#..............#";
        this->map += L"#..............#";
        this->map += L"#..............#";
        this->map += L"#..............#";
        this->map += L"#........##....#";
        this->map += L"#..............#";
        this->map += L"#..............#";
        this->map += L"#..............#";
        this->map += L"#..............#";
        this->map += L"#..............#";
        this->map += L"#..............#";
        this->map += L"#..............#";
        this->map += L"################";
    }

    void buildWorldFromMap()
    {   
        for (int y = 0; y < this->nMapHeight; y++)
        {
            for (int x = 0; x < this->nMapWidth; x++)
            {
                wchar_t currChar = map[y * nMapWidth + x];
                if (currChar == '#')
                {
                    Vector3D centerPos(x * N_MAP_WORLD_RATIO, y * N_MAP_WORLD_RATIO, 2.5f);
                    Cube* newCube = new Cube(centerPos, '#', FOREGROUND_BLUE, 0.0f);
                    this->objects.push_back(newCube);
                }
            }
        }
        this->objects.push_back(new Floor(FOREGROUND_RED));     // Floor
        this->objects.push_back(new Ceiling(FOREGROUND_RED | FOREGROUND_GREEN));
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

        Vector3D playerPos = this->player.getCenterPos();
        Vector3D playerPosOnMap(playerPos[0] / 5, playerPos[1] / 5, 0);
        this->screen[(int)playerPosOnMap[1] * this->nScreenWidth + (int)playerPosOnMap[0]] = 'P';
        this->attributes[(int)playerPosOnMap[1] * this->nScreenWidth + (int)playerPosOnMap[0]] = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    }

    void displayStats() 
    {
        std::wstring stats = L"X=%3.2f, Y=%3.2f, Z=%3.2f A=%3.2f FPS=%3.2f";
        Vector3D playerPos = this->player.getCenterPos();
        swprintf_s(screen, stats.size(), L"X=%3.2f, Y=%3.2f, Z=%3.2f, A=%3.2f FPS=%3.2f", playerPos[0], playerPos[1], playerPos[2], 
            player.getAngle(), 1.0 / this->fElapsedTime);

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
            Vector3D direction(cosf(player.getAngle()), sinf(player.getAngle()), 0.0f);
            player.move(direction, this->fElapsedTime);
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            Vector3D direction(cosf(player.getAngle()), sinf(player.getAngle()), 0.0f);
            player.move(direction * -1, this->fElapsedTime);
        }

        if (GetAsyncKeyState((unsigned short)'U') & 0x8000)
        {
            Vector3D direction(0, 0, 1);
            player.move(direction, this->fElapsedTime);
        }

        if (GetAsyncKeyState((unsigned short)'J') & 0x8000)
        {
            Vector3D direction(0, 0, -1);
            player.move(direction, this->fElapsedTime);
        }
    }

    void update()
    {
        return;
    }

    void render()
    {
        Vector3D playerPos = this->player.getCenterPos();
        float playerAngle = this->player.getAngle();

        // Aspect ratio and FOV
        float aspectRatio = (float)this->nScreenWidth / (float)this->nScreenHeight;
        float fov = 3.14159 / 4.0; // 45 degree field of view

        for (int y = 0; y < this->nScreenHeight; y++) 
        {
            for (int x = 0; x < this->nScreenWidth; x++)
            {
                // Calculate normalized device coordinates
                float ndcX = (2.0f * x / (float)this->nScreenWidth - 1.0f) * aspectRatio;
                float ndcY = 1.0f - 2.0f * y / (float)this->nScreenHeight;

                // Calculate the direction of the ray in camera space
                float rayDirX = 1.0f; // Looking along the x-axis initially
                float rayDirY = ndcX * tanf(fov / 2.0f);
                float rayDirZ = ndcY * tanf(fov / 2.0f);

                // Rotate the ray direction to align with the player's view direction
                Vector3D rayDirection = Vector3D(
                    cosf(playerAngle) * rayDirX - sinf(playerAngle) * rayDirY,
                    sinf(playerAngle) * rayDirX + cosf(playerAngle) * rayDirY,
                    rayDirZ
                );
                Line lRay(playerPos, rayDirection);

                // Find nearest seen object
                float fDistance = this->fDepth; // Init with depth limit
                WORD wCharToUse = ' ';
                WORD wColorToUse = 0;
                for(GameObject* obj: this->objects)
                {
                    std::pair<float, WORD> intersection = obj->getIntersection(lRay, this->fDepth);
                    
                    if (intersection.first > 0 && intersection.first < fDistance) 
                    {
                        fDistance = intersection.first;
                        wColorToUse = obj->getPixelColor();
                        wCharToUse = intersection.second;
                    }
                }

                this->screen[y * this->nScreenWidth + x] = wCharToUse;
                this->attributes[y * this->nScreenWidth + x] = wColorToUse;
            }            
        }

        this->displayMap();
        this->displayStats();
        //exit(1);
    }

public:
    Game()
    {
        this->setUpConsole();
        this->setUpMap();
        this->buildWorldFromMap();
        this->player = Player(Vector3D(40, 40, this->nScreenHeight / (2 * (float)N_SCREEN_WORLD_RATIO)), 2.0f);
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