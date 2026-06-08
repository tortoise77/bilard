#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

int main() {
    // create the window
    sf::RenderWindow window(sf::VideoMode(800, 600), "My window");

    // create some shapes
    sf::CircleShape circle(100.0);
    circle.setPosition(100.0, 300.0);
    circle.setFillColor(sf::Color(100, 250, 50));

    sf::RectangleShape rectangle(sf::Vector2f(120.0, 60.0));
    rectangle.setPosition(500.0, 400.0);
    rectangle.setFillColor(sf::Color(100, 50, 250));

    sf::ConvexShape triangle;
    triangle.setPointCount(3);
    triangle.setPoint(0, sf::Vector2f(0.0, 0.0));
    triangle.setPoint(1, sf::Vector2f(0.0, 100.0));
    triangle.setPoint(2, sf::Vector2f(140.0, 40.0));
    triangle.setOutlineColor(sf::Color::Red);
    triangle.setOutlineThickness(5);
    triangle.setPosition(600.0, 100.0);

    std::vector<sf::Shape*> shapes;
    shapes.push_back(&circle);
    shapes.push_back(&rectangle);
    shapes.push_back(&triangle);

    sf::Clock clock;
    sf::Vector2f velocity(-800, -677);
    float velocityAng = 70, time = 0;

    // run the program as long as the window is open
    while (window.isOpen()) {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event)) {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        sf::Time elapsed = clock.restart();
        //std::cout << elapsed.asMicroseconds() << std::endl;
        rectangle.move(sf::Vector2f(velocity.x*elapsed.asSeconds() , velocity.y*elapsed.asSeconds()));
        rectangle.rotate(velocityAng * elapsed.asSeconds());

        time += elapsed.asSeconds();
        sf::FloatRect rectangle_bounds = rectangle.getGlobalBounds();
        if (time > .5) {
            std::cout << rectangle_bounds.top << " " << rectangle_bounds.left << " " ;
            std::cout << rectangle_bounds.width << " " << rectangle_bounds.height << std::endl;
            time = 0;
        }

        if (rectangle_bounds.left < 0)
            velocity.x = +std::abs(velocity.x);
        if (rectangle_bounds.left + rectangle_bounds.width > window.getSize().x)
            velocity.x = -std::abs(velocity.x);

        if (rectangle_bounds.top<0)
            velocity.y = +std::abs(velocity.x);

        if (rectangle_bounds.top + rectangle_bounds.height > window.getSize().y)
            velocity.y = -std::abs(velocity.x);

        // clear the window with black color
        window.clear(sf::Color::Black);

        // draw everything here...
        for (auto &s: shapes)
            window.draw(*s);

        // end the current frame
        window.display();
    }

    return 0;
}

