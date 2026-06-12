#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>

// parametryzacja gry
struct GameConfig {
    float friction;
    float stopVelocity;
    float cueManipSpeed;
    int numBalls;
};

extern GameConfig config;

// pomocnicza funkcja znaku do fizyki
template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

// główna klasa bazowa - od niej wszystko dziedziczy
class GameObject {
public:
    virtual ~GameObject() = default;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
};

class Table : public GameObject {
private:
    sf::RectangleShape box;
public:
    Table(float w, float h, float left, float top);
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
};

class Pocket : public GameObject {
private:
    sf::CircleShape shape;
    float radius;
public:
    Pocket(float r, sf::Vector2f pos);
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    sf::Vector2f getPosition() const;
    float getRadius() const;
};

class Ball : public GameObject {
private:
    sf::CircleShape shape;
    sf::RectangleShape rotationLine;
    sf::Vector2f velocity;
    float radius;
    float rotationDeg;
    sf::Color color;

public:
    Ball(float r, sf::Vector2f pos, sf::Color c);
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    sf::Vector2f getPosition() const;
    void setPosition(sf::Vector2f p);
    sf::Vector2f getVelocity() const;
    void setVelocity(sf::Vector2f v);
    float getRadius() const;
    sf::Color getColor() const;
};

// klasa do trzymania informacji o graczach
class Player {
public:
    std::string name;
    int score;
    Player(std::string n);
};