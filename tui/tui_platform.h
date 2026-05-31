#ifndef TUI_PLATFORM
#define TUI_PLATFORM

#include "tui_utils.h"

#if defined(_WIN32) || defined(_WIN64)
	#define TUI_WINDOWS
	// windows es raro
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif //defined(_WIN32) || defined(_WIN64)

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>

 //TODO: is this portable?
#include <poll.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

typedef enum {
	MOUSE_BUTTON_NONE       = 0,
	MOUSE_BUTTON_LEFT       = 1,
	MOUSE_BUTTON_MIDDLE     = 2,
	MOUSE_BUTTON_RIGHT      = 3,
	MOUSE_BUTTON_WHEEL_UP   = 4,
	MOUSE_BUTTON_WHEEL_DOWN = 5,
	//TODO: get more from here https://docs.godotengine.org/en/stable/classes/class_%40globalscope.html#enum-globalscope-mousebutton
} MouseButton;

typedef enum {
	KEY_MOD_NONE  = 0x00,
	KEY_MOD_SHIFT = 0x02,
	KEY_MOD_CTRL  = 0x04,
	KEY_MOD_ALT   = 0x08,
} ModKeys;

typedef enum {
    KEY_NONE   = 0,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ESCAPE,
    KEY_TAB,
    KEY_BACKTAB, //shift + tab
    KEY_BACKSPACE,
    KEY_ENTER,
    KEY_DELETE,
    KEY_HOME,
    KEY_END,
    KEY_PAGEUP,
    KEY_PAGEDOWN,
    KEY_SHIFT,
    KEY_CTRL,
    KEY_ALT,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,

    //nums
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    //alpha
    KEY_A = 'a',
    KEY_B = 'b',
    KEY_C = 'c',
    KEY_D = 'd',
    KEY_E = 'e',
    KEY_F = 'f',
    KEY_G = 'g',
    KEY_H = 'h',
    KEY_I = 'i',
    KEY_J = 'j',
    KEY_K = 'k',
    KEY_L = 'l',
    KEY_M = 'm',
    KEY_N = 'n',
    KEY_O = 'o',
    KEY_P = 'p',
    KEY_Q = 'q',
    KEY_R = 'r',
    KEY_S = 's',
    KEY_T = 't',
    KEY_U = 'u',
    KEY_V = 'v',
    KEY_W = 'w',
    KEY_X = 'x',
    KEY_Y = 'y',
    KEY_Z = 'z',
    KEY_SPACE = ' ',
    KEY_MAX,
} Key;

typedef enum {
	INPUT_NONE,
	INPUT_KEY,
	INPUT_MOUSE_BUTTON,
	// INPUT_MOUSE_MOTION,
	INPUT_WINDOW_RESIZE,
} InputType;

typedef struct {
	Key      key;     //"a" == "A" use KEY_A here!
	uint32_t unicode; //"a" != "A" this is for typing
	bool     ctrl;
	bool     alt;
	bool     shift;
} InputEventKey;

typedef struct {
	vec2i pos;
	MouseButton button_index;
	bool ctrl;
	bool alt;
	bool shift;
} InputEventMouseButton;

typedef struct {
	InputType input_type;
	union {
		InputEventKey key_event;
		InputEventMouseButton mouse_button_event;
		// InputEventMouseMotion mouse_motion_event;
	};
	bool consumed;
} InputEvent;

private constexpr int EVENT_QUEUE_MAX = 64;
typedef struct {
	InputEvent events[EVENT_QUEUE_MAX];
	int count;
} InputEventQueue;

extern InputEventQueue EVENT_QUEUE;
private void input_event_queue_push(InputEvent input_event);
private bool tui_poll_input(int timeout_ms);
private void tui_parse_input(void);

typedef bool (*ProcessInputEventFunction)(InputEvent);

//public API
// void tui_set_resize_callback(FunctionPointer on_resize);
void tui_init(void);
void tui_close(void);
vec2i tui_size(void);
void tui_write(const char *str);
void tui_write_format(const char *format, ...);
void tui_write_bytes(const uint8_t *bytes, uint8_t total_bytes);
void tui_input_read(double timeout_s); //NOTE: call this in the loop
void tui_input_process(ProcessInputEventFunction input_processor);

// IMPL ----------------------
#ifdef TUI_PLATFORM_IMPL

InputEventQueue EVENT_QUEUE = {};
private inline void input_event_queue_clear(){
	EVENT_QUEUE.count = 0;
}

private void input_event_queue_push(InputEvent input_event){
	if(EVENT_QUEUE.count >= EVENT_QUEUE_MAX){
		 return; //event queue full
	}
	EVENT_QUEUE.events[EVENT_QUEUE.count++] = input_event;
}

private InputEvent *input_event_queue_at(int index){
	assert(EVENT_QUEUE.count > 0);
    index                  = clamp(index, 0, EVENT_QUEUE.count);
    InputEvent *next_event = &(EVENT_QUEUE.events[index]);
	return next_event;
}

private void emit_resize_event(void){
	//TODO: throttle them...?
	InputEvent resize_event = {
        .input_type = INPUT_WINDOW_RESIZE,
	};
	input_event_queue_push(resize_event);
}

#ifdef TUI_WINDOWS

static HANDLE TUI_WIN_HANDLE_IN;
static HANDLE TUI_WIN_HANDLE_OUT;
static DWORD  TUI_WIN_ORIGINAL_IN_MODE;
static DWORD  TUI_WIN_ORIGINAL_OUT_MODE;

void tui_init(void){
	TUI_WIN_HANDLE_IN  = GetStdHandle(STD_INPUT_HANDLE);
    TUI_WIN_HANDLE_OUT = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleMode(TUI_WIN_HANDLE_IN,  &TUI_WIN_ORIGINAL_IN_MODE);
    GetConsoleMode(TUI_WIN_HANDLE_OUT, &TUI_WIN_ORIGINAL_OUT_MODE);

    DWORD in_mode = ENABLE_WINDOW_INPUT | ENABLE_EXTENDED_FLAGS;
    in_mode |= ENABLE_MOUSE_INPUT;
    SetConsoleMode(TUI_WIN_HANDLE_IN, in_mode);
    SetConsoleMode(TUI_WIN_HANDLE_OUT, TUI_WIN_ORIGINAL_OUT_MODE | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void tui_close(void){
	SetConsoleMode(TUI_WIN_HANDLE_IN,  TUI_WIN_ORIGINAL_IN_MODE);
    SetConsoleMode(TUI_WIN_HANDLE_OUT, TUI_WIN_ORIGINAL_OUT_MODE);

	tui_write("\033[?1000l"); //disabel mouse
    tui_write("\033[?1006l"); //disable SGR
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &TERMIOS_ORIGINAL);
}

private bool tui_poll_input(int timeout_ms){
    DWORD wait_result = WaitForSingleObject(
        TUI_WIN_HANDLE_IN,
        timeout_ms < 0 ? 0 : (DWORD)timeout_ms
    );

    if (wait_result != WAIT_OBJECT_0) {
        return false;
    }

    DWORD count = 0;
    BOOL success = GetNumberOfConsoleInputEvents(
        TUI_WIN_HANDLE_IN, &count
    );
    return (success && count > 0);
}

private void tui_parse_input(void){
	INPUT_RECORD input_record;
    DWORD count;
    auto success = ReadConsoleInputW(tui__hin, &input_record, 1, &count);

    if(success && count > 0){

    }
    //TODO: catch EventType == WINDOW_BUFFER_SIZE_EVENT and fire the resize event!

    //TODO: parse the record and push the event
    // https://learn.microsoft.com/en-us/windows/console/input-record-str
}

vec2i tui_size(void){
	vec2i size = {.x = 80, .y = 24}; //default

	CONSOLE_SCREEN_BUFFER_INFO screen_info;
    auto success = GetConsoleScreenBufferInfo(
    	TUI_WIN_HANDLE_OUT, &screen_info
	);

	if (success != 0) return size;

    size.x = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    size.y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    return size;
}

void tui_write(const char *str){
    DWORD written;
    WriteConsoleA(
    	tui_win_handle_out,
    	str,
    	(DWORD)strlen(buf),
    	&written,
    	NULL
	);
}

void tui_write_bytes(const uint8_t *bytes, uint8_t total_bytes){
	DWORD written;
	WriteFile(
		tui_win_handle_out,
		bytes,
		(DWORD)total_bytes,
		&written,
		NULL
	);
}
▔

#else // LINUX

#include <signal.h>
#include <sys/signalfd.h>

private int SIGNAL_FD_EVENT;
private void tui_setup_sigwinch(void){
	// NOTE: first we block Block the SIGWINCH signal
	//       so the default async handler never triggers
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGWINCH);
    auto result = sigprocmask(SIG_BLOCK, &mask, NULL);
    assert(result >= 0); //error

    // Create the signalfd to translate the signal into an FD event
    // SFD_NONBLOCK: ensures we don't get stuck on the read() function later
    // SFD_CLOEXEC:  stops FD from leaking
    SIGNAL_FD_EVENT = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    assert(SIGNAL_FD_EVENT >= 0);
}

private struct termios TERMIOS_ORIGINAL;
void tui_init(void){
	tcgetattr(STDIN_FILENO, &TERMIOS_ORIGINAL);
	struct termios raw = TERMIOS_ORIGINAL;

	//look at all this bullshit
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |=  (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN]  = 0;
	raw.c_cc[VTIME] = 0;

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

	tui_write("\033[?1000h"); //mouse suppo
    tui_write("\033[?1006h"); //SGR extended coords
    tui_write("\033[?25l");   //ocultar cursor

    tui_setup_sigwinch();
}

void tui_close(void){
	tui_write("\033[?25h");   //mostrar cursor
	tui_write("\033[?1000l"); //disabel mouse
    tui_write("\033[?1006l"); //disable SGR
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &TERMIOS_ORIGINAL);
}

private bool tui_poll_input(int timeout_ms){
	struct pollfd polls[2] = {
		[0] = { .fd = STDIN_FILENO,    .events = POLLIN }, //input
		[1] = { .fd = SIGNAL_FD_EVENT, .events = POLLIN }, //signals
	};
	int events_count = poll(polls, 2, timeout_ms);

	if (events_count <= 0) return false;

	if(polls[1].revents & POLLIN){ //resize event
		struct signalfd_siginfo fd_siginfo;
		ssize_t signal_fd_info = read(
			SIGNAL_FD_EVENT, &fd_siginfo, sizeof(struct signalfd_siginfo)
		);
		if(signal_fd_info == sizeof(struct signalfd_siginfo)){
			if(fd_siginfo.ssi_signo == SIGWINCH){
				emit_resize_event();
			}
		}
	}

	//other events
	return (polls[0].revents & POLLIN);
}


vec2i tui_size(void){
	vec2i size = {.x = 80, .y = 24}; //default
	struct winsize ws;
	auto success = ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

	if (success != 0) return size;

    if (ws.ws_col > 0) size.x = ws.ws_col;
    if (ws.ws_row > 0) size.y = ws.ws_row;

    return size;
}

void tui_write(const char *str){
    write(STDOUT_FILENO, str, strlen(str));
}

void tui_write_bytes(const uint8_t *bytes, uint8_t total_bytes){
	write(STDOUT_FILENO, bytes, total_bytes);
}

typedef enum {
	PARSE_START,
	PARSE_ESCAPE,
	PARSE_CONTROL_SEQUENCE,
	PARSE_UTF8,
} InputParseState;
private InputParseState input_state = PARSE_START;
private double          last_input_time = 0.0; //timeout para ESC y utf8

private void emit_special_key(Key key){
	InputEvent input_event = {
        .input_type    = INPUT_KEY,
        .key_event.key = key,
	};
	input_event_queue_push(input_event);
}

private void emit_key(uint32_t unicode, bool alt){
    //for single byte ascii
    bool ctrl  = (unicode < 32);
    bool shift = (unicode < 128 && unicode != (uint32_t)tolower(unicode));
    Key key    = (Key)(
		//ctrl key removes "@" from the byte
		 tolower(ctrl ? '@' + unicode : unicode)
	);
	InputEvent input_event = {
        .input_type        = INPUT_KEY,
        .key_event.key     = key,
        .key_event.ctrl    = ctrl,
        .key_event.alt     = alt,
        .key_event.shift   = shift,
        .key_event.unicode = unicode,
	};
	input_event_queue_push(input_event);
}

private void emit_escape_sequence(const char *params, uint8_t final_byte){
	switch(final_byte){
	//arrow keys
	case 'A': emit_special_key(KEY_UP);      break;
	case 'B': emit_special_key(KEY_DOWN);    break;
	case 'C': emit_special_key(KEY_RIGHT);   break;
	case 'D': emit_special_key(KEY_LEFT);    break;
	case 'H': emit_special_key(KEY_HOME);    break;
	case 'F': emit_special_key(KEY_END);     break;
	case 'Z': emit_special_key(KEY_BACKTAB); break;

	//FKEYS
	case '~': {
		int code;
		sscanf(params, "%d", &code);
		switch(code){
		case 1:  emit_special_key(KEY_HOME);     break;
		//case 2:  emit_special_key(KEY_INSERT); break; //is this correct?
		case 3:  emit_special_key(KEY_DELETE);   break;
		case 4:  emit_special_key(KEY_END);      break;
		case 5:  emit_special_key(KEY_PAGEUP);   break;
		case 6:  emit_special_key(KEY_PAGEDOWN); break;
		case 11: emit_special_key(KEY_F1);       break;
		case 12: emit_special_key(KEY_F2);       break;
		case 13: emit_special_key(KEY_F3);       break;
		case 14: emit_special_key(KEY_F4);       break;
		case 15: emit_special_key(KEY_F5);       break;
		case 17: emit_special_key(KEY_F6);       break;
		case 18: emit_special_key(KEY_F7);       break;
		case 19: emit_special_key(KEY_F8);       break;
		case 20: emit_special_key(KEY_F9);       break;
		case 21: emit_special_key(KEY_F10);      break;
		case 23: emit_special_key(KEY_F11);      break;
		case 24: emit_special_key(KEY_F12);      break;
		//TODO: agregar mas...?
		}
		break;
	}

	//mouse
	//TODO:
	case 'M':
	case 'm':
	}
}

private void parse_next_byte(uint8_t byte){
    last_input_time = get_curr_time();
	//state machine for parsing ze bytten
    constexpr int  params_max         = 32;
    static    char params[params_max] = {};
    static    int  params_length      = 0;

    //utf8
    //TODO: could we somehow split the state machine and put the utf8 outside?
    static uint8_t utf8_bytes[4] = {};
    static uint8_t utf8_length_current = 0;
    static uint8_t utf8_length_expected = 0;

	switch(input_state){
	case PARSE_START:
		switch(byte){
		case '\033':
			input_state = PARSE_ESCAPE;
			last_input_time = get_curr_time();
			return;
		case '\r':
		case '\n':   emit_special_key(KEY_ENTER);     goto reset_state;
		//TODO: backspace is not working properly, getting detected as KEY_DELETE
		case 127:
		case '\b':   emit_special_key(KEY_BACKSPACE); goto reset_state;
		case '\t':   emit_special_key(KEY_TAB);       goto reset_state;
		//TODO: more keys..?
		default:
            uint8_t length = utf8_char_length(byte);
            if(length <= 1){ //ascii or empty
                emit_key(byte, false);
                goto reset_state;
            }
            //utf8 string!
            input_state          = PARSE_UTF8;
            last_input_time      = get_curr_time();
            utf8_bytes[0]        = byte;
            utf8_length_current  = 1; //accumulates
            utf8_length_expected = length;
            return;
		}
		break;

	case PARSE_ESCAPE:
		if(byte == '['){
			input_state = PARSE_CONTROL_SEQUENCE;
			params_length = 0;
			return;
		}else{
			emit_key(byte, true);
			goto reset_state;
		}

	case PARSE_CONTROL_SEQUENCE:
		if(byte >= '0' && byte <= '?'){
			assert(params_length < params_max);
			params[params_length++] = byte;
			return;
		}else{
			params[params_length] = '\0'; //null termination
			emit_escape_sequence(params, byte);
			goto reset_state;
		}

    case PARSE_UTF8:
    	//add next byte
        utf8_bytes[utf8_length_current++] = byte;
        if(utf8_length_current < utf8_length_expected) return;
    	//utf8 completed!
        emit_key(utf8_pack(utf8_bytes), false);
        goto reset_state;
	}

	reset_state:
	input_state          = PARSE_START;
	params_length        = 0;
	utf8_length_current  = 0;
	utf8_length_expected = 0;
}

private void tui_parse_input(void){
	char input_buffer[64];
	int bytes_written = (int)read(
		STDIN_FILENO, input_buffer, sizeof(input_buffer)
	);

	//IMPORTANT: bytes_written can return -1, we check to
	//           avoid infinite for loop
	if(bytes_written <= 0){
		return;
	}

	for (int i = 0; i < bytes_written; i++){
		parse_next_byte(input_buffer[i]);
	}
}

#endif // LINUX / WINDOWS


void tui_write_format(const char *format, ...){
	if (!format) return;

    char buffer[1024]; //TODO: maybe something else..?
    va_list args;
    va_start(args, format);
    int length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    assert(length < (int)sizeof(buffer)); //check never truncated?
    if (length <= 0) return;

    tui_write(buffer);
}

void tui_input_read(double timeout_s){
	//TODO: a bit hacky now...
	input_event_queue_clear();
	auto now = get_curr_time();
	if(input_state != PARSE_START && (now - last_input_time) > 0.05){
		//if we were waiting for escape to continue and it didnt, emit escape
        if(input_state == PARSE_ESCAPE) emit_special_key(KEY_ESCAPE);
        //otherwise it probably means we got an unfinished utf8, which should never
        //realistically happen, so we just ignore it for now.
		input_state = PARSE_START;
	}
	if (!tui_poll_input(timeout_s * 1000)) return;
	tui_parse_input();
}

void tui_input_process(ProcessInputEventFunction input_processor){
	// if the input processing function retuns false,
	// we do not consume the event, so it "bubbles up"
	// returning true from any of those functions is
	// effectively like calling stopPropagation() in JS.
	for(int i = 0; i < EVENT_QUEUE.count; i++){
		auto input_event = input_event_queue_at(i);
		if(input_event->consumed) continue;
		input_event->consumed = input_processor(*input_event);
	}
}


#endif //TUI_PLATFORM_IMPL
#endif //TUI_PLATFORM
