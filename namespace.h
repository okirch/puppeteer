//////////////////////////////////////////////////////////////////
//
//	Support mapping of Qt constants to strings and vice
//	versa.
//
//
//
//
//////////////////////////////////////////////////////////////////

#ifndef NAMESPACE_H
#define NAMESPACE_H

#include <qevent.h>

struct BitmaskMapping {
	unsigned long		mask;
	const char *		name;
};

extern const char *	eventTypeName(QEvent::Type type);
extern QEvent::Type	eventTypeFromString(const QString &);

extern const char *	keyboardModifiersToString(Qt::KeyboardModifiers modifiers);

extern const char *	buttonToString(Qt::MouseButton);
extern Qt::MouseButton	buttonFromString(const QString &);

extern const char *	buttonMaskToString(Qt::MouseButtons buttons);
extern unsigned long	buttonMaskFromString(const QString &);

extern const char *	keyToString(int key);
extern Qt::Key		keyFromString(const QString &);

#endif // NAMESPACE_H
