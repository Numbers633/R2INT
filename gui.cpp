#include "gui.h"
#include <iostream>

sf::Texture& GetDummyTexture()
{
    static sf::Texture dummy;
    static bool initialized = false;

    if (!initialized)
    {
        sf::Image img({ 1, 1 }, sf::Color::White);
        dummy.loadFromImage(img);
        initialized = true;
    }

    return dummy;
}           

Button::Button() : icon(GetDummyTexture())
{
    rect.setFillColor(sf::Color::White);
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

void Button::SetIcon(const sf::Texture& tex)
{
    texture = &tex;
    icon.setTexture(tex, true);
    UpdateIconTransform();
}
void Button::UpdateIconTransform()
{
    if (texture) {
        sf::Vector2f buttonSize = rect.getSize();
        sf::Vector2u texSize = texture->getSize();
        float scaleX = buttonSize.x / texSize.x;
        float scaleY = buttonSize.y / texSize.y;
        float scale = std::min(scaleX, scaleY);
        icon.setScale({ scale, scale });
        // Center the icon within the button
        sf::FloatRect iconBounds = icon.getLocalBounds();
        icon.setPosition({
            rect.getPosition().x + (buttonSize.x - iconBounds.size.x * scale) / 2.f,
            rect.getPosition().y + (buttonSize.y - iconBounds.size.y * scale) / 2.f }
        );
    }
}
void Button::UpdateLabelTransform()
{
    if (!label)
        return;

    sf::FloatRect bounds = label->getLocalBounds();
    label->setOrigin({
        bounds.position.x + bounds.size.x / 2.f,
        bounds.position.y + bounds.size.y / 2.f });

    label->setPosition(
        rect.getPosition() + rect.getSize() * 0.5f
    );
}

void Button::SetLabel(const sf::Font& font,
    const std::string& text,
    unsigned int size)
{
    label.emplace(font, text, size);
    label->setFillColor(sf::Color::Black);

    UpdateLabelTransform();
}

MainGUI::MainGUI(sf::Vector2u windowSize)
{
    spacing = 16.f;
    sf::Vector2f buttonSize(64.f, 64.f);

    playTex.loadFromFile("assets\\Play.png");
    pauseTex.loadFromFile("assets\\Pause.png");
    resetTex.loadFromFile("assets\\Reset.png");
    settingsTex.loadFromFile("assets\\Menu.png");

    // Bottom-right corner coordinates
    sf::Vector2f bottomRight(windowSize.x - spacing, windowSize.y - spacing);

    // Settings button (furthest right)
    settingsButton.setSize(buttonSize);
    settingsButton.setPosition({ bottomRight.x - buttonSize.x, spacing });
    settingsButton.setColor(sf::Color(0, 144, 128)); // temporary color
    settingsButton.SetIcon(settingsTex);

    // Reset button (left of Settings)
    resetButton.setSize(buttonSize);
    resetButton.setPosition(
        { bottomRight.x - buttonSize.x, bottomRight.y - buttonSize.y }
    );
    resetButton.setColor(sf::Color(96, 224, 64)); // temporary color
    resetButton.SetIcon(resetTex);

    // Play button (left of Reset)
    playButton.setSize(buttonSize);
    playButton.setPosition(
        { resetButton.getPosition().x - spacing - buttonSize.x,
        bottomRight.y - buttonSize.y }
    );
    playButton.setColor(sf::Color(0, 255, 128)); // temporary color
    playButton.SetIcon(playTex);

    playButton.SetCallback([]() { std::cout << "Play/Pause clicked!\n"; });
    resetButton.SetCallback([]() { std::cout << "Reset clicked!\n"; });
    settingsButton.SetCallback([]() { std::cout << "Settings clicked!\n"; });
}

void MainGUI::Resize(sf::Vector2u windowSize)
{
    sf::Vector2f buttonSize(64.f, 64.f);

    // Bottom-right corner coordinates
    sf::Vector2f bottomRight(windowSize.x - spacing, windowSize.y - spacing);

    // Settings button (furthest right)
    settingsButton.setSize(buttonSize);
    settingsButton.setPosition({ bottomRight.x - buttonSize.x, spacing });

    // Reset button (left of Settings)
    resetButton.setSize(buttonSize);
    resetButton.setPosition(
        { bottomRight.x - buttonSize.x, bottomRight.y - buttonSize.y }
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