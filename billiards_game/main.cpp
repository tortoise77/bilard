#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

struct Ball {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float radius;
};

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

int main() {

    // Stała tarcia toczenia: dv/dt = -(v^2) * friction_const
    const float friction_const = 0.01f;
    const float ball_stop_velocity = 5.0f;

    const unsigned int WINDOW_W = 1366;
    const unsigned int WINDOW_H = 768;
    sf::RenderWindow window(sf::VideoMode(WINDOW_W, WINDOW_H), "8 kul - symulacja kolizji 2");
    window.setFramerateLimit(120);

    const float BOX_W = 800.0f;
    const float BOX_H = 600.0f;
    const float BOX_LEFT = (WINDOW_W - BOX_W) / 2.0f;
    const float BOX_TOP  = (WINDOW_H - BOX_H) / 2.0f;

    sf::RectangleShape box(sf::Vector2f(BOX_W, BOX_H));
    box.setPosition(BOX_LEFT, BOX_TOP);
    box.setFillColor(sf::Color(139, 69, 19));
    box.setOutlineColor(sf::Color(90, 45, 12));
    box.setOutlineThickness(4.0f);

    const float WALL_LEFT   = BOX_LEFT;
    const float WALL_RIGHT  = BOX_LEFT + BOX_W;
    const float WALL_TOP    = BOX_TOP;
    const float WALL_BOTTOM = BOX_TOP + BOX_H;

    const int   NUM_BALLS = 8;
    const float DIAMETER  = 50.0f;
    const float RADIUS    = DIAMETER / 2.0f;

    const sf::Color colors[NUM_BALLS] = {
        sf::Color(231, 76, 60),
        sf::Color(46, 204, 113),
        sf::Color(52, 152, 219),
        sf::Color(241, 196, 15),
        sf::Color(155, 89, 182),
        sf::Color(26, 188, 156),
        sf::Color(230, 126, 34),
        sf::Color(236, 240, 241)
    };


    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    std::vector<Ball> balls;
    balls.reserve(NUM_BALLS);

    auto randf = [](float min, float max) {
        return min + static_cast<float>(std::rand()) / RAND_MAX * (max - min);
    };

    for (int i = 0; i < NUM_BALLS; ++i) {
        Ball b;
        b.radius = RADIUS;
        b.shape.setRadius(RADIUS);
        b.shape.setOrigin(RADIUS, RADIUS);
        b.shape.setFillColor(colors[i]);

        sf::Vector2f pos;
        bool ok = false;
        int attempts = 0;
        while (!ok && attempts < 1000) {
            pos.x = randf(WALL_LEFT + RADIUS, WALL_RIGHT - RADIUS);
            pos.y = randf(WALL_TOP + RADIUS, WALL_BOTTOM - RADIUS);
            ok = true;
            for (const auto& other : balls) {
                float dx = pos.x - other.shape.getPosition().x;
                float dy = pos.y - other.shape.getPosition().y;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < b.radius + other.radius + 2.0f) {
                    ok = false;
                    break;
                }
            }
            ++attempts;
        }
        b.shape.setPosition(pos);

        float angle = randf(0.0f, 2.0f * 3.14159265f);
        float speed = randf(150.0f, 350.0f);
        b.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

        balls.push_back(b);
    }

    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Escape)
                window.close();
        }

        float dt = clock.restart().asSeconds();
        //if (dt > 0.05f) dt = 0.05f;

        for (auto& b : balls) {
            if (abs(b.velocity.x)>ball_stop_velocity)
                b.velocity.x -= sgn(b.velocity.x)*b.velocity.x * b.velocity.x * friction_const * dt;
            else
                b.velocity.x = 0.0f;

            if (abs(b.velocity.y)>ball_stop_velocity)
                b.velocity.y -=  sgn(b.velocity.y) * b.velocity.y * b.velocity.y * friction_const * dt;
            else
                b.velocity.y = 0.0f;
        }

        for (auto& b : balls) {
            b.shape.move(b.velocity.x * dt, b.velocity.y * dt);
        }

        for (auto& b : balls) {
            sf::Vector2f p = b.shape.getPosition();
            if (p.x - b.radius < WALL_LEFT) {
                p.x = WALL_LEFT + b.radius;
                b.velocity.x = std::abs(b.velocity.x);
            } else if (p.x + b.radius > WALL_RIGHT) {
                p.x = WALL_RIGHT - b.radius;
                b.velocity.x = -std::abs(b.velocity.x);
            }
            if (p.y - b.radius < WALL_TOP) {
                p.y = WALL_TOP + b.radius;
                b.velocity.y = std::abs(b.velocity.y);
            } else if (p.y + b.radius > WALL_BOTTOM) {
                p.y = WALL_BOTTOM - b.radius;
                b.velocity.y = -std::abs(b.velocity.y);
            }
            b.shape.setPosition(p);
        }

        for (size_t i = 0; i < balls.size(); ++i) {
            for (size_t j = i + 1; j < balls.size(); ++j) {
                sf::Vector2f pi = balls[i].shape.getPosition();
                sf::Vector2f pj = balls[j].shape.getPosition();

                float dx = pj.x - pi.x;
                float dy = pj.y - pi.y;
                float dist = std::sqrt(dx * dx + dy * dy);
                float minDist = balls[i].radius + balls[j].radius;

                if (dist < minDist && dist > 0.0001f) {
                    // Wektor jednostkowy osi zderzenia (x') i styczny (y')
                    float nx = dx / dist;
                    float ny = dy / dist;

                    float overlap = (minDist - dist);
                    float shift = overlap / 2.0f;
                    pi.x -= nx * shift;  pi.y -= ny * shift;
                    pj.x += nx * shift;  pj.y += ny * shift;
                    balls[i].shape.setPosition(pi);
                    balls[j].shape.setPosition(pj);

                    vector<float> u1 = {balls[i].velocity.x, balls[i].velocity.y};
                    vector<float> u2 = {balls[j].velocity.x, balls[j].velocity.y};

                    balls[i].velocity.x = u2[0];
                    balls[i].velocity.y = u2[1];
                    balls[j].velocity.x = u1[0];
                    balls[j].velocity.y = u1[1];
                }
            }
        }

        window.clear(sf::Color(30, 30, 30));
        window.draw(box);
        for (auto& b : balls)
            window.draw(b.shape);
        window.display();
    }

    return 0;
}
