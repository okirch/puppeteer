//////////////////////////////////////////////////////////////////
//
//	Support mapping of Qt constants to strings and vice
//	versa.
//
//
//
//
//////////////////////////////////////////////////////////////////

#define QT3_SUPPORT

#include "namespace.h"

#include <qapplication.h>
#include <qevent.h>
#include <qmenu.h>
#include <qmenubar.h>

#include <stdio.h>


static const char *
bitmaskToString(unsigned long value, const BitmaskMapping *mapping)
{
	static char buffer[256];
	unsigned long orig_value = value;
	unsigned int pos = 0;

	for (; mapping->name; ++mapping) {
		if (mapping->mask == value)
			return mapping->name;

		if ((value & mapping->mask) == mapping->mask) {
			unsigned int len = strlen(mapping->name);

			if (pos + len + 2 >= sizeof(buffer)) {
				snprintf(buffer, sizeof(buffer), "0x%lx", orig_value);
				return buffer;
			}

			if (pos)
				buffer[pos++] = ',';
			strcpy(buffer + pos, mapping->name);
			pos += len;

			value &= ~mapping->mask;
		}
	}

	/* FIXME: signal bits we couldn't map */
	return buffer;
}

static const char *
enumToString(unsigned long value, const BitmaskMapping *mapping)
{
	static char buffer[256];

	for (; mapping->name; ++mapping) {
		if (mapping->mask == value)
			return mapping->name;
	}

	snprintf(buffer, sizeof(buffer), (value > 1000)? "0x%lx" : "%lu", value);
	return buffer;
}

static bool
enumFromString(const QString &string, const BitmaskMapping *mapping, unsigned long *retval)
{
	bool ok;

	*retval = string.toULong(&ok, 0);
	if (ok)
		return true;

	for (; mapping->name; ++mapping) {
		if (string == mapping->name) {
			*retval = mapping->mask;
			return true;
		}
	}

	*retval = 0xdeadbeef;
	return false;
}

static BitmaskMapping modifierMap[] = {
	{ Qt::NoModifier, "none" },
	{ Qt::ShiftModifier, "shift" },
	{ Qt::ControlModifier, "control" },
	{ Qt::AltModifier, "alt" },
	{ Qt::MetaModifier, "meta" },
	{ Qt::KeypadModifier, "keypad" },
	{ Qt::GroupSwitchModifier, "groupswitch" },

	{ 0, NULL }
};

static BitmaskMapping buttonMap[] = {
	{ Qt::NoButton, "none" },
	{ Qt::LeftButton, "left" },
	{ Qt::RightButton, "right" },
	{ Qt::MidButton, "mid" },
	{ Qt::XButton1, "xbtn1" },
	{ Qt::XButton2, "xbtn2" },

	{ 0, NULL }
};

static BitmaskMapping keyMap[] = {
        { Qt::Key_Escape, "escape" },
        { Qt::Key_Tab, "tab" },
        { Qt::Key_Backtab, "backtab" },
#if defined(QT3_SUPPORT) && !defined(Q_MOC_RUN)
        { Qt::Key_BackTab, "backtab" },
#endif
        { Qt::Key_Backspace, "backspace" },
#if defined(QT3_SUPPORT) && !defined(Q_MOC_RUN)
        { Qt::Key_BackSpace, "backspace" },
#endif
        { Qt::Key_Return, "return" },
        { Qt::Key_Enter, "enter" },
        { Qt::Key_Insert, "insert" },
        { Qt::Key_Delete, "delete" },
        { Qt::Key_Pause, "pause" },
        { Qt::Key_Print, "print" },
        { Qt::Key_SysReq, "sysreq" },
        { Qt::Key_Clear, "clear" },
        { Qt::Key_Home, "home" },
        { Qt::Key_End, "end" },
        { Qt::Key_Left, "left" },
        { Qt::Key_Up, "up" },
        { Qt::Key_Right, "right" },
        { Qt::Key_Down, "down" },
        { Qt::Key_PageUp, "pageup" },
#if defined(QT3_SUPPORT) && !defined(Q_MOC_RUN)
        { Qt::Key_Prior, "prior" },
#endif
        { Qt::Key_PageDown, "pagedown" },
#if defined(QT3_SUPPORT) && !defined(Q_MOC_RUN)
        { Qt::Key_Next, "next" },
#endif
        { Qt::Key_Shift, "shift" },
        { Qt::Key_Control, "control" },
        { Qt::Key_Meta, "meta" },
        { Qt::Key_Alt, "alt" },
        { Qt::Key_CapsLock, "capslock" },
        { Qt::Key_NumLock, "numlock" },
        { Qt::Key_ScrollLock, "scrolllock" },
        { Qt::Key_F1, "f1" },
        { Qt::Key_F2, "f2" },
        { Qt::Key_F3, "f3" },
        { Qt::Key_F4, "f4" },
        { Qt::Key_F5, "f5" },
        { Qt::Key_F6, "f6" },
        { Qt::Key_F7, "f7" },
        { Qt::Key_F8, "f8" },
        { Qt::Key_F9, "f9" },
        { Qt::Key_F10, "f10" },
        { Qt::Key_F11, "f11" },
        { Qt::Key_F12, "f12" },
        { Qt::Key_F13, "f13" },
        { Qt::Key_F14, "f14" },
        { Qt::Key_F15, "f15" },
        { Qt::Key_F16, "f16" },
        { Qt::Key_F17, "f17" },
        { Qt::Key_F18, "f18" },
        { Qt::Key_F19, "f19" },
        { Qt::Key_F20, "f20" },
        { Qt::Key_F21, "f21" },
        { Qt::Key_F22, "f22" },
        { Qt::Key_F23, "f23" },
        { Qt::Key_F24, "f24" },
        { Qt::Key_F25, "f25" },
        { Qt::Key_F26, "f26" },
        { Qt::Key_F27, "f27" },
        { Qt::Key_F28, "f28" },
        { Qt::Key_F29, "f29" },
        { Qt::Key_F30, "f30" },
        { Qt::Key_F31, "f31" },
        { Qt::Key_F32, "f32" },
        { Qt::Key_F33, "f33" },
        { Qt::Key_F34, "f34" },
        { Qt::Key_F35, "f35" },
        { Qt::Key_Super_L, "super_l" },
        { Qt::Key_Super_R, "super_r" },
        { Qt::Key_Menu, "menu" },
        { Qt::Key_Hyper_L, "hyper_l" },
        { Qt::Key_Hyper_R, "hyper_r" },
        { Qt::Key_Help, "help" },
        { Qt::Key_Direction_L, "direction_l" },
        { Qt::Key_Direction_R, "direction_r" },
        { Qt::Key_Space, "space" },
        { Qt::Key_Any, "any" },
        { Qt::Key_Exclam, "exclam" },
        { Qt::Key_QuoteDbl, "quotedbl" },
        { Qt::Key_NumberSign, "numbersign" },
        { Qt::Key_Dollar, "dollar" },
        { Qt::Key_Percent, "percent" },
        { Qt::Key_Ampersand, "ampersand" },
        { Qt::Key_Apostrophe, "apostrophe" },
        { Qt::Key_ParenLeft, "parenleft" },
        { Qt::Key_ParenRight, "parenright" },
        { Qt::Key_Asterisk, "asterisk" },
        { Qt::Key_Plus, "plus" },
        { Qt::Key_Comma, "comma" },
        { Qt::Key_Minus, "minus" },
        { Qt::Key_Period, "period" },
        { Qt::Key_Slash, "slash" },
        { Qt::Key_0, "0" },
        { Qt::Key_1, "1" },
        { Qt::Key_2, "2" },
        { Qt::Key_3, "3" },
        { Qt::Key_4, "4" },
        { Qt::Key_5, "5" },
        { Qt::Key_6, "6" },
        { Qt::Key_7, "7" },
        { Qt::Key_8, "8" },
        { Qt::Key_9, "9" },
        { Qt::Key_Colon, "colon" },
        { Qt::Key_Semicolon, "semicolon" },
        { Qt::Key_Less, "less" },
        { Qt::Key_Equal, "equal" },
        { Qt::Key_Greater, "greater" },
        { Qt::Key_Question, "question" },
        { Qt::Key_At, "at" },
        { Qt::Key_A, "a" },
        { Qt::Key_B, "b" },
        { Qt::Key_C, "c" },
        { Qt::Key_D, "d" },
        { Qt::Key_E, "e" },
        { Qt::Key_F, "f" },
        { Qt::Key_G, "g" },
        { Qt::Key_H, "h" },
        { Qt::Key_I, "i" },
        { Qt::Key_J, "j" },
        { Qt::Key_K, "k" },
        { Qt::Key_L, "l" },
        { Qt::Key_M, "m" },
        { Qt::Key_N, "n" },
        { Qt::Key_O, "o" },
        { Qt::Key_P, "p" },
        { Qt::Key_Q, "q" },
        { Qt::Key_R, "r" },
        { Qt::Key_S, "s" },
        { Qt::Key_T, "t" },
        { Qt::Key_U, "u" },
        { Qt::Key_V, "v" },
        { Qt::Key_W, "w" },
        { Qt::Key_X, "x" },
        { Qt::Key_Y, "y" },
        { Qt::Key_Z, "z" },
        { Qt::Key_BracketLeft, "bracketleft" },
        { Qt::Key_Backslash, "backslash" },
        { Qt::Key_BracketRight, "bracketright" },
        { Qt::Key_AsciiCircum, "asciicircum" },
        { Qt::Key_Underscore, "underscore" },
        { Qt::Key_QuoteLeft, "quoteleft" },
        { Qt::Key_BraceLeft, "braceleft" },
        { Qt::Key_Bar, "bar" },
        { Qt::Key_BraceRight, "braceright" },
        { Qt::Key_AsciiTilde, "asciitilde" },

        { Qt::Key_nobreakspace, "nobreakspace" },
        { Qt::Key_exclamdown, "exclamdown" },
        { Qt::Key_cent, "cent" },
        { Qt::Key_sterling, "sterling" },
        { Qt::Key_currency, "currency" },
        { Qt::Key_yen, "yen" },
        { Qt::Key_brokenbar, "brokenbar" },
        { Qt::Key_section, "section" },
        { Qt::Key_diaeresis, "diaeresis" },
        { Qt::Key_copyright, "copyright" },
        { Qt::Key_ordfeminine, "ordfeminine" },
        { Qt::Key_guillemotleft, "guillemotleft" },
        { Qt::Key_notsign, "notsign" },
        { Qt::Key_hyphen, "hyphen" },
        { Qt::Key_registered, "registered" },
        { Qt::Key_macron, "macron" },
        { Qt::Key_degree, "degree" },
        { Qt::Key_plusminus, "plusminus" },
        { Qt::Key_twosuperior, "twosuperior" },
        { Qt::Key_threesuperior, "threesuperior" },
        { Qt::Key_acute, "acute" },
        { Qt::Key_mu, "mu" },
        { Qt::Key_paragraph, "paragraph" },
        { Qt::Key_periodcentered, "periodcentered" },
        { Qt::Key_cedilla, "cedilla" },
        { Qt::Key_onesuperior, "onesuperior" },
        { Qt::Key_masculine, "masculine" },
        { Qt::Key_guillemotright, "guillemotright" },
        { Qt::Key_onequarter, "onequarter" },
        { Qt::Key_onehalf, "onehalf" },
        { Qt::Key_threequarters, "threequarters" },
        { Qt::Key_questiondown, "questiondown" },
        { Qt::Key_Agrave, "agrave" },
        { Qt::Key_Aacute, "aacute" },
        { Qt::Key_Acircumflex, "acircumflex" },
        { Qt::Key_Atilde, "atilde" },
        { Qt::Key_Adiaeresis, "adiaeresis" },
        { Qt::Key_Aring, "aring" },
        { Qt::Key_AE, "ae" },
        { Qt::Key_Ccedilla, "ccedilla" },
        { Qt::Key_Egrave, "egrave" },
        { Qt::Key_Eacute, "eacute" },
        { Qt::Key_Ecircumflex, "ecircumflex" },
        { Qt::Key_Ediaeresis, "ediaeresis" },
        { Qt::Key_Igrave, "igrave" },
        { Qt::Key_Iacute, "iacute" },
        { Qt::Key_Icircumflex, "icircumflex" },
        { Qt::Key_Idiaeresis, "idiaeresis" },
        { Qt::Key_ETH, "eth" },
        { Qt::Key_Ntilde, "ntilde" },
        { Qt::Key_Ograve, "ograve" },
        { Qt::Key_Oacute, "oacute" },
        { Qt::Key_Ocircumflex, "ocircumflex" },
        { Qt::Key_Otilde, "otilde" },
        { Qt::Key_Odiaeresis, "odiaeresis" },
        { Qt::Key_multiply, "multiply" },
        { Qt::Key_Ooblique, "ooblique" },
        { Qt::Key_Ugrave, "ugrave" },
        { Qt::Key_Uacute, "uacute" },
        { Qt::Key_Ucircumflex, "ucircumflex" },
        { Qt::Key_Udiaeresis, "udiaeresis" },
        { Qt::Key_Yacute, "yacute" },
        { Qt::Key_THORN, "thorn" },
        { Qt::Key_ssharp, "ssharp" },
#if defined(QT3_SUPPORT) && !defined(Q_MOC_RUN)
        { Qt::Key_agrave, "agrave" },
        { Qt::Key_aacute, "aacute" },
        { Qt::Key_acircumflex, "acircumflex" },
        { Qt::Key_atilde, "atilde" },
        { Qt::Key_adiaeresis, "adiaeresis" },
        { Qt::Key_aring, "aring" },
        { Qt::Key_ae, "ae" },
        { Qt::Key_ccedilla, "ccedilla" },
        { Qt::Key_egrave, "egrave" },
        { Qt::Key_eacute, "eacute" },
        { Qt::Key_ecircumflex, "ecircumflex" },
        { Qt::Key_ediaeresis, "ediaeresis" },
        { Qt::Key_igrave, "igrave" },
        { Qt::Key_iacute, "iacute" },
        { Qt::Key_icircumflex, "icircumflex" },
        { Qt::Key_idiaeresis, "idiaeresis" },
        { Qt::Key_eth, "eth" },
        { Qt::Key_ntilde, "ntilde" },
        { Qt::Key_ograve, "ograve" },
        { Qt::Key_oacute, "oacute" },
        { Qt::Key_ocircumflex, "ocircumflex" },
        { Qt::Key_otilde, "otilde" },
        { Qt::Key_odiaeresis, "odiaeresis" },
#endif
        { Qt::Key_division, "division" },
#if defined(QT3_SUPPORT) && !defined(Q_MOC_RUN)
        { Qt::Key_oslash, "oslash" },
        { Qt::Key_ugrave, "ugrave" },
        { Qt::Key_uacute, "uacute" },
        { Qt::Key_ucircumflex, "ucircumflex" },
        { Qt::Key_udiaeresis, "udiaeresis" },
        { Qt::Key_yacute, "yacute" },
        { Qt::Key_thorn, "thorn" },
#endif
        { Qt::Key_ydiaeresis, "ydiaeresis" },

};

const char *
keyboardModifiersToString(Qt::KeyboardModifiers modifiers)
{
	return bitmaskToString(modifiers, modifierMap);
}

const char *
buttonMaskToString(Qt::MouseButtons buttons)
{
	return bitmaskToString(buttons, buttonMap);
}

const char *
buttonToString(Qt::MouseButton button)
{
	return enumToString(button, buttonMap);
}

Qt::MouseButton
buttonFromString(const QString &string)
{
	unsigned long value;

	if (!enumFromString(string, buttonMap, &value))
		return Qt::NoButton;
	return (Qt::MouseButton) value;
}

const char *
keyToString(int key)
{
	return enumToString(key, keyMap);
}

Qt::Key
keyFromString(const QString &string)
{
	unsigned long value;

	if (!enumFromString(string, keyMap, &value))
		return (Qt::Key) 0;
	return (Qt::Key) value;
}

static BitmaskMapping	eventMap[] = {
	{ QEvent::None, "None" },
	{ QEvent::Timer, "Timer" },
	{ QEvent::MouseButtonPress, "MouseButtonPress" },
	{ QEvent::MouseButtonRelease, "MouseButtonRelease" },
	{ QEvent::MouseButtonDblClick, "MouseButtonDblClick" },
	{ QEvent::MouseMove, "MouseMove" },
	{ QEvent::KeyPress, "KeyPress" },
	{ QEvent::KeyRelease, "KeyRelease" },
	{ QEvent::FocusIn, "FocusIn" },
	{ QEvent::FocusOut, "FocusOut" },
	{ QEvent::Enter, "Enter" },
	{ QEvent::Leave, "Leave" },
	{ QEvent::Paint, "Paint" },
	{ QEvent::Move, "Move" },
	{ QEvent::Resize, "Resize" },
	{ QEvent::Create, "Create" },
	{ QEvent::Destroy, "Destroy" },
	{ QEvent::Show, "Show" },
	{ QEvent::Hide, "Hide" },
	{ QEvent::Close, "Close" },
	{ QEvent::Quit, "Quit" },
	{ QEvent::ParentChange, "ParentChange" },
	{ QEvent::ParentAboutToChange, "ParentAboutToChange" },
	{ QEvent::ThreadChange, "ThreadChange" },
	{ QEvent::WindowActivate, "WindowActivate" },
	{ QEvent::WindowDeactivate, "WindowDeactivate" },
	{ QEvent::ShowToParent, "ShowToParent" },
	{ QEvent::HideToParent, "HideToParent" },
	{ QEvent::Wheel, "Wheel" },
	{ QEvent::WindowTitleChange, "WindowTitleChange" },
	{ QEvent::WindowIconChange, "WindowIconChange" },
	{ QEvent::ApplicationWindowIconChange, "ApplicationWindowIconChange" },
	{ QEvent::ApplicationFontChange, "ApplicationFontChange" },
	{ QEvent::ApplicationLayoutDirectionChange, "ApplicationLayoutDirectionChange" },
	{ QEvent::ApplicationPaletteChange, "ApplicationPaletteChange" },
	{ QEvent::PaletteChange, "PaletteChange" },
	{ QEvent::Clipboard, "Clipboard" },
	{ QEvent::Speech, "Speech" },
	{ QEvent::MetaCall, "MetaCall" },
	{ QEvent::SockAct, "SockAct" },
	{ QEvent::WinEventAct, "WinEventAct" },
	{ QEvent::DeferredDelete, "DeferredDelete" },
	{ QEvent::DragEnter, "DragEnter" },
	{ QEvent::DragMove, "DragMove" },
	{ QEvent::DragLeave, "DragLeave" },
	{ QEvent::Drop, "Drop" },
	{ QEvent::DragResponse, "DragResponse" },
	{ QEvent::ChildAdded, "ChildAdded" },
	{ QEvent::ChildPolished, "ChildPolished" },

#ifdef QT3_SUPPORT
	{ QEvent::ChildInsertedRequest, "ChildInsertedRequest" },
	{ QEvent::ChildInserted, "ChildInserted" },
	{ QEvent::LayoutHint, "LayoutHint" },
#endif

	{ QEvent::ChildRemoved, "ChildRemoved" },
	{ QEvent::ShowWindowRequest, "ShowWindowRequest" },
	{ QEvent::PolishRequest, "PolishRequest" },
	{ QEvent::Polish, "Polish" },
	{ QEvent::LayoutRequest, "LayoutRequest" },
	{ QEvent::UpdateRequest, "UpdateRequest" },
	{ QEvent::UpdateLater, "UpdateLater" },

	{ QEvent::EmbeddingControl, "EmbeddingControl" },
	{ QEvent::ActivateControl, "ActivateControl" },
	{ QEvent::DeactivateControl, "DeactivateControl" },
	{ QEvent::ContextMenu, "ContextMenu" },
	{ QEvent::InputMethod, "InputMethod" },
	{ QEvent::AccessibilityPrepare, "AccessibilityPrepare" },
	{ QEvent::TabletMove, "TabletMove" },
	{ QEvent::LocaleChange, "LocaleChange" },
	{ QEvent::LanguageChange, "LanguageChange" },
	{ QEvent::LayoutDirectionChange, "LayoutDirectionChange" },
	{ QEvent::Style, "Style" },
	{ QEvent::TabletPress, "TabletPress" },
	{ QEvent::TabletRelease, "TabletRelease" },
	{ QEvent::OkRequest, "OkRequest" },
	{ QEvent::HelpRequest, "HelpRequest" },

	{ QEvent::IconDrag, "IconDrag" },

	{ QEvent::FontChange, "FontChange" },
	{ QEvent::EnabledChange, "EnabledChange" },
	{ QEvent::ActivationChange, "ActivationChange" },
	{ QEvent::StyleChange, "StyleChange" },
	{ QEvent::IconTextChange, "IconTextChange" },
	{ QEvent::ModifiedChange, "ModifiedChange" },
	{ QEvent::MouseTrackingChange, "MouseTrackingChange" },

	{ QEvent::WindowBlocked, "WindowBlocked" },
	{ QEvent::WindowUnblocked, "WindowUnblocked" },
	{ QEvent::WindowStateChange, "WindowStateChange" },

	{ QEvent::ToolTip, "ToolTip" },
	{ QEvent::WhatsThis, "WhatsThis" },
	{ QEvent::StatusTip, "StatusTip" },

	{ QEvent::ActionChanged, "ActionChanged" },
	{ QEvent::ActionAdded, "ActionAdded" },
	{ QEvent::ActionRemoved, "ActionRemoved" },

	{ QEvent::FileOpen, "FileOpen" },

	{ QEvent::Shortcut, "Shortcut" },
	{ QEvent::ShortcutOverride, "ShortcutOverride" },

#ifdef QT3_SUPPORT
	{ QEvent::Accel, "Accel" },
	{ QEvent::AccelAvailable, "AccelAvailable" },
#endif

	{ QEvent::WhatsThisClicked, "WhatsThisClicked" },

	{ QEvent::ToolBarChange, "ToolBarChange" },

	{ QEvent::ApplicationActivate, "ApplicationActivate" },
	{ QEvent::ApplicationDeactivate, "ApplicationDeactivate" },

	{ QEvent::QueryWhatsThis, "QueryWhatsThis" },
	{ QEvent::EnterWhatsThisMode, "EnterWhatsThisMode" },
	{ QEvent::LeaveWhatsThisMode, "LeaveWhatsThisMode" },

	{ QEvent::ZOrderChange, "ZOrderChange" },

	{ QEvent::HoverEnter, "HoverEnter" },
	{ QEvent::HoverLeave, "HoverLeave" },
	{ QEvent::HoverMove, "HoverMove" },

	{ QEvent::AccessibilityHelp, "AccessibilityHelp" },
	{ QEvent::AccessibilityDescription, "AccessibilityDescription" },

#ifdef QT_KEYPAD_NAVIGATION
	{ QEvent::EnterEditFocus, "EnterEditFocus" },
	{ QEvent::LeaveEditFocus, "LeaveEditFocus" },
#endif
	{ QEvent::AcceptDropsChange, "AcceptDropsChange" },

	{ QEvent::MenubarUpdated, "MenubarUpdated" },

	{ QEvent::ZeroTimerEvent, "ZeroTimerEvent" },

	{ QEvent::GraphicsSceneMouseMove, "GraphicsSceneMouseMove" },
	{ QEvent::GraphicsSceneMousePress, "GraphicsSceneMousePress" },
	{ QEvent::GraphicsSceneMouseRelease, "GraphicsSceneMouseRelease" },
	{ QEvent::GraphicsSceneMouseDoubleClick, "GraphicsSceneMouseDoubleClick" },
	{ QEvent::GraphicsSceneContextMenu, "GraphicsSceneContextMenu" },
	{ QEvent::GraphicsSceneHoverEnter, "GraphicsSceneHoverEnter" },
	{ QEvent::GraphicsSceneHoverMove, "GraphicsSceneHoverMove" },
	{ QEvent::GraphicsSceneHoverLeave, "GraphicsSceneHoverLeave" },
	{ QEvent::GraphicsSceneHelp, "GraphicsSceneHelp" },
	{ QEvent::GraphicsSceneDragEnter, "GraphicsSceneDragEnter" },
	{ QEvent::GraphicsSceneDragMove, "GraphicsSceneDragMove" },
	{ QEvent::GraphicsSceneDragLeave, "GraphicsSceneDragLeave" },
	{ QEvent::GraphicsSceneDrop, "GraphicsSceneDrop" },
	{ QEvent::GraphicsSceneWheel, "GraphicsSceneWheel" },

	{ QEvent::KeyboardLayoutChange, "KeyboardLayoutChange" },

	{ QEvent::DynamicPropertyChange, "DynamicPropertyChange" },

	{ QEvent::TabletEnterProximity, "TabletEnterProximity" },
	{ QEvent::TabletLeaveProximity, "TabletLeaveProximity" },

	{ QEvent::NonClientAreaMouseMove, "NonClientAreaMouseMove" },
	{ QEvent::NonClientAreaMouseButtonPress, "NonClientAreaMouseButtonPress" },
	{ QEvent::NonClientAreaMouseButtonRelease, "NonClientAreaMouseButtonRelease" },
	{ QEvent::NonClientAreaMouseButtonDblClick, "NonClientAreaMouseButtonDblClick" },

	{ QEvent::MacSizeChange, "MacSizeChange" },
	{ QEvent::ContentsRectChange, "ContentsRectChange" },
	{ QEvent::MacGLWindowChange, "MacGLWindowChange" },
	{ QEvent::FutureCallOut, "FutureCallOut" },
	{ QEvent::GraphicsSceneResize, "GraphicsSceneResize" }, 
	{ QEvent::GraphicsSceneMove, "GraphicsSceneMove" }, 

	{ QEvent::CursorChange, "CursorChange" },
	{ QEvent::ToolTipChange, "ToolTipChange" },
	{ QEvent::NetworkReplyUpdated, "NetworkReplyUpdated" },
	{ QEvent::GrabMouse, "GrabMouse" },
	{ QEvent::UngrabMouse, "UngrabMouse" },
	{ QEvent::GrabKeyboard, "GrabKeyboard" },
	{ QEvent::UngrabKeyboard, "UngrabKeyboard" },
	{ QEvent::CocoaRequestModal, "CocoaRequestModal" },
	{ QEvent::MacGLClearDrawable, "MacGLClearDrawable" },

	{ QEvent::StateMachineSignal, "StateMachineSignal" },
	{ QEvent::StateMachineWrapped, "StateMachineWrapped" },

	{ QEvent::TouchBegin, "TouchBegin" },
	{ QEvent::TouchUpdate, "TouchUpdate" },
	{ QEvent::TouchEnd, "TouchEnd" },

	{ QEvent::NativeGesture, "NativeGesture" },

	{ QEvent::RequestSoftwareInputPanel, "RequestSoftwareInputPanel" },
	{ QEvent::CloseSoftwareInputPanel, "CloseSoftwareInputPanel" },

	{ QEvent::UpdateSoftKeys, "UpdateSoftKeys" },

	{ QEvent::WinIdChange, "WinIdChange" },
	{ QEvent::Gesture, "Gesture" },
	{ QEvent::GestureOverride, "GestureOverride" },

	{ 0, NULL }
};

QEvent::Type
eventTypeFromString(const QString &string)
{
	unsigned long type;

	if (!enumFromString(string, eventMap, &type))
		return QEvent::None;
	return (QEvent::Type) type;
}


const char *
eventTypeName(QEvent::Type type)
{
	return enumToString(type, eventMap);
}
