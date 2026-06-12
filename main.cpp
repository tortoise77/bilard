#include <SFML/Window.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>
#include "GameObjects.h" // wczytywanie moich klas

using namespace std;

int main() {
    const unsigned int WINDOW_W = 1366;
    const unsigned int WINDOW_H = 768;
    sf::RenderWindow window(sf::VideoMode(WINDOW_W, WINDOW_H), "Bilard - Zaliczenie");
    window.setFramerateLimit(120);

    // KONTROLA WIDOKU - Pozwala na obrót stołu niezależnie od interfejsu (UI)
    sf::View gameView(sf::FloatRect(0, 0, WINDOW_W, WINDOW_H));
    sf::View uiView = window.getDefaultView();
    bool isRotated = false;

    // wczytanie systemowej czcionki
    sf::Font font;
    font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    // tekst UI (punkty i tury na górze)
    sf::Text uiText;
    uiText.setFont(font);
    uiText.setCharacterSize(24);
    uiText.setFillColor(sf::Color::White);
    uiText.setPosition(20.f, 20.f);

    // pasek menu oraz przyciski na dole ekranu
    sf::Text controlsText;
    controlsText.setFont(font);
    controlsText.setCharacterSize(20);
    controlsText.setFillColor(sf::Color(200, 200, 200));
    controlsText.setString("[F3] Obroc stol   |   [F5] Zapisz gre   |   [F9] Wczytaj gre   |   [F6] Zapisz wyniki   |   [ESC] Wyjscie");
    controlsText.setPosition(20.f, WINDOW_H - 40.f);

    // tekst powiadomień systemowych
    sf::Text notifText;
    notifText.setFont(font);
    notifText.setCharacterSize(24);
    notifText.setFillColor(sf::Color::Yellow);
    notifText.setPosition(20.f, WINDOW_H - 80.f);

    std::string currentNotif = "";
    sf::Clock notifClock; // zegar odliczający czas wyświetlania komunikatu

    // Wymiary stołu
    const float BOX_W = 800.0f, BOX_H = 400.0f;
    const float BOX_LEFT = (WINDOW_W - BOX_W) / 2.0f, BOX_TOP = (WINDOW_H - BOX_H) / 2.0f;
    const float WALL_LEFT = BOX_LEFT, WALL_RIGHT = BOX_LEFT + BOX_W;
    const float WALL_TOP = BOX_TOP, WALL_BOTTOM = BOX_TOP + BOX_H;
    const float RADIUS = 25.0f;

    // jeden kontener na obiekty
    std::vector<std::unique_ptr<GameObject>> objects;

    // stół i 6 łuz
    objects.push_back(std::make_unique<Table>(BOX_W, BOX_H, BOX_LEFT, BOX_TOP));
    for (auto& pos : vector<sf::Vector2f>{{WALL_LEFT, WALL_TOP}, {(WALL_LEFT+WALL_RIGHT)/2.0f, WALL_TOP}, {WALL_RIGHT, WALL_TOP}, {WALL_LEFT, WALL_BOTTOM}, {(WALL_LEFT+WALL_RIGHT)/2.0f, WALL_BOTTOM}, {WALL_RIGHT, WALL_BOTTOM}})
        objects.push_back(std::make_unique<Pocket>(40.0f, pos));

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    auto randf = [](float min, float max) { return min + static_cast<float>(std::rand()) / RAND_MAX * (max - min); };

    // losowy rozrzut kul
    for (int i = 0; i < config.numBalls; ++i) {
        sf::Vector2f pos; bool ok = false; int attempts = 0;
        while (!ok && attempts < 1000) {
            pos.x = randf(WALL_LEFT + RADIUS, WALL_RIGHT - RADIUS);
            pos.y = randf(WALL_TOP + RADIUS, WALL_BOTTOM - RADIUS);
            ok = true;
            for (const auto& obj : objects) {
                if (Ball* other = dynamic_cast<Ball*>(obj.get())) {
                    if (std::hypot(pos.x - other->getPosition().x, pos.y - other->getPosition().y) < RADIUS + other->getRadius() + 2.0f) {
                        ok = false; break;
                    }
                }
            }
            ++attempts;
        }
        objects.push_back(std::make_unique<Ball>(RADIUS, pos, sf::Color(rand()%255, rand()%255, rand()%255)));
    }

    Player players[2] = { Player("Gracz 1"), Player("Gracz 2") };
    int currentPlayerIdx = 0;
    int ballsBeforeShot = 0; // zmienna do liczenia tur

    // sterowanie kijem
    bool cueActive = true, isCharging = false;
    float currentPower = 0.0f, cueAngleDeg = -90.f;
    Ball* aimedBall = nullptr;
    Ball* hoveredBall = nullptr;

    sf::RectangleShape cueShape(sf::Vector2f(320.0f, 8.0f));
    cueShape.setOrigin(320.0f, 4.0f);
    cueShape.setFillColor(sf::Color(160, 82, 45));

    sf::Clock clock;
    while (window.isOpen()) {
        // Poprawka dla zmapowania kursora myszy przy obróconym stole
        sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
        sf::Vector2f mPos = window.mapPixelToCoords(pixelPos, gameView);

        // namierzanie bili pod myszką
        if (cueActive && !isCharging) {
            float bestD = 1e9f;
            hoveredBall = nullptr;
            for(auto& o : objects) {
                if(Ball* b = dynamic_cast<Ball*>(o.get())) {
                    float d = std::hypot(b->getPosition().x - mPos.x, b->getPosition().y - mPos.y);
                    if(d < bestD && d < 200.0f) {
                        bestD = d;
                        hoveredBall = b;
                    }
                }
            }
            if (hoveredBall) {
                cueAngleDeg = std::atan2(mPos.y - hoveredBall->getPosition().y, mPos.x - hoveredBall->getPosition().x) * 180.f / 3.14159f;
            }
        }

        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed || (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape)) window.close();

            // Klawiszologia specjalna
            if (e.type == sf::Event::KeyPressed) {
                // Obrót stołu o 180 stopni (F3)
                if (e.key.code == sf::Keyboard::F3) {
                    isRotated = !isRotated;
                    gameView.setRotation(isRotated ? 180.f : 0.f);
                    currentNotif = isRotated ? "Widok obrocony o 180 stopni!" : "Widok domyslny przywrocony!";
                    notifClock.restart();
                }

                // zapis oraz odczyt (F5, F9, F6) z informacją na ekranie
                if (e.key.code == sf::Keyboard::F5) {
                    std::ofstream out("save.txt");
                    out << currentPlayerIdx << " " << players[0].score << " " << players[1].score << "\n";
                    for (const auto& obj : objects) if (Ball* b = dynamic_cast<Ball*>(obj.get()))
                            out << "B " << b->getPosition().x << " " << b->getPosition().y << " " << b->getVelocity().x << " " << b->getVelocity().y << " " << b->getColor().toInteger() << "\n";

                    currentNotif = "Pomyslnie zapisano stan gry!";
                    notifClock.restart();
                }
                if (e.key.code == sf::Keyboard::F9) {
                    std::ifstream in("save.txt");
                    if (in.is_open()) {
                        in >> currentPlayerIdx >> players[0].score >> players[1].score;
                        objects.erase(std::remove_if(objects.begin(), objects.end(), [](const auto& obj) { return dynamic_cast<Ball*>(obj.get()) != nullptr; }), objects.end());
                        std::string type;
                        while (in >> type) {
                            if (type == "B") {
                                float x, y, vx, vy; sf::Uint32 col; in >> x >> y >> vx >> vy >> col;
                                auto b = std::make_unique<Ball>(RADIUS, sf::Vector2f(x, y), sf::Color(col));
                                b->setVelocity(sf::Vector2f(vx, vy)); objects.push_back(std::move(b));
                            }
                        }
                        currentNotif = "Wczytano zapisana gre!";
                        notifClock.restart();
                    } else {
                        currentNotif = "Blad: Nie znaleziono pliku save.txt!";
                        notifClock.restart();
                    }
                }
                if (e.key.code == sf::Keyboard::F6) {
                    std::ofstream out("scores.txt", std::ios::app);
                    out << players[0].name << ": " << players[0].score << " | " << players[1].name << ": " << players[1].score << "\n";

                    currentNotif = "Zapisano wyniki do pliku scores.txt!";
                    notifClock.restart();
                }
            }

            // ładowanie siły kija
            if (e.type == sf::Event::MouseButtonPressed && cueActive && !isCharging) {
                if (e.mouseButton.button == sf::Mouse::Left && hoveredBall) {
                    isCharging = true;
                    aimedBall = hoveredBall;
                    currentPower = 100.0f;
                }
            }

            // oddawanie strzału
            if (e.type == sf::Event::MouseButtonReleased && isCharging) {
                if (e.mouseButton.button == sf::Mouse::Left && aimedBall) {
                    float rad = cueAngleDeg * 3.14159f / 180.f;
                    sf::Vector2f dir(std::cos(rad), std::sin(rad));
                    aimedBall->setVelocity(dir * currentPower);

                    // zapis ilośći bil przed ruchem żeby wiedzieć czy była zmiana tury
                    ballsBeforeShot = 0;
                    for (const auto& obj : objects) if (dynamic_cast<Ball*>(obj.get())) ballsBeforeShot++;

                    cueActive = false;
                    isCharging = false;
                    aimedBall = nullptr;
                    hoveredBall = nullptr;
                    currentPower = 0.0f;
                }
            }
        }

        float dt = clock.restart().asSeconds();
        if (isCharging) currentPower = min(currentPower + 1200.0f * dt, 1500.0f);

        for (auto& obj : objects) obj->update(dt);

        int ballsCount = 0;
        for (auto it = objects.begin(); it != objects.end(); ) {
            if (Ball* b = dynamic_cast<Ball*>(it->get())) {
                ballsCount++;
                sf::Vector2f p = b->getPosition(); sf::Vector2f v = b->getVelocity();

                // odbicia bil
                if(p.x - RADIUS < WALL_LEFT || p.x + RADIUS > WALL_RIGHT) v.x *= -1;
                if(p.y - RADIUS < WALL_TOP || p.y + RADIUS > WALL_BOTTOM) v.y *= -1;
                b->setVelocity(v);

                // wpadanie bil do łuzy
                bool pocketed = false;
                for(auto& o : objects) if(Pocket* pck = dynamic_cast<Pocket*>(o.get()))
                        if(std::hypot(p.x-pck->getPosition().x, p.y-pck->getPosition().y) < 40.0f) pocketed = true;

                if(pocketed) {
                    it = objects.erase(it);
                    players[currentPlayerIdx].score++;
                } else {
                    ++it;
                }
            } else ++it;
        }

        // zderzenia bil
        for (size_t i = 0; i < objects.size(); ++i) {
            Ball* b1 = dynamic_cast<Ball*>(objects[i].get());
            if (!b1) continue;
            for (size_t j = i + 1; j < objects.size(); ++j) {
                Ball* b2 = dynamic_cast<Ball*>(objects[j].get());
                if (!b2) continue;
                sf::Vector2f p1 = b1->getPosition(), p2 = b2->getPosition();
                float dist = std::hypot(p2.x - p1.x, p2.y - p1.y);
                if (dist < 50.0f && dist > 0.0001f) {
                    float shift = (50.0f - dist) / 2.0f;
                    float nx = (p2.x - p1.x) / dist, ny = (p2.y - p1.y) / dist;
                    b1->setPosition(sf::Vector2f(p1.x - nx * shift, p1.y - ny * shift));
                    b2->setPosition(sf::Vector2f(p2.x + nx * shift, p2.y + ny * shift));
                    sf::Vector2f v1 = b1->getVelocity();
                    b1->setVelocity(b2->getVelocity());
                    b2->setVelocity(v1);
                }
            }
        }

        // sprawdzanie czy wszystkie kule się zatrzymały
        if (!cueActive) {
            bool allStopped = true;
            for (const auto& obj : objects) if (Ball* b = dynamic_cast<Ball*>(obj.get())) if (std::abs(b->getVelocity().x) > 0.1f || std::abs(b->getVelocity().y) > 0.1f) allStopped = false;

            if (allStopped) {
                cueActive = true;

                int ballsAfterShot = 0;
                for (const auto& obj : objects) if (dynamic_cast<Ball*>(obj.get())) ballsAfterShot++;

                if (ballsAfterShot == ballsBeforeShot) {
                    currentPlayerIdx = (currentPlayerIdx + 1) % 2;
                }
            }
        }

        // napisy na ekranie
        if (cueActive && ballsCount == 0) uiText.setString("KONIEC GRY! Wygrywa: " + players[(players[0].score > players[1].score ? 0 : 1)].name);
        else uiText.setString("Tura: " + players[currentPlayerIdx].name + " | " + players[0].name + ": " + to_string(players[0].score) + " | " + players[1].name + ": " + to_string(players[1].score));

        // celowanie i ustalanie pozycji kija
        if (cueActive) {
            Ball* target = isCharging ? aimedBall : hoveredBall;
            if (target) {
                float rad = cueAngleDeg * 3.14159f / 180.f;
                sf::Vector2f dir(std::cos(rad), std::sin(rad));
                sf::Vector2f cueTipPos = target->getPosition() - dir * RADIUS;

                if (isCharging) {
                    float pullBack = (currentPower / 1500.0f) * 150.0f;
                    cueTipPos -= dir * pullBack;
                }

                cueShape.setPosition(cueTipPos);
                cueShape.setRotation(cueAngleDeg);
            }
        }

        // --- SEKCJA RYSOWANIA ---
        window.clear(sf::Color(30, 30, 30));

        // 1. Rysowanie środowiska gry (korzystając z gameView dla obrotów stołu)
        window.setView(gameView);
        for (auto& obj : objects) obj->draw(window);

        // rysowanie kija i wskaźnika trajektorii
        if (cueActive && (isCharging ? aimedBall : hoveredBall) != nullptr) {
            Ball* target = isCharging ? aimedBall : hoveredBall;
            float rad = cueAngleDeg * 3.14159f / 180.f;
            sf::Vector2f dir(std::cos(rad), std::sin(rad));

            // Linia trajektorii uderzenia (częściowo transparentna do wyznaczenia kierunku)
            sf::VertexArray trajectory(sf::Lines, 2);
            trajectory[0].position = target->getPosition() + dir * RADIUS;
            trajectory[0].color = sf::Color(255, 255, 255, 120);
            trajectory[1].position = target->getPosition() + dir * 300.0f; // linia sięga w przód
            trajectory[1].color = sf::Color(255, 255, 255, 0); // zanika na końcu

            window.draw(trajectory);
            window.draw(cueShape);
        }

        // 2. Rysowanie interfejsu 2D (korzystając ze stałego uiView)
        window.setView(uiView);

        // Paski postępu graczy
        float winThreshold = config.numBalls / 2.0f + 0.5f; // Szacowany próg wymagany do wygrania w oparciu o sumę kul
        float p1Ratio = std::min(1.0f, players[0].score / winThreshold);
        float p2Ratio = std::min(1.0f, players[1].score / winThreshold);

        // Tło pasków
        sf::RectangleShape p1BarBg(sf::Vector2f(200.f, 8.f));
        p1BarBg.setPosition(20.f, 55.f);
        p1BarBg.setFillColor(sf::Color(60, 60, 60));

        sf::RectangleShape p2BarBg(sf::Vector2f(200.f, 8.f));
        p2BarBg.setPosition(320.f, 55.f);
        p2BarBg.setFillColor(sf::Color(60, 60, 60));

        // Paski wypełnienia
        sf::RectangleShape p1Bar(sf::Vector2f(200.f * p1Ratio, 8.f));
        p1Bar.setPosition(20.f, 55.f);
        p1Bar.setFillColor(sf::Color(100, 150, 255)); // Kolor Gracz 1

        sf::RectangleShape p2Bar(sf::Vector2f(200.f * p2Ratio, 8.f));
        p2Bar.setPosition(320.f, 55.f);
        p2Bar.setFillColor(sf::Color(255, 100, 100)); // Kolor Gracz 2

        window.draw(p1BarBg);
        window.draw(p1Bar);
        window.draw(p2BarBg);
        window.draw(p2Bar);

        // Tekstyy
        window.draw(uiText);
        window.draw(controlsText);

        // znikające powiadomienia na dole ekranu
        if (notifClock.getElapsedTime().asSeconds() < 3.0f) {
            notifText.setString(currentNotif);
            window.draw(notifText);
        }

        window.display();
    }
    return 0;
}