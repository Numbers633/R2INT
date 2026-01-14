#include "gui.h"
#include <iostream>

Button::Button()
{
    rect.setSize({ 128.f, 128.f });          // default button size
    rect.setPosition({ 20.f, 20.f });
    rect.setFillColor(sf::Color(0, 255, 192));
    rect.setOutlineColor(sf::Color::Black);
    rect.setOutlineThickness(2.f);
}

void Button::CheckClick(const sf::Vector2f& mousePos)
{
    if (rect.getGlobalBounds().contains(mousePos))
    {
        if (callback) callback(); // call the assigned function
    }
}

MainGUI::MainGUI(sf::Vector2u windowSize)
{
    spacing = 16.f;
    sf::Vector2f buttonSize(96.f, 96.f);

    // Bottom-right corner coordinates
    sf::Vector2f bottomRight(windowSize.x - spacing, windowSize.y - spacing);

    // Settings button (furthest right)
    settingsButton.setSize(buttonSize);
    settingsButton.setPosition({ bottomRight.x - buttonSize.x, bottomRight.y - buttonSize.y });
    settingsButton.setColor(sf::Color(0, 144, 128)); // temporary color

    // Reset button (left of Settings)
    resetButton.setSize(buttonSize);
    resetButton.setPosition(
        { settingsButton.getPosition().x - spacing - buttonSize.x,
        bottomRight.y - buttonSize.y }
    );
    resetButton.setColor(sf::Color(96, 224, 64)); // temporary color

    // Play button (left of Reset)
    playButton.setSize(buttonSize);
    playButton.setPosition(
        { resetButton.getPosition().x - spacing - buttonSize.x,
        bottomRight.y - buttonSize.y }
    );
    playButton.setColor(sf::Color(0, 255, 128)); // temporary color

    playButton.SetCallback([]() { std::cout << "Play/Pause clicked!\n"; });
    resetButton.SetCallback([]() { std::cout << "Reset clicked!\n"; });
    settingsButton.SetCallback([]() { std::cout << "Settings clicked!\n"; });
}

void MainGUI::Resize(sf::Vector2u windowSize)
{
    sf::Vector2f buttonSize(96.f, 96.f);

    // Bottom-right corner coordinates
    sf::Vector2f bottomRight(windowSize.x - spacing, windowSize.y - spacing);

    // Settings button (furthest right)
    settingsButton.setSize(buttonSize);
    settingsButton.setPosition({ bottomRight.x - buttonSize.x, bottomRight.y - buttonSize.y });

    // Reset button (left of Settings)
    resetButton.setSize(buttonSize);
    resetButton.setPosition(
        { settingsButton.getPosition().x - spacing - buttonSize.x,
        bottomRight.y - buttonSize.y }
    );

    // Play button (left of Reset)
    playButton.setSize(buttonSize);
    playButton.setPosition(
        { resetButton.getPosition().x - spacing - buttonSize.x,
        bottomRight.y - buttonSize.y }
    );
}

void MainGUI::Draw(sf::RenderTarget& target)
{
    target.draw(playButton);
    target.draw(resetButton);
    target.draw(settingsButton);
}

void MainGUI::HandleMouseClick(const sf::Vector2f& mousePos)
{
    playButton.CheckClick(mousePos);
    resetButton.CheckClick(mousePos);
    settingsButton.CheckClick(mousePos);
}