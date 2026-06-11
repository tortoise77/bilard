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

// łuza
struct Pocket {
    sf::CircleShape shape;
    float radius;
};

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

int main() {

    // Stała tarcia toczenia: dv/dt = -(v^2) * friction_const
    const float friction_const = 0.01f;
    const float ball_stop_velocity = 15.0f;

    const unsigned int WINDOW_W = 1366;
    const unsigned int WINDOW_H = 768;
    sf::RenderWindow window(sf::VideoMode(WINDOW_W, WINDOW_H), "8 kul - symulacja kolizji 2");
    window.setFramerateLimit(120);

    const float BOX_W = 800.0f;
    const float BOX_H = 400.0f;
    const float BOX_LEFT = (WINDOW_W - BOX_W) / 2.0f;
    const float BOX_TOP  = (WINDOW_H - BOX_H) / 2.0f;

    sf::RectangleShape box(sf::Vector2f(BOX_W, BOX_H));
    box.setPosition(BOX_LEFT, BOX_TOP);
    box.setFillColor(sf::Color(21, 88, 67));
    box.setOutlineColor(sf::Color(88, 57, 39));
    box.setOutlineThickness(6.0f);

    const float WALL_LEFT   = BOX_LEFT;
    const float WALL_RIGHT  = BOX_LEFT + BOX_W;
    const float WALL_TOP    = BOX_TOP;
    const float WALL_BOTTOM = BOX_TOP + BOX_H;

    const int   NUM_BALLS = 8;
    const float DIAMETER  = 50.0f;
    const float RADIUS    = DIAMETER / 2.0f;

    // promień łuzy ustawiony na 40.0f, aby środek kuli mógł przekroczyć krawędź w rogu stołu
    const float POCKET_RADIUS = 40.0f;

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

    // inicjalizacja wektora łuz w standardowych 6 punktach stołu bilardowego
    std::vector<Pocket> pockets;
    std::vector<sf::Vector2f> pocketPositions = {
        {WALL_LEFT, WALL_TOP},                          // lewa góra
        {(WALL_LEFT + WALL_RIGHT) / 2.0f, WALL_TOP},    // środek góra
        {WALL_RIGHT, WALL_TOP},                         // prawa góra
        {WALL_LEFT, WALL_BOTTOM},                       // lewy dół
        {(WALL_LEFT + WALL_RIGHT) / 2.0f, WALL_BOTTOM}, // środek dół
        {WALL_RIGHT, WALL_BOTTOM}                       // prawy dół
    };

    for (const auto& pos : pocketPositions) {
        Pocket p;
        p.radius = POCKET_RADIUS;
        p.shape.setRadius(POCKET_RADIUS);
        p.shape.setOrigin(POCKET_RADIUS, POCKET_RADIUS);
        p.shape.setFillColor(sf::Color(15, 15, 15));
        p.shape.setOutlineColor(sf::Color(50, 30, 20)); // obwód
        p.shape.setOutlineThickness(3.0f);
        p.shape.setPosition(pos);
        pockets.push_back(p);
    }

    std::vector<Ball> balls;
    balls.reserve(NUM_BALLS);

    auto randf = [](float min, float max) {
        return min + static_cast<float>(std::rand()) / RAND_MAX * (max - min);
    };

    const float CUE_LENGTH    = 320.0f;
    const float CUE_THICKNESS = 8.0f;
    const float CUE_MANIP_SPEED = 45.0f; // deg/s przy trzymaniu A lub S

    // Podstawa kija (jej X przesuwa się A/S, Y stała – np. środek stołu)
    sf::Vector2f cueBase((WALL_LEFT + WALL_RIGHT) / 2.0f,
                         WALL_BOTTOM);

    float cueAngleDeg = -90.f;

    // Graficzny prostokąt kija: origin na lewym końcu (= podstawa)
    sf::RectangleShape cueShape(sf::Vector2f(CUE_LENGTH, CUE_THICKNESS));
    cueShape.setFillColor(sf::Color(160, 82, 45));   // brązowy kij
    cueShape.setOrigin(0.0f, CUE_THICKNESS / 2.0f);

    bool cueActive = true; // kij widoczny/aktywny gdy wszystkie kule stoją

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

        b.velocity = sf::Vector2f(0.f, 0.f);

        balls.push_back(b);
    }

    sf::Clock clock;

    while (window.isOpen()) {

        sf::Vector2i mp = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos(static_cast<float>(mp.x), static_cast<float>(mp.y));

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Escape)
                window.close();
            // Uderzenie kijem po kliknięciu LMB
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left && cueActive)
            {
                sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x),
                                      static_cast<float>(event.mouseButton.y));

                float cueAngleRad = cueAngleDeg * 3.14159265f / 180.f;
                sf::Vector2f dir(std::cos(cueAngleRad), std::sin(cueAngleRad)); // od podstawy ku myszy

                // Czubek kija to dokładnie pozycja myszy
                sf::Vector2f tip = mousePos;

                // Szukaj kuli najbliżej czubka
                float bestDist = 1e9f;
                Ball* hit = nullptr;
                for (auto& b : balls) {
                    sf::Vector2f d = b.shape.getPosition() - tip;
                    float dist = std::sqrt(d.x*d.x + d.y*d.y);
                    if (dist < bestDist) { bestDist = dist; hit = &b; }
                }

                // Uderz kulę jeśli czubek jest wystarczająco blisko
                if (hit && bestDist < hit->radius + 15.0f) {
                    // Losowa siła uderzenia z zakresu [300, 900] px/s
                    float force = 300.0f + static_cast<float>(std::rand()) / RAND_MAX * 600.0f;
                    hit->velocity = dir * force;
                    cueActive = false; // chowamy kij podczas ruchu kul
                }
            }
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

        // wpadanie bil (środek bili pokonał krawędź łuzy)
        for (auto it = balls.begin(); it != balls.end(); ) {
            sf::Vector2f ballPos = it->shape.getPosition();
            bool pocketed = false;

            for (const auto& pocket : pockets) {
                sf::Vector2f pocketPos = pocket.shape.getPosition();
                float dx = ballPos.x - pocketPos.x;
                float dy = ballPos.y - pocketPos.y;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist <= pocket.radius) {
                    pocketed = true;
                    break;
                }
            }

            if (pocketed) {
                it = balls.erase(it); // usunięcie elementu z wektora
            } else {
                ++it;
            }
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

                    // Kule "zamienają się" pędami.
                    // Mają tą samą masę, więc zamieniają się jedynie prędkościami

                    vector<float> u1 = {balls[i].velocity.x, balls[i].velocity.y};
                    vector<float> u2 = {balls[j].velocity.x, balls[j].velocity.y};

                    balls[i].velocity.x = u2[0];
                    balls[i].velocity.y = u2[1];
                    balls[j].velocity.x = u1[0];
                    balls[j].velocity.y = u1[1];
                }
            }
        }

        if (!cueActive) {
            bool allStopped = true;
            for (const auto& b : balls)
                if (b.velocity.x != 0.0f || b.velocity.y != 0.0f)
                { allStopped = false; break; }
            if (allStopped) cueActive = true;
        }

        if (cueActive) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                cueAngleDeg += CUE_MANIP_SPEED * dt;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                cueAngleDeg -= CUE_MANIP_SPEED * dt;

            float cueAngleRad = cueAngleDeg * 3.14159265f / 180.f;

            // dir: od podstawy w stronę czubka (myszy)
            sf::Vector2f dir(std::cos(cueAngleRad), std::sin(cueAngleRad));

            // Czubek = mysz → podstawa jest "za" myszą o CUE_LENGTH
            sf::Vector2f cueBasePos = mousePos - dir * CUE_LENGTH;

            // Shape ma origin przy lewym końcu → ustaw przy podstawie, obróć o cueAngleDeg
            cueShape.setPosition(cueBasePos);
            cueShape.setRotation(cueAngleDeg);
        }

        window.clear(sf::Color(30, 30, 30));
        window.draw(box);

        // rysowanie łuz przed bilami, aby mogły na nie nachodzić
        for (auto& p : pockets)
            window.draw(p.shape);

        for (auto& b : balls)
            window.draw(b.shape);

        if (cueActive && !balls.empty()) // zabezpieczenie na wypadek, gdyby na stole nie było już kul
            window.draw(cueShape);

        window.display();
    }

    return 0;
}
