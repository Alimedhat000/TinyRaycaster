#include <SFML/Graphics.hpp>
#include <iostream>
#include <math.h>
constexpr float PI = 3.141592f;
constexpr float DEGREE = 0.0174533f; //One Degree in rad
constexpr int WINDOW_HIEGHT = 512;
constexpr int WINDOW_WIDTH = 1024;
float PlayerX = 64, PlayerY = 64, PlayerDirX, PlayerDirY, PlayerAngle; //Player Coordinates, Direction vectors of the Player in X and Y, and player's angle
sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HIEGHT), "TinyRayCaster");
sf::RectangleShape player(sf::Vector2f(8.f, 8.f));
sf::RectangleShape thickLine;
float lineThickness = 5.0f; // Set thickness here
int MapWidth = 8, MapHeight = 8; // Number of tiles
float MapTileSize = 64.f; // Number of pixels in a tile 
int map[] = {
    1,1,1,1,1,1,1,1,
    1,0,1,0,0,0,0,1,
    1,0,1,0,0,1,0,1,
    1,0,0,0,0,0,0,1,
    1,0,0,0,0,1,0,1,
    1,0,0,0,0,1,0,1,
    1,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,
};
void drawRays();
void draw3DWall(float Distance, int Rayindex, sf::Color);
void drawLine(const sf::Vector2f& start, const sf::Vector2f& end, float thickness, sf::Color color);
float distance(sf::Vector2f Points);
void drawPov();
void drawPlayer();
void drawMap2D();
void draw();
void handleKeyInput();
float distance(sf::Vector2f Points) {
    return static_cast<float>(sqrt(pow((Points.x - PlayerX), 2) + pow((Points.y - PlayerY), 2)));
}
void drawRays() {
    int RayIndex, MapX, MapY, MapIndex, DepthOfField;
    float RayX, RayY, RayAngle, StepX, StepY;

    RayAngle = PlayerAngle - DEGREE * 45;
    if (RayAngle < 0.f)
        RayAngle += 2 * PI;
    if (RayAngle > 2 * PI)
        RayAngle -= 2 * PI;

    for (RayIndex = 0; RayIndex < 90; RayIndex++) { // For now, just one ray
        // Horizontal ray-casting check
        float HorizontalRayX{ PlayerX }, HorizontalRayY{ PlayerY }, HorizontalDistance{ 1000000000 };
        float aTan = -1 / tan(RayAngle);
        DepthOfField = 0;

        // Determine starting positions and steps based on ray angle
        if (RayAngle > 0 && RayAngle < PI) { // Looking up
            RayY = static_cast<float>((static_cast<int>(PlayerY) / 64) * 64) - 0.0001f;
            RayX = PlayerX + ((PlayerY - RayY) * aTan);
            StepY = -64;
            StepX = -StepY * aTan;
        }
        else if (RayAngle > PI && RayAngle < 2 * PI) { // Looking down
            RayY = static_cast<float>((static_cast<int>(PlayerY) / 64) * 64) + 64.f;
            RayX = PlayerX + ((PlayerY - RayY) * aTan);
            StepY = 64;
            StepX = -StepY * aTan;
        }
        else { // Ray is looking directly right or left
            RayX = PlayerX;
            RayY = PlayerY;
            DepthOfField = 8; // Don't check further
        }

        // Horizontal ray casting loop
        while (DepthOfField < 8) {
            MapX = static_cast<int>(RayX) >> 6;
            MapY = static_cast<int>(RayY) >> 6;
            MapIndex = MapY * MapWidth + MapX;

            if (MapX < 0 || MapX >= MapWidth || MapY < 0 || MapY >= MapHeight) {
                DepthOfField = 8; // Stop if ray goes out of bounds
            }
            else if (map[MapIndex] == 1) { // Wall hit
                HorizontalRayX = RayX;
                HorizontalRayY = RayY;
                HorizontalDistance = distance(sf::Vector2f(HorizontalRayX, HorizontalRayY));
                DepthOfField = 8; // Stop searching after hitting a wall
            }
            else {
                RayX += StepX;
                RayY += StepY;
                DepthOfField += 1;
            }
        }

        // Reset for vertical ray-casting check
        float VerticalRayX{ PlayerX }, VerticalRayY{ PlayerY }, VerticalDistance{ 1000000000 };
        DepthOfField = 0;
        float nTan = -1 * tan(RayAngle);

        // Determine starting positions and steps based on ray angle
        if (RayAngle < PI / 2 || RayAngle > 3 * PI / 2) { // Looking right
            RayX = static_cast<float>((static_cast<int>(PlayerX) / 64) * 64) - 0.0001f;
            RayY = PlayerY + ((PlayerX - RayX) * nTan);
            StepX = -64;
            StepY = -StepX * nTan;
        }
        else if (RayAngle > PI / 2 && RayAngle < 3 * PI / 2) { // Looking left
            RayX = static_cast<float>((static_cast<int>(PlayerX) / 64) * 64) + 64.f;
            RayY = PlayerY + ((PlayerX - RayX) * nTan);
            StepX = 64;
            StepY = -StepX * nTan;
        }
        else { // Ray is looking directly up or down
            RayX = PlayerX;
            RayY = PlayerY;
            DepthOfField = 8; // Don't check further
        }

        // Vertical ray casting loop
        while (DepthOfField < 8) {
            MapX = static_cast<int>(RayX) >> 6;
            MapY = static_cast<int>(RayY) >> 6;
            MapIndex = MapY * MapWidth + MapX;

            if (MapX < 0 || MapX >= MapWidth || MapY < 0 || MapY >= MapHeight) {
                DepthOfField = 8; // Stop if ray goes out of bounds
            }
            else if (map[MapIndex] == 1) { // Wall hit
                VerticalRayX = RayX;
                VerticalRayY = RayY;
                VerticalDistance = distance(sf::Vector2f(VerticalRayX, VerticalRayY));
                DepthOfField = 8; // Stop searching after hitting a wall
            }
            else {
                RayX += StepX;
                RayY += StepY;
                DepthOfField += 1;
            }
        }

        // Determine which ray hit first and draw the line
        float FisheyeFix = PlayerAngle - RayAngle;
        if (FisheyeFix > 2 * PI)
            FisheyeFix -= 2 * PI;
        if (FisheyeFix < 0)
            FisheyeFix += 2 * PI;

        if (HorizontalDistance < VerticalDistance) {
            drawLine(sf::Vector2f(PlayerX, PlayerY), sf::Vector2f(HorizontalRayX, HorizontalRayY), 2.f, sf::Color::Magenta);
            draw3DWall(HorizontalDistance * cos(FisheyeFix), RayIndex, sf::Color(255, 255, 255, 255));
        }
        else {
            drawLine(sf::Vector2f(PlayerX, PlayerY), sf::Vector2f(VerticalRayX, VerticalRayY), 2.f, sf::Color::Magenta);
            draw3DWall(VerticalDistance * cos(FisheyeFix), RayIndex, sf::Color(255, 255, 255, 150));
        }
        RayAngle += DEGREE;
        if (RayAngle < 0)
            RayAngle += 2 * PI;
        if (RayAngle > 2 * PI)
            RayAngle -= 2 * PI;
    }
}
void draw3DWall(float Distance, int RayIndex, sf::Color color) {
    // Calculate the height of the wall line based on distance
    float LineHeight = (MapTileSize * 320) / Distance;

    // Clamp the line height to the window height
    LineHeight = (LineHeight > 320) ? 320 : LineHeight;

    // Calculate the width of the wall line based on the number of rays
    //float wallWidth = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(60);

    // Create the line shape
    sf::RectangleShape Line(sf::Vector2f(5, LineHeight));

    // Position the line in the center of the screen
    Line.setPosition(sf::Vector2f(RayIndex * 5.f + 530.f, 160 - LineHeight / 2.f));

    // Set color for the line
    Line.setFillColor(color);

    // Draw the line on the window
    window.draw(Line);
}
void drawLine(const sf::Vector2f& start, const sf::Vector2f& end, float thickness, sf::Color color) {
    // Calculate the distance (length) between start and end points
    float distance = sqrt((end.x - start.x) * (end.x - start.x) + (end.y - start.y) * (end.y - start.y));

    // Create a rectangle to represent the line
    sf::RectangleShape line(sf::Vector2f(distance, thickness));

    // Set the origin of the line to be the middle of its thickness
    line.setOrigin(0, thickness / 2);

    // Set the position of the line at the start point
    line.setPosition(start);

    // Calculate the angle between the two points
    float angle = atan2(end.y - start.y, end.x - start.x) * 180 / PI;

    // Rotate the rectangle (line) to match the angle between the points
    line.setRotation(angle);

    // Set the color of the line
    line.setFillColor(color);

    // Draw the line on the window
    window.draw(line);
}
void drawPov() {
    // Calculate direction vector
    sf::Vector2f direction(PlayerDirX * 10, PlayerDirY * 10);

    // Set size of the rectangle as length of the line and thickness
    thickLine.setSize(sf::Vector2f(sqrt(direction.x * direction.x + direction.y * direction.y), lineThickness));
    thickLine.setOrigin(sf::Vector2f(1 * sqrt(direction.x * direction.x + direction.y * direction.y), lineThickness / 2.f));

    // Set the position at the player's coordinates
    thickLine.setPosition(PlayerX, PlayerY);

    // Rotate the rectangle to match the angle of the line
    thickLine.setRotation(atan2(PlayerDirY, PlayerDirX) * 180 / PI);

    thickLine.setFillColor(sf::Color::Red);
    window.draw(thickLine);
}
void drawPlayer() {
    player.setFillColor(sf::Color::Green);
    player.setPosition(sf::Vector2f(PlayerX, PlayerY)); //Change The Position.
    player.setRotation(atan2(PlayerDirY, PlayerDirX) * 180 / PI); // Rotating the Player by and angle of Tan-1(PlayerDirX/PlayerDirY) and converting it to Degrees.
    drawRays();
    drawPov();
    window.draw(player);
}
void drawMap2D() {
    sf::RectangleShape block(sf::Vector2f(MapTileSize - 1, MapTileSize - 1)); // Correct block size
    for (int y = 0; y < MapHeight; y++) {
        for (int x = 0; x < MapWidth; x++) {
            if (map[y * MapWidth + x] == 1) {
                block.setFillColor(sf::Color(255, 255, 255, 255));
            }
            else {
                block.setFillColor(sf::Color(0, 0, 0, 255));
            }
            block.setPosition(sf::Vector2f(x * MapTileSize, y * MapTileSize));
            window.draw(block); // Correct this line
        }
    }
}
void draw() {
    window.clear(sf::Color(41, 41, 41, 255));
    drawMap2D();
    drawPlayer();
    window.display();
}
void handleKeyInput() {
    const float playerSpeed = 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) && PlayerX > 0) {
        PlayerAngle -= 0.01f;
        if (PlayerAngle <= 0)
        {
            PlayerAngle += 2 * PI;

        }
        PlayerDirX = cos(PlayerAngle) * 2;
        PlayerDirY = sin(PlayerAngle) * 2;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) && PlayerX < 1024) {
        PlayerAngle += 0.01f;
        if (PlayerAngle >= 2 * PI)
        {
            PlayerAngle -= 2 * PI;

        }
        PlayerDirX = cos(PlayerAngle) * 2;
        PlayerDirY = sin(PlayerAngle) * 2;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        float length = sqrt(PlayerDirX * PlayerDirX + PlayerDirY * PlayerDirY);
        PlayerX += (PlayerDirX / length) * -playerSpeed;
        PlayerY += (PlayerDirY / length) * -playerSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        float length = sqrt(PlayerDirX * PlayerDirX + PlayerDirY * PlayerDirY);
        PlayerX += (PlayerDirX / length) * playerSpeed;
        PlayerY += (PlayerDirY / length) * playerSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
    {
        window.close();
    }
}
int main() {
    player.setOrigin(sf::Vector2f(4.f, 4.f));
    PlayerAngle = PI;
    PlayerDirX = cos(PlayerAngle) * 2;
    PlayerDirY = sin(PlayerAngle) * 2;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        handleKeyInput();
        draw();
    }
    return 0;
}
