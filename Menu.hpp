#include "gui.h"

class Menu {
public:
    Menu(
        int rows,
        int cols,
        sf::Vector2f boxSize,
        sf::Vector2f startPos,
        sf::Vector2f spacing,
        const sf::Font& font,
        std::vector<std::string> buttonLabels = {},
        unsigned int textSize = 48
    );

    void setButtons(std::vector<Button>&& newButtons);
    void SetButtonCallback(std::size_t index, std::function<void()> cb);
    void setPosition(sf::Vector2f pos);
    void centerIn(sf::Vector2u windowSize);
    sf::Vector2f getSize() const;

    void draw(sf::RenderTarget& target, const sf::Vector2f& mousePos, std::function<sf::Color(int, bool)> colorFunc);

    void handleClick(const sf::Vector2f& mousePos);

private:
    int rows;
    int cols;

    sf::Vector2f buttonSize;
    sf::Vector2f spacing;
    sf::Vector2f startPos;

    std::vector<Button> buttons;
};

class MenuManager {
public:
    MenuManager();
    void AddMenu(const std::string& name, Menu&& menu);
    void Open(const std::string& name);
    void Close();          // closes everything
    void Toggle(std::string root);
    void Back();           // go back one level
    void Draw(sf::RenderTarget&, sf::Vector2f mousePos);
    void HandleClick(sf::Vector2f mousePos);
    void SetColorFunction(std::function<sf::Color(int, bool)> func) { colorFunc = std::move(func); }
    void centerMenus(sf::Vector2u windowSize);

private:
    std::unordered_map<std::string, Menu> menus;
    std::vector<Menu*> stack;   // navigation stack
    std::function<sf::Color(int, bool)> colorFunc;
};

