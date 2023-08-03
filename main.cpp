#include "resource.h"

#include <iostream>

#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Mouse.hpp>
#include <windows.h>
#include <tlhelp32.h>
#include "SimpleIni.h"
#include "useful_stuff.h"
#include "font_data.h"
#include <sstream>
#include <vector>


/* !!!WARNING!!! horrible spaghetti code ahead */




class VisDat {
public:
    /* Other Settings */
    bool FullPageReading = true; //Way faster but cant correct for reading errors

    /* Debugging */
    bool verboseLogs = true; // as opposed to no logs at all lol

    /*   Process stuff   */
    std::string processWindowTitle = "";
    DWORD StartAddress = 0;
    DWORD  processID = 0;
    HANDLE  processHandle = 0;

    DWORD NumberOfBytesToRead = 0;

    /* Control Settings */
    DWORD ScrollSpeed = 20;
    DWORD ScrollSpeedSlow = 1;

    /*   SFML Settings   */
    DWORD sfWinX;
    DWORD sfWinY;
    DWORD sfWinXScaled = 0;
    DWORD sfWinYScaled = 0;
    DWORD sfBitDepth;
    DWORD sfScale;

    /*-> Crap for drawing */
    /*-> Picture */
    sf::Image sfIMG;
    sf::Texture sfTex;
    sf::Sprite sfSpr;

    /*-> Text */
    sf::Font sfFont;
    //std::string PathToFontFile = "arial.ttf";
    sf::Text sfText;
    DWORD sfTextSize;

    /*-> MouseBox */
    sf::RectangleShape MBox;

    /* Paint Mode */
    int pthickness = 3;
    DWORD col32 = 0xFFFFFFFF;
    WORD  col16 = 0xFFFF;
    BYTE  col8  = 0xFF;
    bool clickflip = true;
    bool firstclick = true;
    int drawx1, drawy1, drawx2, drawy2;

    /* Mouse vars */
    int MouseX = 0, MouseY = 0;

    /*  Constructor  */
    VisDat(std::string GameWindowTitle,
           DWORD Scale = 2,
           DWORD WindowX = 256, DWORD WindowY = 256,
           DWORD BitDepth = 32,
           DWORD FontSize = 24,
           DWORD StartAdd = 0x400000,
           bool  FullPage  = false
          ) {


        /* Neat Console Title */
        system("title VisDatCB Console");
        std::cout << "v1.0 https://github.com/crackman2/VisDatCB" << std::endl;


        /* Init everything */
        if (GameWindowTitle != "") {
            /* Copy stuff for later use */
            processWindowTitle = GameWindowTitle;
            sfWinX = WindowX;
            sfWinY = WindowY;
            sfWinXScaled = WindowX * Scale;
            sfWinYScaled = WindowY * Scale;
            sfScale = Scale;
            sfBitDepth = BitDepth;
            NumberOfBytesToRead = BitDepth / 8;
            sfTextSize = FontSize;
            StartAddress = StartAdd;
            FullPageReading = FullPage;

            //Logs
            if (verboseLogs) {
                std::cout << "Window title: " << processWindowTitle << std::endl;
                std::cout << "WindowX: " << WindowX << std::endl;
                std::cout << "WindowY: " << WindowY << std::endl;
                std::cout << "Scale: " << Scale << std::endl;
                std::cout << "BitDepth: " << BitDepth << std::endl;
            }


            /* Init Hacky stuff */
            if (GetProcessData()) {
                /* Init rendering related stuff */
                sfIMG.create(sfWinXScaled, sfWinYScaled);

                /* Init Text stuff */
                if (!sfFont.loadFromMemory(arial_ttf,arial_ttf_len)) //from font_data.h
                    SpewErrorAndDie("Could not load font file :(");
                sfText.setFont(sfFont);
                sfText.setCharacterSize(sfTextSize);
                sfText.setFillColor(sf::Color::Green);

                sfText.setPosition(sfWinXScaled - sfTextSize*10, 0); //right bound text
                if (verboseLogs) {
                    std::cout << "Font OK" << std::endl;
                    std::cout << "Start address: 0x" << std::hex << StartAddress << std::dec << std::endl;
                }
            } else {
                SpewErrorAndDie("Process not found. Exiting..");
            }
        } else {
            SpewErrorAndDie("Game Window Title not valid! Exiting..");
        }

    }



    /* ------> for ReadDataAndDrawOnIMG <------- */
    /* ----------- FULL PAGE READING ----------- */
    /* for FullPage reading and drawing in 32bit */
    void FullPage_Draw32(BYTE * Data) {
        unsigned int vectorindex = 0;
        for (unsigned int y = 0; y < sfWinYScaled; y += sfScale) {
            for (unsigned int x = 0; x < sfWinXScaled; x += sfScale) {
                for (unsigned int yd = 0; yd < sfScale; yd++) {
                    for (unsigned int xd = 0; xd < sfScale; xd++) {
                        /* Check for BitDepth 32 or 16 */
                        sfIMG.setPixel(x + xd, y + yd, sf::Color(Data[vectorindex], Data[vectorindex + 1], Data[vectorindex + 2], 255));
                    }
                }
                vectorindex += 4;
            }
        }
    }

    /* for FullPage reading and drawing in 16bit */
    void FullPage_Draw16(WORD * Data) {
        unsigned int vectorindex = 0;
        for (unsigned int y = 0; y < sfWinYScaled; y += sfScale) {
            for (unsigned int x = 0; x < sfWinXScaled; x += sfScale) {
                for (unsigned int yd = 0; yd < sfScale; yd++) {
                    for (unsigned int xd = 0; xd < sfScale; xd++) {
                        /* Check for BitDepth 32 or 16 */
                        sfIMG.setPixel(x + xd, y + yd, sf::Color(useful::rgb16_to_rgb32(Data[vectorindex])));
                    }
                }
                vectorindex += 1;
            }
        }
    }

    /* for FullPage reading and drawing in 8bit */
    void FullPage_Draw8(BYTE * Data) {
        unsigned int vectorindex = 0;
        for (unsigned int y = 0; y < sfWinYScaled; y += sfScale) {
            for (unsigned int x = 0; x < sfWinXScaled; x += sfScale) {
                for (unsigned int yd = 0; yd < sfScale; yd++) {
                    for (unsigned int xd = 0; xd < sfScale; xd++) {
                        /* Check for BitDepth 32 or 16 */
                        sfIMG.setPixel(x + xd, y + yd, sf::Color(useful::rgb8_to_rgb32(Data[vectorindex])));
                    }
                }
                vectorindex += 1;
            }
        }
    }


   /* ----------- NORMAL READING ----------- */

   /* draws error pattern, when reading failed */
    void DrawReadErrorPattern(unsigned int x, unsigned int y){
        for (unsigned int yd = 0; yd < sfScale; yd++) {
            for (unsigned int xd = 0; xd < sfScale; xd++) {
                sfIMG.setPixel(x + xd, y + yd, sf::Color::Black);
            }
        }
        sfIMG.setPixel(x, y, sf::Color::Red);
    }


    /* draws a single pixel, but scaled according to sfScale 32 bit */
    void DrawScaledPixel32(unsigned int x, unsigned int y, BYTE * tmp){
        for (unsigned int yd = 0; yd < sfScale; yd++) {
            for (unsigned int xd = 0; xd < sfScale; xd++) {
                sfIMG.setPixel(x + xd, y + yd, sf::Color(tmp[0], tmp[1], tmp[2], 255));
            }
        }
    }

    /* draws a single pixel, but scaled according to sfScale 16 bit */
    void DrawScaledPixel16(unsigned int x, unsigned int y, WORD * tmp){
        for (unsigned int yd = 0; yd < sfScale; yd++) {
            for (unsigned int xd = 0; xd < sfScale; xd++) {
                sfIMG.setPixel(x + xd, y + yd, sf::Color(useful::rgb16_to_rgb32(*tmp)));
            }
        }
    }

    /* draws a single pixel, but scaled according to sfScale 8 bit, weird colors, no palette */
    void DrawScaledPixel8(unsigned int x, unsigned int y, BYTE * tmp){
        for (unsigned int yd = 0; yd < sfScale; yd++) {
            for (unsigned int xd = 0; xd < sfScale; xd++) {
                sfIMG.setPixel(x + xd, y + yd, sf::Color(useful::rgb8_to_rgb32(*tmp)));
            }
        }
    }

    /* Reading from memory and writing to image */
    void ReadDataAndDrawOnIMG() {
        if (!FullPageReading) {
            /* ----> NORMAL READING <---- */
            DWORD ReadAddressCounter = 0;
            for (unsigned int y = 0; y < sfWinYScaled; y += sfScale) {
                for (unsigned int x = 0; x < sfWinXScaled; x += sfScale) {

                    /* Check BitDepth to determine reading and drawing style*/
                    if (sfBitDepth == 32) {
                        BYTE tmp[3] = { 0,0,0 };
                        /* Draw only if reading was succ */
                        if (ReadProcessMemory(processHandle, (LPVOID)(StartAddress + ReadAddressCounter), &tmp, sizeof(BYTE) * 3, 0)) {

                            DrawScaledPixel32(x, y, tmp);
                        } else {
                            DrawReadErrorPattern(x,y);
                        }
                        ReadAddressCounter += 4; //step every DWORD
                    }

                    /* What if the BitDepth = 16? */
                    else if (sfBitDepth == 16) {
                        WORD tmp = 0;

                        /* Draw only if reading was succ */
                        if (ReadProcessMemory(processHandle, (LPVOID)(StartAddress + ReadAddressCounter), &tmp, sizeof(WORD), 0)) {
                            /* Draw with scaling */
                            DrawScaledPixel16(x, y, &tmp);
                        } else {
                            /* Draw read error pattern */
                            DrawReadErrorPattern(x,y);
                        }
                        ReadAddressCounter += 2; //step every 2 bytes
                    }

                    /* What if the BitDepth = 8? Color gets weird because we have no palette */
                    else if (sfBitDepth == 8) {
                        BYTE tmp = 0;

                        /* Draw only if reading was succ */
                        if (ReadProcessMemory(processHandle, (LPVOID)(StartAddress + ReadAddressCounter), &tmp, sizeof(BYTE), 0)) {
                            /* Draw with scaling */
                            DrawScaledPixel8(x, y, &tmp);
                        } else {
                            /* Draw read error pattern */
                            DrawReadErrorPattern(x,y);
                        }
                        ReadAddressCounter ++; //increase byte a single byte
                    }
                    else {
                        SpewErrorAndDie("BitDepth invalid!");
                    }
                }
            }
        } else {
            /* ----> FULL PAGE READING <---- */
            if (sfBitDepth == 32) {
                const unsigned int DataSize = (sfWinXScaled / sfScale)*(sfWinYScaled / sfScale) * 4;
                BYTE *Data = new BYTE[DataSize];
                ReadProcessMemory(processHandle, (LPVOID)StartAddress, Data, DataSize, 0);
                FullPage_Draw32(Data);
                delete[] Data;

            } else if (sfBitDepth == 16) {
                const unsigned int DataSize = (sfWinXScaled / sfScale)*(sfWinYScaled / sfScale) * 2;
                WORD *Data = new WORD[DataSize];
                ReadProcessMemory(processHandle, (LPVOID)StartAddress, Data, DataSize, 0);
                FullPage_Draw16(Data);
                delete[] Data;

            } else if(sfBitDepth == 8) {
                const unsigned int DataSize = (sfWinXScaled / sfScale)*(sfWinYScaled / sfScale);
                BYTE *Data = new BYTE[DataSize];
                ReadProcessMemory(processHandle, (LPVOID)StartAddress, Data, DataSize, 0);
                FullPage_Draw8(Data);
                delete[] Data;

            }
        }
        ClearIMGVoid();
        PrepareDrawing();
    }

    void ClearIMGVoid() {
        /* Clear the void and draw a line*/
        for (unsigned int y = 0; y < (sfWinY * sfScale); y += sfScale) {
            for (unsigned int x = sfWinXScaled; x < (sfWinX * sfScale); x += sfScale) {
                for (unsigned int xt = 0; xt < sfScale; xt++) {
                    for (unsigned int yt = 0; yt < sfScale; yt++) {
                        sfIMG.setPixel(x + xt, y + yt, sf::Color::Black);
                    }
                }

                if (x == sfWinXScaled) {
                    for (unsigned int yt = 0; yt < sfScale; yt++) {
                        sfIMG.setPixel(x, y + yt, sf::Color::Magenta);
                    }
                }
            }
        }
    }

    void MouseBox() {
        MBox.setSize(sf::Vector2f((pthickness)*sfScale,(pthickness)*sfScale));
        MBox.setPosition(sf::Vector2f((MouseX*sfScale) - (pthickness / 2)*sfScale,(MouseY*sfScale)-(pthickness/2)*sfScale));
        MBox.setOutlineThickness(1);
        MBox.setOutlineColor(sf::Color::Red);
        MBox.setFillColor(sf::Color::Transparent);
    }

    /* Controls */
    void cScroll(int Speed, bool slow) {
        if (slow) {
            StartAddress -= Speed * (sfWinXScaled / sfScale) * NumberOfBytesToRead * ScrollSpeedSlow;
        } else {
            StartAddress -= Speed * (sfWinXScaled / sfScale) * NumberOfBytesToRead * ScrollSpeed;
        }
    }

    void cSetWidth(int Speed) {
        if (Speed < 0 && sfWinXScaled + (Speed*sfScale) > 0) {
            sfWinXScaled += Speed * sfScale;
        }

        if (Speed > 0 && sfWinXScaled + (Speed*sfScale) <= sfWinX*sfScale) {
            sfWinXScaled += Speed * sfScale;
        }
    }

    void cShiftByte(int Speed) {
        StartAddress -= Speed;
    }

    void cPaintDraw() {
        if (firstclick) {
            drawx1 = MouseX;
            drawx2 = MouseX;
            drawy1 = MouseY;
            drawy2 = MouseY;
            firstclick = false;
        }


        if (clickflip) {
            drawx1 = MouseX;
            drawy1 = MouseY;
            DDA(drawx1, drawy1, drawx2, drawy2);
        } else {
            drawx2 = MouseX;
            drawy2 = MouseY;
            DDA(drawx2, drawy2, drawx1, drawy1);
        }

        clickflip = !clickflip;
    }

    void cPaintDropper() {
        if (sfBitDepth == 32) {
            ReadProcessMemory(processHandle, (LPVOID)(StartAddress + (MouseX + MouseY * (sfWinXScaled / sfScale))*4), &col32, 4, 0);
        } else if (sfBitDepth == 16) {
            ReadProcessMemory(processHandle, (LPVOID)(StartAddress + (MouseX + MouseY * (sfWinXScaled / sfScale))*2), &col16, 2, 0);
        } else if (sfBitDepth == 8) {
            ReadProcessMemory(processHandle, (LPVOID)(StartAddress + (MouseX + MouseY * (sfWinXScaled / sfScale))), &col8, 1, 0);
        }
    }

    void cSaveAddressToClipboard() {
        int step = 4;
        if (sfBitDepth == 16)
            step = 2;
        if (sfBitDepth == 16)
            step = 1;

        char last_char;

        std::stringstream clip, temp;
        temp << std::hex << StartAddress + (MouseX + (MouseY*(sfWinXScaled/sfScale))) * step;
        clip << temp.str();
        temp.seekg(-1, std::ios::end);
        temp >> last_char;
        clip << last_char;
        useful::toClipboard(clip.str());
        std::cout << "Address: 0x" << clip.str() << " saved to clipboard!" << std::endl;
    }

    void cSaveAddressToConfig(CSimpleIniA * cfg) {
        int step = 4;
        if (sfBitDepth == 16)
            step = 2;
        if (sfBitDepth == 8)
            step = 1;

        std::stringstream clip;
        clip << std::hex << StartAddress;
        cfg->SetValue("settings", "start",std::string(clip.str()).c_str());
        cfg->SaveFile(std::string(useful::ExePath() + std::string("/VisDat_Config.ini")).c_str());
        std::cout << "Current View Address: 0x" << clip.str() << " saved to config!" << std::endl;
    }

    /* Text output */
    void UpdateText(std::string txt) {
        sfText.setString(txt);
    }

    std::string MouseHover() {
        int step = 4;
        if (sfBitDepth == 16)
            step = 2;
        if (sfBitDepth == 8)
            step = 1;
        std::stringstream hover;
        hover << std::uppercase << std::hex << StartAddress + (MouseX + (MouseY*(sfWinXScaled / sfScale))) * step;
        DWORD read = 0;
        if(ReadProcessMemory(processHandle, (LPVOID)(StartAddress + (MouseX + (MouseY*(sfWinXScaled / sfScale))) * step), &read, step, 0))
            hover << std::endl << "Value: 0x" << std::uppercase << std::hex << read << std::endl;
        else
            hover << std::endl << "Value: NO ACCESS" << std::endl;
        return hover.str();
    }

private:


    bool GetProcessData() {
        std::string ext = processWindowTitle.substr(processWindowTitle.size() - 4);
        HWND hWnd = 0;
        if(ext != ".exe") {
            hWnd = FindWindow(0,processWindowTitle.c_str());
            GetWindowThreadProcessId(hWnd, &processID);
        } else {
            //MessageBox(0,"EXE","is exe",0);
            processID = useful::findMyProc(processWindowTitle.c_str());
        }

        processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, processID);

        if (verboseLogs) {
            std::cout << "--- Process Data OK ---" << std::endl;
            std::cout << " -> processID: " << processID << std::endl;
            std::cout << " -> processHandle: " << processHandle << std::endl;
        }


        /* output proper errors when things go wrong */
        if (processHandle == 0) {

            if (verboseLogs) {
                std::cout << "Process Data Error" << std::endl;
            }
            DWORD error = GetLastError();
            // Output the error code and description
            std::cerr << "OpenProcess failed with error " << error << ": ";
            LPVOID errorMsg;
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                error,
                0, // Default language
                reinterpret_cast<LPTSTR>(&errorMsg),
                0,
                NULL
            );
            std::cerr << reinterpret_cast<LPTSTR>(errorMsg) << std::endl;
            LocalFree(errorMsg);
            // Handle the error appropriately
            // ...
            return false;
        }

        return true;
    }

    /* Error msg and quit */
    void SpewErrorAndDie(std::string Msg) {
        std::cout << Msg << std::endl << std::endl;
        system("pause");
        exit(0);
    }

    /* preparation before calling window.draw() */
    void PrepareDrawing() {
        sfTex.loadFromImage(sfIMG);
        sfSpr.setTexture(sfTex);
    }

    /* debug print values with description */
    void dbgp(std::string txt, DWORD num) {
        std::cout << txt << num << std::endl << std::endl;
        system("pause");
    }



    /* paints a single pixel into memory */
    void DDADrawSinglePixel(DWORD X, DWORD Y) {
        if        (sfBitDepth == 32) {
            WriteProcessMemory(processHandle, (LPVOID)(StartAddress + (((DWORD)X) + ((DWORD)Y) * (sfWinXScaled / sfScale)) * 4), &col32, 4, 0);
        } else if (sfBitDepth == 16) {
            WriteProcessMemory(processHandle, (LPVOID)(StartAddress + (((DWORD)X) + ((DWORD)Y) * (sfWinXScaled / sfScale)) * 2), &col16, 2, 0);
        } else if (sfBitDepth == 8 ) {
            WriteProcessMemory(processHandle, (LPVOID)(StartAddress + (((DWORD)X) + ((DWORD)Y) * (sfWinXScaled / sfScale))    ), &col8, 1, 0);
        }
    }

    /* paints a square of pixels into memory, edge length defined by thickness */
    void DDADrawWholeSquare(DWORD X, DWORD Y, int pthickness, bool adder) {
        for (int ye = -pthickness / 2; ye < (pthickness / 2) + adder; ye++) {
            for (int xe = -pthickness / 2; xe < (pthickness / 2) + adder; xe++) {
                if (sfBitDepth == 32) {
                    // VirtualProtectEx(processHandle, (LPVOID)(LPVOID)(StartAddress + (((DWORD)X + xe) + ((DWORD)Y + ye) * (sfWinXScaled / sfScale)) * 4), 4, PAGE_EXECUTE_READWRITE, NULL);  //commented to see if something breaks
                    WriteProcessMemory(processHandle, (LPVOID)(StartAddress + (((DWORD)X + xe) + ((DWORD)Y + ye) * (sfWinXScaled / sfScale)) * 4), &col32, 4, 0);
                } else if (sfBitDepth == 16) {
                    WriteProcessMemory(processHandle, (LPVOID)(StartAddress + (((DWORD)X + xe) + ((DWORD)Y + ye) * (sfWinXScaled / sfScale)) * 2), &col16, 2, 0);
                } else if (sfBitDepth == 8) {
                    WriteProcessMemory(processHandle, (LPVOID)(StartAddress + (((DWORD)X + xe) + ((DWORD)Y + ye) * (sfWinXScaled / sfScale))), &col8, 1, 0);
                }
            }
        }
    }

    /* painting the color into memory */
    void DDA(int X0, int Y0, int X1, int Y1) {
        // calculate dx & dy
        int dx = X1 - X0;
        int dy = Y1 - Y0;

        // calculate steps required for generating pixels
        int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

        // calculate increment in x & y for each steps
        float Xinc = dx / (float)steps;
        float Yinc = dy / (float)steps;

        // Put pixel for each step
        float X = X0;
        float Y = Y0;
        for (int i = 0; i <= steps; i++) {
            bool adder = pthickness % 2 == 1; //dont remember what this is for, sorry
            if (pthickness > 1) {  /* if thickness is greater than one, draw square */
                DDADrawWholeSquare(X, Y, pthickness, adder);
            } else {
                DDADrawSinglePixel(X,Y);
            }
            X += Xinc;           // increment in x at each step
            Y += Yinc;           // increment in y at each step
            // for visualization of line-
            // generation step by step
        }
    }
};


int main() {
    /* Load settings from INI */
    CSimpleIniA config;
    if (config.LoadFile(std::string(useful::ExePath() + std::string("/VisDat_Config.ini")).c_str()) < 0) {
        useful::firstTimeINISetup(&config);
    }

    /* apply settings from ini */
    int cfgWinX = std::stoi(config.GetValue("settings", "winx"));
    int cfgWinY = std::stoi(config.GetValue("settings", "winy"));
    int cfgScale = std::stoi(config.GetValue("settings", "scale"));
    int cfgBitDepth = std::stoi(config.GetValue("settings", "bitdepth"));
    int cfgTextSize = std::stoi(config.GetValue("settings", "textsize"));
    bool cfgFullPage = std::stoi(config.GetValue("settings", "bFullPageReading"));

    std::string cfgWinTitle = config.GetValue("settings", "title");
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
            if (event.type == sf::Event::MouseWheelMoved) {

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {                                  /* Mousewheel + LCtrl -> Change width*/
                    vis.cSetWidth(event.mouseWheel.delta);

                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt)) {                              /* Mousewheel + LAlt  -> Shift screen by 1 byte */
                    vis.cShiftByte(event.mouseWheel.delta);

                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tab)) {                               /* Mousewheel + Tab  -> Shift screen by 1 byte */
                    useful::schift += event.mouseWheel.delta;

                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {                             /* Mousewheel + Space  -> Change pen thickness */
                    if ((event.mouseWheel.delta < 0) && (vis.pthickness + event.mouseWheel.delta) > 0) {
                        vis.pthickness += event.mouseWheel.delta;
                    } else if (event.mouseWheel.delta > 0) {
                        vis.pthickness += event.mouseWheel.delta;
                    }
                } else {
                    vis.cScroll(event.mouseWheel.delta, sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)); /* Mousewheel -> Scroll screen */
                }
            }

            /* Get Mouse Position */
            if (event.type == sf::Event::MouseMoved) {
                sf::Vector2i v = sf::Mouse::getPosition(window);
                vis.MouseX = v.x / vis.sfScale;
                vis.MouseY = v.y / vis.sfScale;
            }

            /* Handle any Mouse Clicks */
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle) || sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {                               /* Middle -> Paintdropper */
                    vis.cPaintDropper();
                } else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {                         /* Right ->  Paint */
                    vis.cPaintDraw();
                } else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {                                /* Left + LShift ->  Save current address to config */
                        vis.cSaveAddressToConfig(&config);
                    } else {
                        vis.cSaveAddressToClipboard();                                                     /* Left          ->  Save current address to clipboard */
                    }
                }
            }

            if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
                vis.firstclick = true;
            }

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
                           "Middle click = Copy color at mouse position to paint with)","Controls",0);
            }
        }

        /* Update status Text */
        final_ss << "Address: 0x" << std::hex << std::uppercase << vis.StartAddress << std::endl;
        final_ss << "Mouse:   0x" << vis.MouseHover();
        final_ss << "Width: " << std::dec << vis.sfWinXScaled / vis.sfScale << std::endl;
        final_ss << "mX: " << vis.MouseX << std::endl;
        final_ss << "mY: " << vis.MouseY << std::endl;
        final_ss << "Thick: " << std::dec << vis.pthickness << std::endl;

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
