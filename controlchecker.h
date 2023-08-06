#ifndef CONTROLCHECKER_H
#define CONTROLCHECKER_H

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Mouse.hpp>

#include "useful_stuff.h"
#include "VisDat.h"

namespace controlchecker{

void CheckControls(VisDat * vis, sf::Event * event, sf::RenderWindow * window, CSimpleIniA * config){
        /* Mousewheel counts*/
        if (event->type == sf::Event::MouseWheelMoved) {

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {                                  /* Mousewheel + LCtrl -> Change width*/
                vis->cSetWidth(event->mouseWheel.delta);

            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt)) {                              /* Mousewheel + LAlt  -> Shift screen by 1 byte */
                vis->cShiftByte(event->mouseWheel.delta);

            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tab)) {                               /* Mousewheel + Tab  -> Shift screen by 1 bit for 16bit and 8bit mode */
                useful::schift += event->mouseWheel.delta;

            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {                             /* Mousewheel + Space  -> Change pen thickness */
                if ((event->mouseWheel.delta < 0) && (vis->pthickness + event->mouseWheel.delta) > 0) {
                    vis->pthickness += event->mouseWheel.delta;
                } else if (event->mouseWheel.delta > 0) {
                    vis->pthickness += event->mouseWheel.delta;
                }
            } else {
                vis->cScroll(event->mouseWheel.delta, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)); /* Mousewheel -> Scroll screen */
            }
        }

        /* arrow keys */
        char ArrowDirection = 0;

        if(useful::wasAnArrowKeyPressed(&ArrowDirection)){
            switch(ArrowDirection){
            case 'U':
                vis->cScroll(vis->ScrollSpeedKeyboard, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
                break;
            case 'D':
                vis->cScroll(-(vis->ScrollSpeedKeyboard), sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
                break;
            }
        }

        /* Enter key for skipping memory */
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)){
            vis->SkipUnreadableMemory();
        }



        /* Get Mouse Position */
        if (event->type == sf::Event::MouseMoved) {
            sf::Vector2i v = sf::Mouse::getPosition(*window);
            vis->MouseX = v.x / vis->sfScale;
            vis->MouseY = v.y / vis->sfScale;
        }

        /* Handle any Mouse Clicks */
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle) || sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {                               /* Middle -> Paintdropper */
                vis->cPaintDropper();
            } else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {                         /* Right ->  Paint */
                vis->cPaintDraw();
            } else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {                                /* Left + LShift ->  Save current address to config */
                    vis->cSaveAddressToConfig(config);
                } else {
                    vis->cSaveAddressToClipboard();                                                     /* Left          ->  Save current address to clipboard */
                }
            }
        }

        /* for DDA */
        if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
            vis->firstclick = true;
        }

        /* switch bitdepth */
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Add)){
            while(sf::Keyboard::isKeyPressed(sf::Keyboard::Add)){
                Sleep(10);
            }
            switch(vis->sfBitDepth){
                case 8:
                    vis->sfBitDepth = 16;
                    break;
                case 16:
                    vis->sfBitDepth = 32;
                    break;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Subtract)){
            while(sf::Keyboard::isKeyPressed(sf::Keyboard::Subtract)){
                Sleep(10);
            }
            switch(vis->sfBitDepth){
                case 16:
                    vis->sfBitDepth = 8;
                    break;
                case 32:
                    vis->sfBitDepth = 16;
                    break;
            }
        }
    }
}

#endif // CONTROLCHECKER_H
