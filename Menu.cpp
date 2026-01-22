#include "Menu.hpp"

Menu::Menu(
    int r,
    int c,
    sf::Vector2f size,
    sf::Vector2f start,
    sf::Vector2f space,
    const sf::Font& font,
    std::vector<std::string> buttonLabels,
    unsigned int textSize
)
    : rows(r), cols(c),
    buttonSize(size),
    spacing(space),
    startPos(start)
{
    buttons.resize(rows * cols);

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int index = row * cols + col;

            sf::Vector2f pos(
                startPos.x + col * (buttonSize.x + spacing.x),
                startPos.y + row * (buttonSize.y + spacing.y)
            );

            buttons[index].setSize(buttonSize);
            buttons[index].setPosition(pos);
            buttons[index].SetLabel(font,
                (index < (int)buttonLabels.size()) ? buttonLabels[index] : "",
                textSize
            );
        }
    }
}

sf::Vector2f Menu::getSize() const {
    return {
        cols * buttonSize.x + (cols - 1) * spacing.x,
        rows * buttonSize.y + (rows - 1) * spacing.y
    };
}

void Menu::setPosition(sf::Vector2f pos) {
    startPos = pos;

    for (int i = 0; i < (int)buttons.size(); ++i) {
        int row = i / cols;
        int col = i % cols;

        buttons[i].setPosition({
            startPos.x + col * (buttonSize.x + spacing.x),
            startPos.y + row * (buttonSize.y + spacing.y)
            });
    }
}

void Menu::centerIn(sf::Vector2u windowSize) {
    sf::Vector2f size = getSize();

    sf::Vector2f pos(
        (windowSize.x - size.x) * 0.5f,
        (windowSize.y - size.y) * 0.5f
    );

    setPosition(pos);
}


void Menu::setButtons(std::vector<Button>&& newButtons) {
    buttons = std::move(newButtons);

    for (int i = 0; i < (int)buttons.size(); ++i) {
        int row = i / cols;
        int col = i % cols;

        buttons[i].setSize(buttonSize);
        buttons[i].setPosition({
            startPos.x + col * (buttonSize.x + spacing.x),
            startPos.y + row * (buttonSize.y + spacing.y)
            });
    }
}

void Menu::draw(sf::RenderTarget& target, const sf::Vector2f& mousePos, std::function<sf::Color(int, bool)> colorFunc)
{
    for (int i = 0; i < (int)buttons.size(); ++i) {
        bool hovered = buttons[i].getBounds().contains(mousePos);
        buttons[i].setColor(colorFunc(i, hovered));

        target.draw(buttons[i]);
    }
}

void Menu::handleClick(const sf::Vector2f& mousePos) {
    for (auto& b : buttons)
        b.CheckClick(mousePos);
}

void Menu::SetButtonCallback(std::size_t index, std::function<void()> cb)
{
    if (index >= buttons.size())
        return; // or assert

    buttons[index].SetCallback(std::move(cb));
}