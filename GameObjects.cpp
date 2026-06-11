#include "GameObjects.h"

// początkowe wartości konfiguracyjne
GameConfig config = {0.01f, 15.0f, 45.0f, 8};

// stół do gry
Table::Table(float w, float h, float left, float top) {
    box.setSize(sf::Vector2f(w, h));
    box.setPosition(left, top);
    box.setFillColor(sf::Color(21, 88, 67));
    box.setOutlineColor(sf::Color(88, 57, 39));
    box.setOutlineThickness(6.0f);
}
void Table::update(float) {}
void Table::draw(sf::RenderWindow& window) { window.draw(box); }

// łuzy w rogach i na środku
Pocket::Pocket(float r, sf::Vector2f pos) : radius(r) {
    shape.setRadius(radius);
    shape.setOrigin(radius, radius);
    shape.setFillColor(sf::Color(15, 15, 15));
    shape.setOutlineColor(sf::Color(50, 30, 20));
    shape.setOutlineThickness(3.0f);
    shape.setPosition(pos);
}
void Pocket::update(float) {}
void Pocket::draw(sf::RenderWindow& window) { window.draw(shape); }
sf::Vector2f Pocket::getPosition() const { return shape.getPosition(); }
float Pocket::getRadius() const { return radius; }

// kule, pasek do widoczności rotacji
Ball::Ball(float r, sf::Vector2f pos, sf::Color c) : radius(r), rotationDeg(0.0f), color(c) {
    shape.setRadius(radius);
    shape.setOrigin(radius, radius);
    shape.setFillColor(color);
    shape.setPosition(pos);
    velocity = sf::Vector2f(0.f, 0.f);

    rotationLine.setSize(sf::Vector2f(radius, 3.0f));
    rotationLine.setFillColor(sf::Color(0, 0, 0, 150));
    rotationLine.setOrigin(0.0f, 1.5f);
}

// fizyka ruchu i tarcia dla pojedynczej kuli
void Ball::update(float dt) {
    if (std::abs(velocity.x) > config.stopVelocity)
        velocity.x -= sgn(velocity.x) * velocity.x * velocity.x * config.friction * dt;
    else velocity.x = 0.0f;

    if (std::abs(velocity.y) > config.stopVelocity)
        velocity.y -= sgn(velocity.y) * velocity.y * velocity.y * config.friction * dt;
    else velocity.y = 0.0f;

    shape.move(velocity.x * dt, velocity.y * dt);

    // przeliczenie prędkośći liniowej na rotację w stopniach
    float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    float angularVelocityDeg = (speed / radius) * (180.0f / 3.14159f);
    rotationDeg += angularVelocityDeg * dt;

    shape.setRotation(rotationDeg);
    rotationLine.setPosition(shape.getPosition());
    rotationLine.setRotation(rotationDeg);
}

void Ball::draw(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(rotationLine);
}
sf::Vector2f Ball::getPosition() const { return shape.getPosition(); }
void Ball::setPosition(sf::Vector2f p) { shape.setPosition(p); }
sf::Vector2f Ball::getVelocity() const { return velocity; }
void Ball::setVelocity(sf::Vector2f v) { velocity = v; }
float Ball::getRadius() const { return radius; }
sf::Color Ball::getColor() const { return color; }

Player::Player(std::string n) : name(n), score(0) {}