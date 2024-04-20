#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <tgmath.h>
#include <time.h>
#include <limits>

#define BRIGHT_RED 207
#define BRIGHT_GREEN 175
#define INV_BRIGHT_RED 252

enum INPUTS {LEFT = 1, RIGHT = 2, UP = 3, DOWN = 4, DNULL = 0};
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
int txtcolors[] = {15, 9, 10, 12, 14, 5, 11, 13, 8}, bombs_flagged = 0, correct_mines = 0;
// Let's declare the class for a field
class Field {

    public:

    int bomb_amount;
    bool bomb_innit = false;
    bool field_shown = false;
    bool field_flagged = false;
};

class GmTemplate {

    public:

    std::string name = "bs_template";
    int gm_width = 10;
    int gm_height = 10;
    int gm_bomb_amount = 10;
    int color_identifier = 15;
};

// Functions below the main function
int get_integer();
float clamp(float val, float low_boundary, float high_boundary);
void array_randomize(int arr[], int arr_size);
void array_cout(int arr[], int arr_size);
void analyze_bombs(Field field_arr[], int field_position, int brd_width, int brd_height);
void gotoxy(int x, int y);
void update_cursor(int pos, int old_pos, int brd_width, int color);
void reset_color() { SetConsoleTextAttribute(hConsole, 15); }
void show_field(Field field_arr[], int pos, int brd_width, int brd_height);
void show_surrounding_fields(Field field_arr[], int pos, int brd_width, int brd_height);
void flag_field(Field field_arr[], int pos, int brd_width);
void declare_template(GmTemplate temp_arr[], int temp_pos, std::string parname, int parwidth, int parheight, int parbomb, int parcolor);
void screen_refresh(Field field_arr[], int width, int height);
INPUTS detect_input();

int main() {

    // Randomize the seed and declare stuff yknow
    system("cls");
    srand(time(0));
    reset_color();

    const int sleep_time = 10, hold_module = 7;
    float seconds = 0;
    int width, height, bomb_amount, game_cursor = 0, old_game_cursor = 1, current_frame = 0, return_time = 2, b_time = 0, f_time = 0;
    int cursor_color = BRIGHT_GREEN, user_input = 0, template_chosen, seconds_length;
    bool game_loop = true, cancel_movement = false, win = false;

    INPUTS move_receiver = DNULL, old_move_receiver = DNULL;

    std::string mode_string_1, mode_string_2, timer_string, version = "1.0.0";

    // Set Game Templates
    const int template_amount = 5;
    GmTemplate templates[template_amount];

    declare_template(templates, 0, "Easy", 10, 10, 10, 9);
    declare_template(templates, 1, "Medium", 16, 16, 40, 10);
    declare_template(templates, 2, "Hard", 30, 16, 99, 14);
    declare_template(templates, 3, "Expert", 36, 20, 144, 12);
    declare_template(templates, 4, "Master", 42, 24, 202, BRIGHT_RED);

    // Declare pointers for dynamic arrays
    Field *fields = NULL;
    int *bomb_pos = NULL;

    // Starting game title
    SetConsoleTextAttribute(hConsole,12);
    std::cout<<"                    _                                       \n";
    std::cout<<"                   | |                                      \n";
    std::cout<<"  ___ _ __ ___   __| |_____      _____  ___ _ __   ___ _ __ \n";
    std::cout<<" / __| '_ ` _ \\ / _` / __\\ \\ /\\ / / _ \\/ _ \\ '_ \\ / _ \\ '__|\n";
    std::cout<<"| (__| | | | | | (_| \\__ \\  V  V /  __/  __/ |_) |  __/ |   \n";
    std::cout<<" \\___|_| |_| |_|\\__,_|___/ \\_/\\_/ \\___|\\___| .__/ \\___|_|   \n";
    std::cout<<"                                           | |              \n";
    std::cout<<"                                           |_|              \n";
    // Site for the ASCII art: https://patorjk.com/software/taag/#p=display&f=Doom&t=cmdsweeper

    SetConsoleTextAttribute(hConsole,11);
    std::cout<<"\nV. "<<version<<", by @LemonpieGBS\n\n";
    reset_color();

    // Select a custom size or template
    std::cout<<"Would you like to select a template for the game or set the size yourself?";
    std::cout<<"\n #1. Use templates (Easy / Hard / etc.)";
    std::cout<<"\n #2. Play a Custom Game\n";

    SetConsoleTextAttribute(hConsole,11);
    std::cout<<"\nEnter your preferred option: ";
    reset_color();
    user_input = get_integer();

    // Cin Failsafe
    //if(!std::cin) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); user_input = -1; }

    while(user_input != 1 && user_input != 2) {
        SetConsoleTextAttribute(hConsole,11);
        std::cout<<"\nPlease enter a valid option: ";
        reset_color();
        user_input = get_integer();
        
    }
    system("cls");

    if(user_input == 1) {

        SetConsoleTextAttribute(hConsole,11);
        std::cout<<"Available templates: \n\n";
        reset_color();

        // Display all the templates
        for(int i = 0; i < template_amount; i++) {
            std::cout<<" #"<<(i+1)<<". ";
            SetConsoleTextAttribute(hConsole,templates[i].color_identifier);
            std::cout<<templates[i].name;
            reset_color();
            std::cout<<" ("<<templates[i].gm_width<<"x"<<templates[i].gm_height<<", "<<templates[i].gm_bomb_amount<<" mines)\n";
        }
        SetConsoleTextAttribute(hConsole,12);
        std::cout<<"\n# WARNING: Some game sizes may not be available for your display size, in case graphic glitches arise, make sure to zoom out the console and press F to refresh the display\n";
        SetConsoleTextAttribute(hConsole,11);
        std::cout<<"# TIP: You can zoom out the console with CNTRL + Scroll Wheel\n\n";

        reset_color();

        std::cout<<"\nSelect a template: ";
        user_input = get_integer();

        // While not a valid option, the user must input a valid number
        while( !(user_input >= 1 && user_input <= template_amount)) {

            std::cout<<"\nPlease enter a valid option: ";
            user_input = get_integer();
    
        }

        // Apply template
        width = templates[user_input - 1].gm_width;
        height = templates[user_input - 1].gm_height;
        bomb_amount = templates[user_input - 1].gm_bomb_amount;
        template_chosen = user_input - 1;

    } else if(user_input == 2) {

        SetConsoleTextAttribute(hConsole,11);
        std::cout<<"Custom game setup: \n";

        // Let's set the dimensions for the game
        SetConsoleTextAttribute(hConsole,14);
        std::cout<<"\n Specify the Width of the game: ";
        reset_color();
        width = get_integer();

        SetConsoleTextAttribute(hConsole,14);
        std::cout<<"\n Specify the Height of the game: ";
        reset_color();
        height = get_integer();

        SetConsoleTextAttribute(hConsole,14);
        std::cout<<"\n Specify the Number of Bombs: ";
        reset_color();
        bomb_amount = get_integer();

        template_chosen = -1;

        SetConsoleTextAttribute(hConsole,12);
        std::cout<<"\n# WARNING: Some game sizes may not be available for your display size, in case graphic glitches arise, make sure to zoom out the console and press F to refresh the display\n";
        SetConsoleTextAttribute(hConsole,11);
        std::cout<<"# TIP: You can zoom out the console with CNTRL + Scroll Wheel\n\n";

        reset_color();

        system("Pause");

    }

    reset_color();
    // Let's clamp width and height just in case
    width = (width < 10) ? 10 : width;
    height = (height < 10) ? 10 : height;

    // Let's declare the total area & clamp the bomb amount
    int ttl_area = width*height;
    bomb_amount = clamp((float) bomb_amount,1.0,(float) width*height - 1);

    fields = new Field[ttl_area];
    bomb_pos = new int[ttl_area];

    // Let's set up the bomb randomizing process
    for(int i = 0; i < ttl_area; i++) { bomb_pos[i] = i; }
    array_randomize(bomb_pos,ttl_area);

    // Now we just give the selected fields some bombs
    for(int i = 0; i < bomb_amount; i++) { fields[bomb_pos[i]].bomb_innit = true; }
    delete[] bomb_pos;

    // And we now calculate each field's bomb amount
    for(int i = 0; i < ttl_area; i++) { analyze_bombs(fields,i,width,height); }

    // Clear the Screen and draw the game!
    system("cls");
    int ttl_field_roll = 0;
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            std::cout<<"[ ]";
            ttl_field_roll++;
        }
        std::cout<<"\n";
    }
    
    // Declare the Mode Text
    mode_string_1 = (template_chosen == -1) ? "Custom" : templates[template_chosen].name;
    mode_string_2 = " (" + std::to_string(width) + "x" + std::to_string(height) + ")";

    mode_string_1 = "[" + mode_string_1 + "]" + mode_string_2;

    // Update the cursor and start the game loop
    update_cursor(game_cursor,old_game_cursor,width,cursor_color);

    // Start the timer
    time_t start = time(0);

    while(game_loop) {

        seconds = (seconds < 9999) ? difftime(time(0), start) : 9999;

        // Detect inputs from the move receiver
        old_move_receiver = move_receiver;
        move_receiver = detect_input();

        // Detect if a hold is happening
        if(old_move_receiver == move_receiver && move_receiver != DNULL) {
            // Update the current frame and reset it when it gets past the hold module (cycle)
            current_frame = (current_frame >= hold_module) ? 0 : current_frame + 1;
            cancel_movement = (current_frame != 0);
        } else { current_frame = -hold_module; cancel_movement = false; }

        if(!cancel_movement && (move_receiver != DNULL || old_move_receiver != DNULL)) {

            // Assign old_game_cursor to game_cursor before moving
            old_game_cursor = game_cursor;

            // Move the game_cursor based on the input
            switch(move_receiver) {
                case(LEFT): game_cursor = (game_cursor % width == 0) ? game_cursor : game_cursor - 1; break;
                case(RIGHT): game_cursor = ( (game_cursor + 1) % width == 0) ? game_cursor : game_cursor + 1; break;
                case(UP): game_cursor = (game_cursor < width) ? game_cursor : game_cursor - width; break;
                case(DOWN): game_cursor = (game_cursor >= ttl_area - width) ? game_cursor : game_cursor + width; break;
                default: break;
            }

        }
        cursor_color = BRIGHT_GREEN;

        // Detect stuff happening when enter is pressed
        if(GetKeyState(VK_RETURN) < 0) {
            return_time++;
            cursor_color = BRIGHT_RED; 
        } else { return_time = 0; }

        if(GetKeyState('B') < 0) {
            b_time++;
            cursor_color = BRIGHT_RED;
        } else { b_time = 0; }

        if(GetKeyState('F') < 0) {
            f_time++;
        } else { f_time = 0; }

        // Update cursor
        update_cursor(game_cursor,old_game_cursor,width,cursor_color);

        // If enter is pressed for the first frame, it will mine
        if(return_time == 1) {

            if(!fields[game_cursor].field_flagged) show_field(fields,game_cursor,width,height);

            if(fields[game_cursor].bomb_innit && !fields[game_cursor].field_flagged) {
                for(int i = 0; i < ttl_area; i++) {
                    if(fields[i].bomb_innit) {
                        int x_coord = i % width;
                        int y_coord = ceil( (float) (i + 1) / width) - 1;

                        // Reveal a mine :(
                        SetConsoleTextAttribute(hConsole, BRIGHT_RED);
                        gotoxy(x_coord*3,y_coord);
                        std::cout<<"[X]";
                    } else if(!fields[i].bomb_innit && fields[i].field_flagged) {
                        int x_coord = i % width;
                        int y_coord = ceil( (float) (i + 1) / width) - 1;

                        // Reveal a mine :(
                        SetConsoleTextAttribute(hConsole, INV_BRIGHT_RED);
                        gotoxy(x_coord*3,y_coord);
                        std::cout<<"[B]";
                    }
                }
                game_loop = false;
            }

        }

        // If B is pressed for the first frame, it will flag
        if(b_time == 1) { flag_field(fields,game_cursor,width); }
        
        // If F is pressed refresh the screen
        if(f_time == 1 && game_loop) { screen_refresh(fields,width,height); }

        // Check win condition
        if(correct_mines == ttl_area - bomb_amount) {
            game_loop = false;
            win = true;
            bombs_flagged = bomb_amount;

            reset_color();
            int x_coord = game_cursor % width;
            int y_coord = ceil( (float) (game_cursor + 1) / width) - 1;

            gotoxy(x_coord*3,y_coord);
            std::cout<<"[";
            gotoxy(x_coord*3 + 2,y_coord);
            std::cout<<"]";
        }

        // Let's display the text below
        reset_color();

        // Mine Amount Text [Centered Left, 1st Line]
        gotoxy(0,height+1);
        std::cout<<bombs_flagged<<"/"<<bomb_amount<<" mines";

        // Time Text [Center Right, 1st Line]
        seconds_length = 4 - std::to_string((int) seconds).length();
        timer_string = "Time: ";
        for(int i = 1; i <= seconds_length; i++) { timer_string.append("0"); }
        timer_string.append(std::to_string((int) seconds));

        gotoxy(width*3 - timer_string.length(),height+1);
        std::cout<<timer_string;

        // Difficulty Text [Centered, 2nd Line]
        if(template_chosen != -1) SetConsoleTextAttribute(hConsole,templates[template_chosen].color_identifier);
        else SetConsoleTextAttribute(hConsole, (width + height > 64 && (float) bomb_amount / (float) ttl_area >= 0.2) ? BRIGHT_RED : 8);
        gotoxy( (width*3) / 2 - mode_string_1.length()/2,height+2);
        std::cout<<mode_string_1;

        // Let the program sleep between frames
        Sleep(sleep_time);
    }

    // Display final message
    reset_color();

    // Set everything back to normal yknow
    if(win) {

        SetConsoleTextAttribute(hConsole,14);
        std::string game_won = "YOU HAVE WON THE MINESWEEPER!";
        gotoxy((width*3)/2 - game_won.length()/2,height+4);
        std::cout<<game_won;

        for(int i = 0; i < 8; i++) {
            for(int j = 0; j < ttl_area; j++) {
                if(!fields[j].bomb_innit) continue;

                int x_coord = j % width;
                int y_coord = ceil( (float) (j + 1) / width) - 1;

                // Reveal a mine :(
                SetConsoleTextAttribute(hConsole, (i % 2 == 0) ? INV_BRIGHT_RED : BRIGHT_RED);
                gotoxy(x_coord*3,y_coord);
                std::cout<<"[!]";
            }
            gotoxy(0,height+4);
            Sleep(500);
        }

        reset_color();
    }
    else {
        SetConsoleTextAttribute(hConsole,12);
        std::string game_lost = "Better luck next time! :(";
        gotoxy((width*3)/2 - game_lost.length()/2,height+4);
        std::cout<<game_lost;
        reset_color();
    }

    gotoxy(0,height+6);
    Sleep(2000);
    system("Pause");

    delete[] fields;

}

int get_integer() {

    int return_integer;
    using namespace std;

    while(!(cin>>return_integer)) {
        cout<<"\n# ERR: Please enter a valid integer value: ";
        cin.clear();

        #undef max
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return return_integer;
}

float clamp(float val, float low_boundary, float high_boundary) {

    if(val > high_boundary) val = high_boundary;
    else if(val < low_boundary) val = low_boundary;

    return val;
}

void array_randomize(int arr[], int arr_size) {

    int shufflecontainer = 0;
    for(int i = 0; i < arr_size; i++) {

        int j = rand() % arr_size;
        shufflecontainer = arr[j];

        arr[j] = arr[i];
        arr[i] = shufflecontainer;
    }
}

void array_cout(int arr[], int arr_size) {

    std::cout<<"Array Output: ";
    for(int i = 0; i < arr_size; i++) {

        std::string add = (i < arr_size - 1) ? ", " : ".\n";
        std::cout<<arr[i]<<add;
    }
}

void analyze_bombs(Field field_arr[], int field_position, int brd_width, int brd_height) {

    // Detect the limits
    bool alt_right = false, alt_left = false, alt_up = false, alt_down = false;
    if(field_position % brd_width == 0) alt_left = true;
    if((field_position + 1) % brd_width == 0) alt_right = true;
    if(field_position < brd_width) alt_up = true;
    if(field_position >= brd_width*brd_height - brd_width) alt_down = true;

    int bomb_amounts = 0;

    if(!alt_right) {
        bomb_amounts += field_arr[field_position + 1].bomb_innit;
        if(!alt_down) { bomb_amounts += field_arr[field_position + brd_width + 1].bomb_innit; }
        if(!alt_up) { bomb_amounts += field_arr[field_position - brd_width + 1].bomb_innit; }
    }

    if(!alt_left) {
        bomb_amounts += field_arr[field_position - 1].bomb_innit;
        if(!alt_down) { bomb_amounts += field_arr[field_position + brd_width - 1].bomb_innit; }
        if(!alt_up) { bomb_amounts += field_arr[field_position - brd_width - 1].bomb_innit; }
    }

    if(!alt_down) { bomb_amounts += field_arr[field_position + brd_width].bomb_innit; }
    if(!alt_up) { bomb_amounts += field_arr[field_position - brd_width].bomb_innit; }

    field_arr[field_position].bomb_amount = bomb_amounts;
}

void gotoxy(int x, int y) { 
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(output, {(short) x,(short) y});
}

void update_cursor(int pos, int old_pos, int brd_width, int color) {
    int x_coord = pos % brd_width;
    int y_coord = ceil( (float) (pos + 1) / brd_width) - 1;

    int old_x_coord = old_pos % brd_width;
    int old_y_coord = ceil( (float) (old_pos + 1) / brd_width) - 1;

    SetConsoleTextAttribute(hConsole, color);
    gotoxy(x_coord*3,y_coord);
    std::cout<<"[";
    gotoxy(x_coord*3 + 2,y_coord);
    std::cout<<"]";

    reset_color();
    if(pos != old_pos) {
        gotoxy(old_x_coord*3,old_y_coord);
        std::cout<<"[";
        gotoxy(old_x_coord*3 + 2,old_y_coord);
        std::cout<<"]";
    }
}

void show_field(Field field_arr[], int pos, int brd_width, int brd_height) {

    if(field_arr[pos].field_shown) { return; }

    int x_coord = pos % brd_width;
    int y_coord = ceil( (float) (pos + 1) / brd_width) - 1;
    field_arr[pos].field_shown = true;

    if(field_arr[pos].bomb_innit) {

        // Reveal a mine :(
        SetConsoleTextAttribute(hConsole, BRIGHT_RED);
        gotoxy(x_coord*3,y_coord);
        std::cout<<"[X]";

    } else if(field_arr[pos].bomb_amount == 0) {
        
        // Reveal all surrounding area if space is 0
        gotoxy(x_coord*3 + 1,y_coord);
        std::cout<<"-";
        show_surrounding_fields(field_arr,pos,brd_width,brd_height);
        correct_mines++;

    } else {

        // Reveal the numbah
        SetConsoleTextAttribute(hConsole, txtcolors[field_arr[pos].bomb_amount]);
        gotoxy(x_coord*3 + 1,y_coord);
        std::cout<<field_arr[pos].bomb_amount;
        correct_mines++;
    }

    if(field_arr[pos].field_flagged) { field_arr[pos].field_flagged = false; bombs_flagged--; }

    reset_color();
}

void show_surrounding_fields(Field field_arr[], int pos, int brd_width, int brd_height) {

    int x_coord = pos % brd_width;
    int y_coord = ceil( (float) (pos + 1) / brd_width) - 1;

    bool alt_right, alt_up, alt_down, alt_left;
    if(x_coord == 0) alt_left = true;
    if(x_coord == brd_width - 1) alt_right = true;
    if(y_coord == 0) alt_up = true;
    if(y_coord == brd_height - 1) alt_down = true;

    if(!alt_right) {
        show_field(field_arr,pos+1,brd_width,brd_height);
            
        if(!alt_down) { show_field(field_arr,pos+brd_width+1,brd_width,brd_height); }
        if(!alt_up) { show_field(field_arr,pos-brd_width+1,brd_width,brd_height); }
    }

    if(!alt_left) {
        show_field(field_arr,pos-1,brd_width,brd_height);
        if(!alt_down) { show_field(field_arr,pos+brd_width-1,brd_width,brd_height); }
        if(!alt_up) { show_field(field_arr,pos-brd_width-1,brd_width,brd_height); }
    }

    if(!alt_down) { show_field(field_arr,pos+brd_width,brd_width,brd_height); }
    if(!alt_up) { show_field(field_arr,pos-brd_width,brd_width,brd_height); }
}

void flag_field(Field field_arr[], int pos, int brd_width) {

    if(field_arr[pos].field_shown) { return; }

    int x_coord = pos % brd_width;
    int y_coord = ceil( (float) (pos + 1) / brd_width) - 1;

    field_arr[pos].field_flagged = !field_arr[pos].field_flagged;
    if(field_arr[pos].field_flagged) {
        SetConsoleTextAttribute(hConsole, BRIGHT_RED);
        gotoxy(x_coord*3 + 1,y_coord);
        std::cout<<"B";
        bombs_flagged++;
    } else {
        reset_color();
        gotoxy(x_coord*3 + 1,y_coord);
        std::cout<<" ";
        bombs_flagged--;
    }
}

void declare_template(GmTemplate temp_arr[], int temp_pos, std::string parname, int parwidth, int parheight, int parbomb, int parcolor) {
    temp_arr[temp_pos].name = parname;
    temp_arr[temp_pos].gm_width = parwidth;
    temp_arr[temp_pos].gm_height = parheight;
    temp_arr[temp_pos].gm_bomb_amount = parbomb;
    temp_arr[temp_pos].color_identifier = parcolor;
}

void screen_refresh(Field field_arr[], int width, int height) {
    gotoxy(0,0);
    reset_color();
    system("cls");

    int ttl_field_roll = 0, x_coord, y_coord;
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {

            x_coord = ttl_field_roll % width;
            y_coord = ceil( (float) (ttl_field_roll + 1) / width) - 1;

            gotoxy(x_coord*3,i);
            std::cout<<"[ ]";

            if(! field_arr[ttl_field_roll].field_shown) {
                
                if(field_arr[ttl_field_roll].field_flagged) {

                    SetConsoleTextAttribute(hConsole, BRIGHT_RED);
                    gotoxy(x_coord*3 + 1,y_coord);
                    std::cout<<"B";
                }
            } else {

                gotoxy(x_coord*3 + 1,y_coord);
                if(field_arr[ttl_field_roll].bomb_amount != 0) {
                    SetConsoleTextAttribute(hConsole, txtcolors[field_arr[ttl_field_roll].bomb_amount]);
                    std::cout<<field_arr[ttl_field_roll].bomb_amount;
                } else std::cout<<"-";
            }

            ttl_field_roll++;
            reset_color();
        }
        std::cout<<"\n";
    }
}

INPUTS detect_input() {

    if(GetKeyState('A') < 0) { return LEFT; }
    else if(GetKeyState('D') < 0) { return RIGHT; }
    else if(GetKeyState('W') < 0) { return UP; }
    else if(GetKeyState('S') < 0) { return DOWN; }
    return DNULL;
}