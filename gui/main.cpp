#include <SFML/Graphics.hpp>

#include "GameView.h"
#include "InputController.h"
#include "hive/Game.h"
#include <iostream>

int main()
{
    // Not getDesktopMode() outright: a window created at exactly the
    // desktop size does not fit once the window manager adds its
    // decorations, and the manager then places it wherever it can rather
    // than shrinking it. Measured on the course dev container's
    // fluxbox/VNC desktop (1440x768), the client area landed at y = -58 --
    // the entire 64px top bar, turn indicator and Move/Throw/Cancel
    // buttons included, sat above the top edge of the screen. Fluxbox also
    // keeps a 24px toolbar along the bottom. Leave room for both.
    //
    // Raising VNC_RESOLUTION in the course image would not have helped:
    // getDesktopMode() grows with it, so the window stays exactly as
    // oversized as before.
    constexpr unsigned kDecorationAllowance = 96; // titlebar + fluxbox toolbar
    constexpr unsigned kSideAllowance = 16;       // frame borders
    const sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    const unsigned width = desktop.width > kSideAllowance ? desktop.width - kSideAllowance : desktop.width;
    const unsigned height =
        desktop.height > kDecorationAllowance ? desktop.height - kDecorationAllowance : desktop.height;

    sf::RenderWindow window(sf::VideoMode(width, height), "Hive");
    // 15fps porque no es necesario más para un juego de mesa, no quiero comerles la memoria. Que soy, chrome?
    window.setFramerateLimit(15);

    Game game;
    GameView view;
    InputController controller(game);

    // Print explicando que la GUI se puede ver en localhost:6080 cuando se corre desde un container
    std::cout << "La interfaz está disponible en http://localhost:6080 por noVNC" << std::endl;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
            }
            else if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    const sf::Vector2f point(static_cast<float>(event.mouseButton.x),
                                             static_cast<float>(event.mouseButton.y));
                    const ClickTarget target = view.hitTest(point, game, controller, window.getSize());
                    if (target.boardHex)
                    {
                        controller.handleBoardClick(*target.boardHex);
                    }
                    else if (target.handColor && target.handType)
                    {
                        controller.handleHandClick(*target.handColor, *target.handType);
                    }
                    else if (target.actionKind)
                    {
                        controller.chooseAction(*target.actionKind);
                    }
                    else if (target.cancel)
                    {
                        controller.cancel();
                    }
                    else if (target.imitationType)
                    {
                        controller.chooseImitation(*target.imitationType);
                    }
                }
            }
        }

        // Soft cream: the charcoal (Black) tiles' own dark outline keeps
        // them legible even against a light background (a near-black
        // background, e.g. (40,40,40), made them nearly indistinguishable
        // from it instead).
        window.clear(sf::Color(235, 228, 210));
        view.drawBoard(window, game);
        view.drawHands(window, game);
        // Must run after both drawBoard and drawHands: it draws a ring
        // over a selected board hex AND, separately, over a selected
        // hand-panel row (see GameView::drawSelection), and either ring
        // would otherwise get painted over by that surface's own draw
        // call happening later.
        view.drawSelection(window, game, controller);
        view.drawStatus(window, game);
        view.drawActionMenu(window, controller);
        view.drawCancelButton(window, controller);
        view.drawImitationPopup(window, game, controller);
        window.display();
    }

    return 0;
}
