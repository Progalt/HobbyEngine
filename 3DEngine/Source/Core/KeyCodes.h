#pragma once

#include "../Core/Platform.h"


    enum class MouseButton
    {
        Unkown,

        Left,
        Middle,
        Right
    };

    enum class KeyCode
    {
        Unknown = 0,

        Return,
        Escape,
        Backspace,
        Tab,
        Space,
        Exclaim,
        DoubleQuote,
        Hash,
        Percent,
        Dollar,
        Ampersand,
        Quote,
        LeftParen,
        RightParen,
        Asterisk,
        Plus,
        Comma,
        Minus,
        Period,
        Slash,
        Key0,
        Key1,
        Key2,
        Key3,
        Key4,
        Key5,
        Key6,
        Key7,
        Key8,
        Key9,
        Colon,
        SemiColon,
        Less,
        Equals,
        Greater,
        Question,
        At,

        LeftBracket,
        BackSlash,
        RightBracket,
        Caret,
        Underscore,
        BackQuote,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,

        CapsLock,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,

        PrintScreen,
        ScrollLock,
        Pause,
        Insert,
        Home,
        PageUp,
        Delete,
        End,
        PageDown,
        Right,
        Left,
        Down,
        Up,

        __Count
    };




#include <SDL2/SDL.h>


    inline KeyCode TranslateVirtualKey(SDL_Keycode code)
    {
        switch (code)
        {
        case SDLK_UNKNOWN:
            return KeyCode::Unknown;
            break;
        case SDLK_BACKSPACE:
            return KeyCode::Backspace;
            break;
        case SDLK_TAB:
            return KeyCode::Tab;
            break;
        case SDLK_RETURN:
            return KeyCode::Return;
            break;
        case SDLK_ESCAPE:
            return KeyCode::Escape;
            break;
        case SDLK_SPACE:
            return KeyCode::Space;
            break;
        case SDLK_EXCLAIM:
            return KeyCode::Exclaim;
            break;
        case SDLK_QUOTEDBL:
            return KeyCode::DoubleQuote;
            break;
        case SDLK_HASH:
            return KeyCode::Hash;
            break;
        case SDLK_DOLLAR:
            return KeyCode::Dollar;
            break;
        case SDLK_PERCENT:
            return KeyCode::Percent;
            break;
        case SDLK_AMPERSAND:
            return KeyCode::Ampersand;
            break;
        case SDLK_QUOTE:
            return KeyCode::Quote;
            break;
        case SDLK_LEFTPAREN:
            return KeyCode::LeftParen;
            break;
        case SDLK_RIGHTPAREN:
            return KeyCode::RightParen;
        case SDLK_ASTERISK:
            return KeyCode::Asterisk;
            break;
        case SDLK_PLUS:
            return KeyCode::Plus;
            break;
        case SDLK_MINUS:
            return KeyCode::Minus;
            break;
        case SDLK_PERIOD:
            return KeyCode::Period;
            break;
        case SDLK_SLASH:
            return KeyCode::Slash;
            break;
        case SDLK_0:
            return KeyCode::Key0;
            break;
        case SDLK_1:
            return KeyCode::Key1;
            break;
        case SDLK_2:
            return KeyCode::Key2;
            break;
        case SDLK_3:
            return KeyCode::Key3;
            break;
        case SDLK_4:
            return KeyCode::Key4;
            break;
        case SDLK_5:
            return KeyCode::Key5;
            break;
        case SDLK_6:
            return KeyCode::Key6;
            break;
        case SDLK_7:
            return KeyCode::Key7;
            break;
        case SDLK_8:
            return KeyCode::Key8;
            break;
        case SDLK_9:
            return KeyCode::Key9;
            break;
        case SDLK_COLON:
            return KeyCode::Colon;
            break;
        case SDLK_SEMICOLON:
            return KeyCode::SemiColon;
            break;
        case SDLK_LESS:
            return KeyCode::Less;
            break;
        case SDLK_EQUALS:
            return KeyCode::Equals;
            break;
        case SDLK_GREATER:
            return KeyCode::Greater;
            break;
        case SDLK_QUESTION:
            return KeyCode::Question;
            break;
        case SDLK_AT:
            return KeyCode::At;
            break;
        case SDLK_LEFTBRACKET:
            return KeyCode::LeftBracket;
            break;
        case SDLK_BACKSLASH:
            return KeyCode::BackSlash;
            break;
        case SDLK_RIGHTBRACKET:
            return KeyCode::RightBracket;
            break;
        case SDLK_CARET:
            return KeyCode::Caret;
            break;
        case SDLK_UNDERSCORE:
            return KeyCode::Underscore;
            break;
        case SDLK_BACKQUOTE:
            return KeyCode::BackQuote;
            break;
        case SDLK_a:
            return KeyCode::A;
            break;
        case SDLK_b:
            return KeyCode::B;
            break;
        case SDLK_c:
            return KeyCode::C;
            break;
        case SDLK_d:
            return KeyCode::D;
            break;
        case SDLK_e:
            return KeyCode::E;
            break;
        case SDLK_f:
            return KeyCode::F;
            break;
        case SDLK_g:
            return KeyCode::G;
            break;
        case SDLK_h:
            return KeyCode::H;
            break;
        case SDLK_i:
            return KeyCode::I;
            break;
        case SDLK_j:
            return KeyCode::J;
            break;
        case SDLK_k:
            return KeyCode::K;
            break;
        case SDLK_l:
            return KeyCode::L;
            break;
        case SDLK_m:
            return KeyCode::M;
            break;
        case SDLK_n:
            return KeyCode::N;
            break;
        case SDLK_o:
            return KeyCode::O;
            break;
        case SDLK_p:
            return KeyCode::P;
            break;
        case SDLK_q:
            return KeyCode::Q;
            break;
        case SDLK_r:
            return KeyCode::R;
            break;
        case SDLK_s:
            return KeyCode::S;
            break;
        case SDLK_t:
            return KeyCode::T;
            break;
        case SDLK_u:
            return KeyCode::U;
            break;
        case SDLK_v:
            return KeyCode::V;
            break;
        case SDLK_w:
            return KeyCode::W;
            break;
        case SDLK_x:
            return KeyCode::X;
            break;
        case SDLK_y:
            return KeyCode::Y;
            break;
        case SDLK_z:
            return KeyCode::Z;
            break;
        case SDLK_DELETE:
            return KeyCode::Delete;
            break;
        case SDLK_CAPSLOCK:
            return KeyCode::CapsLock;
            break;
        case SDLK_F1:
            return KeyCode::F1;
            break;
        case SDLK_F2:
            return KeyCode::F2;
            break;
        case SDLK_F3:
            return KeyCode::F3;
            break;
        case SDLK_F4:
            return KeyCode::F4;
            break;
        case SDLK_F5:
            return KeyCode::F5;
            break;
        case SDLK_F6:
            return KeyCode::F6;
            break;
        case SDLK_F7:
            return KeyCode::F7;
            break;
        case SDLK_F8:
            return KeyCode::F8;
            break;
        case SDLK_F9:
            return KeyCode::F9;
            break;
        case SDLK_F10:
            return KeyCode::F10;
            break;
        case SDLK_F11:
            return KeyCode::F11;
            break;
        case SDLK_F12:
            return KeyCode::F12;
            break;
        case SDLK_PRINTSCREEN:
            return KeyCode::PrintScreen;
            break;
        case SDLK_SCROLLLOCK:
            return KeyCode::ScrollLock;
            break;
        case SDLK_PAUSE:
            return KeyCode::Pause;
            break;
        case SDLK_INSERT:
            return KeyCode::Insert;
            break;
        case SDLK_HOME:
            return KeyCode::Home;
            break;
        case SDLK_PAGEUP:
            return KeyCode::PageUp;
            break;
        case SDLK_END:
            return KeyCode::End;
            break;
        case SDLK_PAGEDOWN:
            return KeyCode::PageDown;
            break;
        case SDLK_RIGHT:
            return KeyCode::Right;
            break;
        case SDLK_LEFT:
            return KeyCode::Left;
            break;
        case SDLK_DOWN:
            return KeyCode::Down;
            break;
        case SDLK_UP:
            return KeyCode::Up;
            break;
        default:
            return KeyCode::Unknown;
            break;
        }
    }
