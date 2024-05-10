#include <algorithm>
#include <filesystem>
#include <sys/ioctl.h>
#include <ncurses.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>

class Console
{
    public:
    Console() {}

    void hideCursor()
    {
        print("\033[?25l");
    }

    void showCursor()
    {
        print("\033[?25h");
    }

    void moveCursor(int row, int column)
    {
        print("\033[" + std::to_string(row) + ";" + std::to_string(column) + "H");
    }

    void clearScreen()
    {
        print("\033[2J");
    }

    void print(const std::string& str)
    {
        std::cout << str << std::flush;
    }
};

class Frame
{
    public:
    Frame() {}

    void limit()
    {
        constexpr long MS_PER_FRAME { 120 };
        std::this_thread::sleep_for(std::chrono::milliseconds(MS_PER_FRAME));
    }
};

enum class ProgramState { Continue, Stop, Left, Right, Up, Down };

class Input
{
    public:
    Input() {}

    void setup()
    {
        initscr();
        noecho();
        cbreak();
        nodelay(stdscr, TRUE);
    }

    void clean()
    {
        endwin();
    }

    ProgramState handle()
    {
        
        int c = getch();

        if (c == 'w')
            return ProgramState::Up;
        if (c == 'a')
            return ProgramState::Left;
        if (c == 's')
            return ProgramState::Down;
        if (c == 'd')
            return ProgramState::Right;

        if (c == 27)
            return ProgramState::Stop;            

        return ProgramState::Continue;
    }
};

class Render
{
    private:
    Console console{};

    public:
    void Draw(const std::vector<std::string>& map)
    {
        for (int row = 0; row < map.size(); row++)
        {
            console.moveCursor(row, 0);
            console.print(map[row]);
        }
    }
};

enum class Direction { Up, Right, Down, Left };

class Game
{
    public:
    Direction direction{Direction::Right};

    Game() 
    {
        map = 
        {
            "                                                                   ",
            "  %            %    %                                %       %     ",
            "  %            %    %                                %       %     ",
            "  %            %    %                                %       %     ",
            "  %            %    %                                %       %     ",
            "  % %%    %%   %    %     %%      %   %   %%   % %   %     %%%     ",
            "  %%  %  %%%%  %    %    %  %     %   %  %  %  %% %  %    %  %     ",
            "  %   %  %     %    %    %  %     % % %  %  %  %     %    %  %     ",
            "  %   %   %%%   %%   %%   %%       % %    %%   %      %%   %%      ",
            "                                                                   ",
            "                                                                   ",
            "                                                                   ",
            "                                                                   ",
            "                                                                   ",
            "                                                                   ",
            "                                                                   ",
            "                                                                   ",
            "                                                                   ",
            "                                                                   ",
            "                                                                   ",
            "                                                                   ",
            "                                                                   ",
            "                                                                   ",
        };

        maxRowIndex = map.size()-1;
        maxColIndex = map[0].size()-1;
    }

    void SetDirection(ProgramState state)
    {
        if (state == ProgramState::Down)
            direction = Direction::Down;
        else if (state == ProgramState::Right)
            direction = Direction::Right;
        else if (state == ProgramState::Up)
            direction = Direction::Up;
        else if (state == ProgramState::Left)
            direction = Direction::Left;
    }

    void Update()
    {
        switch (direction)
        {
            case Direction::Down:
            {
                std::string lastLine = map[maxRowIndex];
                for (int r = maxRowIndex; r > 0; r--)
                    map[r] = map[r-1];
                map[0] = lastLine;
                break;
            }
            case Direction::Up:
            {
                std::string firstLine = map[0];
                for (int r = 0; r < maxRowIndex; r++)
                    map[r] = map[r+1];
                map[maxRowIndex] = firstLine;
                break;
            }
            case Direction::Right:
            {
                for (int r = 0; r <= maxRowIndex; r++)
                {
                    char c = map[r][maxColIndex];
                    for (int col = maxColIndex; col > 0; col--)
                        map[r][col] = map[r][col-1];
                    map[r][0] = c;
                }
                break;
            }          
            case Direction::Left:
            {
                for (int row = 0; row <= maxRowIndex; row++)
                {
                    char c = map[row][0];
                    for (int col = 0; col <= maxColIndex; col++)
                        map[row][col] = map[row][col+1];
                    map[row][maxColIndex] = c;
                }
                break;
            }
        }
    }

    const std::vector<std::string>& GetMap() 
    {
        return map;
    }

    private:
    std::vector<std::string> map;
    int maxRowIndex{};
    int maxColIndex{};
};

int main()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int maxRow = w.ws_row;
    int maxCol = w.ws_col;

    Console console{};
    Input input{};
    Frame frame{};
    Render render{};
    Game game{};

    console.hideCursor();
    console.clearScreen();
    input.setup();

    ProgramState state;
    while (1)
    {
        frame.limit();

        state = input.handle();
        if (state == ProgramState::Stop)
            break;

        game.SetDirection(state);
        game.Update();

        render.Draw(game.GetMap());
    }

    console.showCursor();
    input.clean();
    return 0;
}