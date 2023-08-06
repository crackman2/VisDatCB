#include "resource.h"

#include <iostream>

#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Mouse.hpp>
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <sstream>
#include <vector>
#include "VisDat.h"
#include "controlchecker.h"


/* !!!WARNING!!! horrible spaghetti code ahead */


int main() {
    /* Load settings from INI */
    CSimpleIniA config;
    if (config.LoadFile(std::string(useful::ExePath() + std::string("/VisDat_Config.ini")).c_str()) < 0) {
        useful::firstTimeINISetup(&config);
    }

    /* apply settings from ini */
    int cfgWinX      = std::stoi(config.GetValue("settings", "winx"));
    int cfgWinY      = std::stoi(config.GetValue("settings", "winy"));
    int cfgScale     = std::stoi(config.GetValue("settings", "scale"));
    int cfgBitDepth  = std::stoi(config.GetValue("settings", "bitdepth"));
    int cfgTextSize  = std::stoi(config.GetValue("settings", "textsize"));
    bool cfgFullPage = std::stoi(config.GetValue("settings", "bFullPageReading"));
    std::string cfgWinTitle =    config.GetValue("settings", "title");

    DWORD cfgStart = 0;
    std::stringstream startss;
    startss << std::hex << config.GetValue("settings", "start");
    startss >> cfgStart;


    /* Start VisDat and render window */
    VisDat vis(cfgWinTitle,cfgScale,cfgWinX,cfgWinY,cfgBitDepth,cfgTextSize,cfgStart,cfgFullPage);
    sf::RenderWindow window(sf::VideoMode(vis.sfWinXScaled, vis.sfWinYScaled, vis.sfBitDepth), "VisDatCB");
    sf::Event event;
    std::stringstream final_ss;

    /* Set window icon */
    HICON icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
    if (icon) {
        HWND hwnd = window.getSystemHandle();
        SetClassLongPtr(hwnd, GCLP_HICON, (LONG_PTR)icon);
    }

    while (window.isOpen()) {

        /* Handle events */
        while (window.pollEvent(event)) {

            /* Close it when it's being closed lol */
            if (event.type == sf::Event::Closed) {
                window.close();
                CloseHandle(vis.processHandle);
            }

            /* Controls */
            controlchecker::CheckControls(&vis, &event, &window, &config);

            /* show controls */
            if( sf::Keyboard::isKeyPressed(sf::Keyboard::F1) ) {
                MessageBox(0,"Mousewheel = Scroll up and down\n"
                           "Mousewheel + LShift = Scroll up and down slowly\n"
                           "Mousewheel + Space = Adjust pen thickness\n"
                           "Mousewheel + LAlt = Shift data horizontally by 1 byte\n"
                           "Mousewheel + Tab = Shift color palette by 1 bit (only in 16bit color mode)\n"
                           "Mousewheel + LCtrl = Adjust width\n"
                           "Left click = Copy address at mouse position to clipboard\n"
                           "Left click + LShift = Save current view to VisDat_Config.ini\n"
                           "Right click = Paint at mouse position\n"
                           "Middle click = Copy color at mouse position to paint with)\n"
                           "+ and - to increase or decrease BitDepth\n"
                           "Arrow keys up and down to auto-scroll\n"


                           ,"Controls",0);
            }
        }

        /* Update status Text */
        final_ss << "address: " << std::hex << std::uppercase << vis.StartAddress << std::endl;
        final_ss << "module : " << vis.GetCurrentModuleName() << std::endl;
        final_ss << "mouse  : " << vis.MouseHover();
        final_ss << "width  : " << std::dec << vis.sfWinXScaled / vis.sfScale << std::endl;
        final_ss << "mX     : " << vis.MouseX << std::endl;
        final_ss << "mY     : " << vis.MouseY << std::endl;
        final_ss << "thick  : " << std::dec << vis.pthickness << std::endl;
        final_ss << "bits   : " << std::dec << vis.sfBitDepth << std::endl;
        //          "Value  : "

        /* Draw the things */
        window.clear(sf::Color::Black);
        vis.ReadDataAndDrawOnIMG();
        vis.UpdateText(final_ss.str());
        vis.MouseBox();
        final_ss.str(""); //Clear stream

        window.draw(vis.sfSpr);
        window.draw(vis.sfText);
        window.draw(vis.MBox);
        window.display();
    }
    return 0;
}
