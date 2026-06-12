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
@@ -48,6 +54,9 @@ int main() {
    const float DIAMETER  = 50.0f;
    const float RADIUS    = DIAMETER / 2.0f;

    // promień łuzy ustawiony na 40.0f, aby środek kuli mógł przekroczyć krawędź w rogu stołu
    const float POCKET_RADIUS = 40.0f;

    const sf::Color colors[NUM_BALLS] = {
        sf::Color(231, 76, 60),
        sf::Color(46, 204, 113),
        @@ -62,6 +71,29 @@ int main() {

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

    @@ -202,6 +234,30 @@ int main() {
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
            @@ -267,10 +323,17 @@ int main() {

                window.clear(sf::Color(30, 30, 30));
                window.draw(box);

                // rysowanie łuz przed bilami, aby mogły na nie nachodzić
                for (auto& p : pockets)
                    window.draw(p.shape);

                for (auto& b : balls)
                    window.draw(b.shape);
                if (cueActive)

                if (cueActive && !balls.empty()) // zabezpieczenie na wypadek, gdyby na stole nie było już kul
                    window.draw(cueShape);

                window.display();
            }