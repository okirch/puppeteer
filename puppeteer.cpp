//////////////////////////////////////////////////////////////////
//
//	Puppeteer - main class
//
//
//
//
//
//////////////////////////////////////////////////////////////////

#define QT3_SUPPORT // remove this soonishly

#include <sys/time.h>
#include <qapplication.h>
#include <qevent.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qcombobox.h>
#include <qabstractitemview.h>
#include <qmetaobject.h>

#include <qdom.h>
#include <qfile.h>

#include <stdio.h>
#include "puppeteer.h"
#include "namespace.h"


static bool		neverRecordEvent(QEvent::Type type);

Puppeteer::Puppeteer()
: applicationActive(false), mScript(0)
{
	connect(qApp, SIGNAL(aboutToQuit()), SLOT(aboutToQuitSlot()));
}

Puppeteer::~Puppeteer()
{
	if (mScript)
		delete mScript;
}

void
Puppeteer::start()
{
	Puppeteer *self = new Puppeteer;
	const char *script;

	script = getenv("PUPPETEER_PLAYBACK");
	if (script != NULL)
		self->playbackStart(script);
	else
		self->startRecording();
}

void
Puppeteer::startRecording()
{
	qApp->installEventFilter(this);
}

void
Puppeteer::aboutToQuitSlot()
{
	if (mScript) {
		// Playback case
		Script::Action *playbackAction;

		playbackAction = mScript->currentAction();
		if (playbackAction && playbackAction->type() == Script::WaitApplicationExit) {
			printf("=== Application exiting.\n");
			playbackNextAction();
		}

		if (mScript->currentAction() == 0)
			printf("=== All is well. Script succeeded\n");
		else
			printf("=== Ooops, application exits before script is done\n");
	} else {
		// Recording case
		RecordNode("quit").write();
	}
}

void
Puppeteer::actionTimeoutSlot()
{
	Script::Action *currentAction;

	if (mScript == 0 || (currentAction = mScript->currentAction()) == 0)
		return;

	switch (currentAction->type()) {
	case Script::WaitApplicationExit:
		printf("=== Timed out waiting for application to exit (after %lu msec)\n", currentAction->timeout());
		playbackFailure();
		break;

	case Script::WaitEvent:
		printf("=== Timed out waiting for event (after %lu msec)\n", currentAction->timeout());

		/* We should probably terminate the application here, somehow */
		playbackFailure();
		break;

	case Script::SendEvent:
		// Get ready to inject the event
		playbackEvent(currentAction->event());
		playbackNextAction();
		break;

	case Script::SetFocus:
		if (!playbackSetFocus(currentAction->event())) {
			playbackFailure();
			break;
		}

		playbackNextAction();
		break;

	case Script::VerifyProperties:
		if (!playbackVerifyProperties(currentAction->event())) {
			playbackFailure();
			break;
		}

		playbackNextAction();
		break;

	default:
		printf("=== Timed out waiting for something that's not implemented\n");
		break;
	}
}

/*
 * Playback
 */
void
Puppeteer::playbackStart(QString filename)
{
	Script *script;

	script = new Script;
	if (!script->load(filename)) {
		fprintf(stderr, "Unable to parse playback script \"%s\"\n", qPrintable(filename));
		return;
	}

	connect(&mTimer, SIGNAL(timeout()), this, SLOT(actionTimeoutSlot()));

	// Now execute it
	mScript = script;
	if (script->currentAction() == 0) {
		// Uhm, empty script
		playbackFinished();
	} else {
		playbackDescribeAction(script->currentAction());
	}

	qApp->installEventFilter(this);
}

void
Puppeteer::playbackDescribeAction(const Script::Action *action)
{
	if (action == 0)
		return;

	switch (action->type()) {
	case Script::WaitApplicationExit:
		printf("=== Waiting for application to exit\n");
		break;

	case Script::WaitEvent:
		printf("=== Waiting for event:\n");
		action->event()->write();
		break;

	case Script::SendEvent:
		printf("=== Preparing to send event\n");
		action->event()->write();
		break;

	case Script::SetFocus:
		printf("=== Preparing to set keyboard focus\n");
		break;

	case Script::VerifyProperties:
		printf("=== Preparing to verify UI state\n");
		break;

	default:
		printf("=== I'm sure I'm about to do something meaningful, but I can't say what it is\n");
	}
}

bool
Puppeteer::playbackNextAction()
{
	Script::Action *nextAction;

	mTimer.stop();

	if (!mScript)
		return false;
	mScript->actionDone();

	if ((nextAction = mScript->currentAction()) == 0) {
		playbackFinished();
	} else {
		playbackDescribeAction(nextAction);
		mTimer.setInterval(nextAction->timeout());
		mTimer.setSingleShot(true);
		mTimer.start();
	}

	return true;
}

bool
Puppeteer::playbackEvent(const EventRecord *rec)
{
	QWidget *widget;
	QEvent *ev;

	if (!(widget = objectForRecord(rec))) {
		fprintf(stderr, "=== cannot inject event, receiver object not found\n");
		rec->write();
		playbackFailure();
		return false;
	}

	if (!(ev = buildEvent(widget, rec))) {
		fprintf(stderr, "=== cannot inject event, unable to build event from spec\n");
		rec->write();
		playbackFailure();
		return false;
	}

	printf("=== Posting event:\n");
	rec = recordEvent(widget, ev);
	rec->write();
	delete rec;

	qApp->postEvent(widget, ev);

	return true;
}

bool
Puppeteer::playbackSetFocus(const EventRecord *rec)
{
	QWidget *w;

	if (!(w = objectForRecord(rec))) {
		fprintf(stderr, "=== cannot set focus, receiver object not found\n");
		rec->write();
		return false;
	}

	if (qApp->focusWidget() == w) {
		fprintf(stderr, "=== object already has focus\n");
		return true;
	}

	// Note: this is somewhat tricky. During the execution of the setFocus() call,
	// we will enter a recursive event loop, during which the FocusIn event
	// arrives. If we wanted to match for this event, we would have to post this
	// event match here. However, we don't have anything like "auxiliary" event
	// matching in place, so we just ignore this and explicitly check for the
	// application focus widget.
	w->setFocus(Qt::OtherFocusReason);

	if (qApp->focusWidget() != w) {
		fprintf(stderr, "=== unable to set focus\n");
		return false;
	}

	return true;
}

bool
Puppeteer::playbackVerifyProperties(const EventRecord *rec)
{
	RecordNode *data;
	QWidget *w;

	if (!(w = objectForRecord(rec))) {
		fprintf(stderr, "=== cannot verify properties, receiver object not found\n");
		rec->write();
		return false;
	}

	if ((data = rec->child("classdata")) == 0) {
		printf("=== No classdata provided.\n");
		return false;
	}

	const RecordNode::list &list(data->children());
	for (RecordNode::list::const_iterator it = list.begin(); it != list.end(); ++it) {
		const RecordNode *child = *it;
		QString propertyName, expectedValue, actualValue;

		if (child->name() != "property")
			continue;

		propertyName = child->attribute("name");
		expectedValue = child->attribute("value");
		if (!getObjectProperty(w, propertyName, actualValue)) {
			printf("=== Object does not support property %s\n", qPrintable(propertyName));
			return false;
		}

		if (expectedValue != actualValue) {
			printf("=== Object property %s does not match. Expected \"%s\", got \"%s\"\n",
					qPrintable(propertyName),
					qPrintable(expectedValue),
					qPrintable(actualValue));
			return false;
		}

		printf("=== Verify ok: object property %s=\"%s\"\n",
				qPrintable(propertyName),
				qPrintable(expectedValue));
	}

	printf("=== PASS: Successfully verified properties\n");
	return true;
}

void
Puppeteer::playbackFailure()
{
	printf("=== Playback failed, tape completely garbled.\n");
	mTimer.stop();

	if (mScript)
		delete mScript;
	mScript = 0;
}

void
Puppeteer::playbackFinished()
{
	printf("=== Playback reached end of tape. Watch the spinning reels and listen to the white noise.\n");
	mTimer.stop();
}

/*
 * Filter for application level events
 * This is where the main action happens.
 *
 * During Recording, we merely analyze all the events, and write out as much relevant info
 * as we can gather about them.
 *
 * During playback, when we get here, we have to check whether the script is currently
 * waiting for a specific event. If it is, we compare the current event with the one
 * being waited for.
 *
 * Note that injection of events does not happen here; we always delay these by
 * a little bit - hence, injection happens from actionTimeoutSlot().
 */
bool
Puppeteer::eventFilter(QObject *object, QEvent *event)
{
	EventRecord *rec;

	/* TBD: If the script is just idling, don't even bother with
	 * analyzing this event
	 */
	if ((rec = recordEvent(object, event)) != 0) {
		if (mScript) {
			// Playback case
			Script::Action *playbackAction;

			playbackAction = mScript->currentAction();
			if (playbackAction && playbackAction->type() == Script::WaitEvent) {
				if (playbackAction->matchCurrentEvent(rec)) {
					printf("=== Matched Event:\n");
					rec->write();
					printf("===\n");

					playbackNextAction();
				}
			}
		} else {
			// Recording case
			rec->write();
		}
		delete rec;
	}

	return false;
}

/*
 * Given a QObject, build a string "path" using its name and
 * that of its ancestors.
 */
static void
buildObjectPath(QObject *object, QString &res)
{
	QString objName;
	QObject *parent;

	if (object == 0)
		return;

	objName = object->objectName();
	if (objName.isEmpty()) {
		while ((parent = object->parent()) != NULL
			&& parent->objectName().isEmpty())
			object = parent;

		objName = "*";
	} else {
		parent = object->parent();
	}

	if (parent != NULL) {
		buildObjectPath(parent, res);
		res.append('.');
	}
	res.append(objName);
}

QString
Puppeteer::buildObjectPath(QObject *object, EventRecord *rec)
{
	QString objName;
	QString name;

	// If the trailing component of the object path is not known, we
	// are missing some vital piece of information
	objName = object->objectName();
	if (!objName.isEmpty()) {
		::buildObjectPath(object, name);
	} else {
		const QMetaObject *metaObj = object->metaObject();
		RecordNode *classHints;

		::buildObjectPath(object, name);
		classHints = rec->addClassHints(metaObj->className());

		QMenu *menu;

		menu = qobject_cast<QMenu *>(object);
		if (menu != 0) {
			buildObjectProperty(object, "title", classHints);
		}
	}
	return name;
}

bool
Puppeteer::buildObjectProperty(QObject *object, const char *propertyName, RecordNode *classHints)
{
	QString value;

	if (!getObjectProperty(object, propertyName, value))
		return false;

	if (!value.isEmpty()) {
		RecordNode *child = classHints->addChild("property");

		child->addAttribute("name", propertyName);
		child->addAttribute("value", value);
	}

	return true;
}

bool
Puppeteer::getObjectProperty(const QObject *object, const char *name, QString &value)
{
	const QMetaObject *metaObj = object->metaObject();
	QMetaProperty metaProp;
	int index;

	index = metaObj->indexOfProperty(name);
	if (index < 0)
		return false;

	metaProp = metaObj->property(index);

	QVariant data(metaProp.read(object));
	if (!data.isValid())
		return false;

	value = data.toString();
	return true;
}

void
Puppeteer::recordObjectPath(QObject *object, EventRecord *rec)
{
	rec->addAttribute("objectPath", buildObjectPath(object, rec));
}

QWidget *
Puppeteer::objectForRecord(const EventRecord *rec) const
{
	QString targetName = rec->attribute("objectPath");
	QStringList path = targetName.split('.');
	QWidgetList workingSet = qApp->topLevelWidgets();
	bool wasWildcard = true;

	printf("Looking for object with path \"%s\"\n", qPrintable(targetName));

	/* If the path starts with a non-wildcard, we should
	 * begin with the top level widget matching exactly this name.
	 */
	if (path[0] != "*") {
		QWidgetList newWorkingSet;

		printf("Looking for top-level widget(s) named %s\n", qPrintable(path[0]));
		for (int j = 0; j < workingSet.count(); ++j) {
			QWidget *w = workingSet[j];

			if (w->objectName() == path[0])
				newWorkingSet.append(w);
		}

		workingSet = newWorkingSet;
		path.takeFirst();
	}

	for (int i = 0; i < path.count(); ++i) {
		QWidgetList newWorkingSet;
		QString name = path[i];

		if (workingSet.isEmpty())
			return 0;

		if (name == "*") {
			wasWildcard = true;
			continue;
		}

		printf("  Looking for name=%s in set of %u widgets\n", qPrintable(name), workingSet.count());
		for (int j = 0; j < workingSet.count(); ++j) {
			QWidget *w = workingSet[j];
			QList<QWidget *> descendants;

			descendants = w->findChildren<QWidget *>(name);
			printf("    %d: widget %s has %u matching descendants\n",
					j, qPrintable(w->objectName()), descendants.count());
			newWorkingSet.append(descendants);
		}

		workingSet = newWorkingSet;
		wasWildcard = false;
	}

	if (wasWildcard) {
		const RecordNode *classHints(rec->classHints());

		if (classHints == 0) {
			printf("=== Ambiguous receiver object. Path ends with a wildcard, but no classhints given\n");
			return 0;
		}

		workingSet = filterObjectsByClasshints(workingSet, classHints);
	} else
	if (workingSet.count() > 1) {
		/* Path ended in a name, but the result is still ambiguous.
		 * Poor naming practice on the part of the programmer..
		 *
		 * Try to use the type information that may be provided by the EventRecord.
		 */
	}

	if (workingSet.count() == 1) {
		printf("Success, found exactly one object\n");
		return workingSet[0];
	}

	return 0;
}

QWidgetList
Puppeteer::filterObjectsByClasshints(const QWidgetList &workingSet, const RecordNode *hints) const
{
	QString className = hints->attribute("name");
	QWidgetList result;

	if (className.isEmpty())
		return result;

	for (QWidgetList::const_iterator it = workingSet.begin(); it != workingSet.end(); ++it) {
		filterObjectsByClasshints(*it, className, hints, result);
	}

	return result;
}

void
Puppeteer::filterObjectsByClasshints(QWidget *w, const QString &className, const RecordNode *hints, QWidgetList &result) const
{
	if (w->inherits(className)) {
		// Check the class hints for properties that could help us identify the
		// object we're looking for
		if (hints == 0 || matchObjectProperties(w, hints)) {
			result.append(w);
			return;
		}
	}

	const QObjectList &children(w->children());
	for (QObjectList::const_iterator it = children.begin(); it != children.end(); ++it) {
		QWidget *child = qobject_cast<QWidget *>(*it);

		if (child)
			filterObjectsByClasshints(child, className, hints, result);
	}
}

bool
Puppeteer::matchObjectProperties(QWidget *w, const RecordNode *hints) const
{
	const RecordNode::list &children(hints->children());
	const QMetaObject *metaObj = w->metaObject();

	for (RecordNode::list::const_iterator it = children.begin(); it != children.end(); ++it) {
		const RecordNode *child = *it;
		QString propertyName, propertyValue;
		int propertyIndex;

		if (child->name() != "property")
			continue;

		propertyName = child->attribute("name");
		propertyValue = child->attribute("value");

		if ((propertyIndex = metaObj->indexOfProperty(propertyName)) < 0)
			return false;

		QVariant data = metaObj->property(propertyIndex).read(w);
		if (!data.isValid())
			return false;

		QString actualValue = data.toString();
		if (propertyValue != actualValue
		 && propertyValue != sanitizeButtonString(actualValue))
			return false;

		printf("good, object has expected property %s=%s\n", qPrintable(propertyName), qPrintable(propertyValue));
	}

	return true;
}

EventRecord *
Puppeteer::recordEvent(QObject *object, QEvent *event)
{
	EventRecord *rec;

	if (neverRecordEvent(event->type()))
		return 0;

	rec = new EventRecord(eventTypeName(event->type()), Puppeteer::timestamp());

	switch (event->type()) {
	case QEvent::ApplicationActivate:
		// No event playback prior to this stage
		applicationActive = true;
		break;
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
		recordMouseEvent(object, (QMouseEvent *) event, rec);
		break;
	case QEvent::Show:
		recordShowEvent(object, (QShowEvent *) event, rec);
		break;
	case QEvent::FocusIn:
	case QEvent::FocusOut:
		recordFocusEvent(object, (QFocusEvent *) event, rec);
		break;
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
		recordKeyEvent(object, (QKeyEvent *) event, rec);
		break;

	default: ;
	}

	return rec;
}

void
Puppeteer::recordMouseEvent(QObject *object, QMouseEvent *ev, EventRecord *rec)
{
	recordObjectPath(object, rec);

	rec->addAttribute("keyboardModifiers", keyboardModifiersToString(ev->modifiers()));
	rec->addAttribute("x", QString::number(ev->x()));
	rec->addAttribute("y", QString::number(ev->y()));
	rec->addAttribute("globalX", QString::number(ev->globalX()));
	rec->addAttribute("globalY", QString::number(ev->globalY()));
	rec->addAttribute("button", buttonToString(ev->button()));
	rec->addAttribute("buttonState", buttonMaskToString(ev->buttons()));

	// Later on, when we want to play back this event, we need as much info
	// as possible about where to click.
	//
	// We could blindly click into the given widget at just the same offset, but
	// that will not necessarily generate the same results. For example, the labels
	// of a menu bar will have a different size due to a different font being chosen,
	// or maybe they will even have a different label thanks to switching to a
	// different language.
	if (ev->buttons()) {
		QMenuBar *menuBar;
		QMenu *menu;

		if ((menuBar = qobject_cast<QMenuBar *>(object)) != 0) {
			QAction *action = menuBar->actionAt(ev->pos());

			if (action != 0)
				recordAction(action, rec->addTargetHints());
		} else
		if ((menu = qobject_cast<QMenu *>(object)) != 0) {
			QAction *action = menu->actionAt(ev->pos());

			if (action != 0)
				recordAction(action, rec->addTargetHints());
		}
	}
}

void
Puppeteer::recordShowEvent(QObject *object, QShowEvent *ev, EventRecord *rec)
{
	recordObjectPath(object, rec);
}

void
Puppeteer::recordFocusEvent(QObject *object, QFocusEvent *ev, EventRecord *rec)
{
	recordObjectPath(object, rec);
	// FIXME: could also record the reason here
}

void
Puppeteer::recordKeyEvent(QObject *object, QKeyEvent *ev, EventRecord *rec)
{
	recordObjectPath(object, rec);
	rec->addAttribute("keyboardModifiers", keyboardModifiersToString(ev->modifiers()));
	rec->addAttribute("key", keyToString(ev->key()));
	rec->addAttribute("text", ev->text());
}

void
Puppeteer::recordAction(QAction *action, RecordNode *rec)
{
	rec = rec->addChild("action");

	rec->addAttribute("text", sanitizeButtonString(action->text()));
	rec->addAttribute("iconText", action->iconText());

	QVariant data(action->data());
	if (data.isValid()) {
		QString value = data.toString();
		RecordNode *dataNode;

		if (!value.isEmpty()) {
			dataNode = rec->addChildUnique("data");

			dataNode->addAttribute("type", data.typeName());
			dataNode->addAttribute("value", value);
		}
	}
}

QString
Puppeteer::sanitizeButtonString(QString text) const
{
	if (text.indexOf('&') >= 0) {
		QString sane;
		int pos;

		while (true) {
			if ((pos = text.indexOf('&')) < 0) {
				sane.append(text);
				break;
			}
			sane.append(text.left(pos));

			// skip over & character and copy next char verbatim
			// (not sure what the escape convention is in Qt)
			pos++;
			if (pos < text.length()) {
				sane.append(text.at(pos));
				pos++;
			}
			if (pos >= text.length())
				break;
			text = text.mid(pos);
		}
		text = sane;
	}
	return text;
}

QString
Puppeteer::timestamp()
{
	static struct timeval t0;
	struct timeval now, delta;
	QString timestamp;

	if (t0.tv_sec == 0)
		gettimeofday(&t0, NULL);
	gettimeofday(&now, NULL);
	timersub(&now, &t0, &delta);
	timestamp.sprintf("%u.%06u", (unsigned int) delta.tv_sec, (unsigned int) delta.tv_usec);

	return timestamp;
}


QEvent *
Puppeteer::buildEvent(QWidget *&widget, const EventRecord *rec) const
{
	QString typeName;
	QEvent::Type type;
	QEvent *ev;

	typeName = rec->attribute("type");
	type = eventTypeFromString(typeName);
	if (type == QEvent::None) {
		fprintf(stderr, "=== %s: invalid event type %s\n", __func__, qPrintable(typeName));
		return 0;
	}

	switch (type) {
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
		ev = buildMouseEvent(widget, type, rec);
		break;

	case QEvent::KeyPress:
	case QEvent::KeyRelease:
		ev = buildKeyEvent(widget, type, rec);
		break;

	default:
		fprintf(stderr, "=== %s: unsupported event type %s\n", __func__, qPrintable(typeName));
		return 0;
	}

	return ev;
}

QMouseEvent *
Puppeteer::buildMouseEvent(QWidget *&widget, QEvent::Type type, const EventRecord *rec) const
{
	QMouseEvent *ev;
	QPoint pos(-1, -1);
	Qt::MouseButton button = Qt::NoButton;
	Qt::MouseButtons buttonState = 0;
	Qt::KeyboardModifiers modifiers = 0;
	bool havePos = false;
	QString value;

	value = rec->attribute("button");
	if (!value.isEmpty())
		button = buttonFromString(value);

	value = rec->attribute("x");
	if (!value.isEmpty()) {
		pos.setX(value.toUInt());
		havePos = true;
	}
	value = rec->attribute("y");
	if (!value.isEmpty()) {
		pos.setY(value.toUInt());
		havePos = true;
	}

	value = rec->attribute("modifiers");
	// TBD: map the string to Qt::KeyboardModifiers

	if (!havePos) {
		const RecordNode *classHints = rec->classHints();
		const RecordNode *targetHints = rec->targetHints();
		QString className;

		if (classHints)
			className = classHints->attribute("name");

		if (widget->inherits("QMenuBar")) {
			havePos = menuBarMouseEventTarget(widget, targetHints, pos);
		} else
		if (widget->inherits("QMenu")) {
			havePos = menuMouseEventTarget(widget, targetHints, pos);
		} else
		if (widget->inherits("QComboBox")) {
			havePos = comboBoxMouseEventTarget(widget, targetHints, pos);
		}
	}

	if (!havePos && widget != 0) {
		printf("=== No position given, just aiming dead center and hoping for the best\n");
		if (pos.x() < 0)
			pos.setX(widget->width()/2);
		if (pos.y() < 0)
			pos.setY(widget->height()/2);
	}

	// If we're pressing a button, we should at least pretend that the button
	// is asserted in the current buttonState.
	if (type == QEvent::MouseButtonPress)
		buttonState |= button;
	else
		buttonState &= ~button;

	ev = new QMouseEvent(type, pos, button, buttonState, modifiers);
	return ev;
}

bool
Puppeteer::menuBarMouseEventTarget(QWidget *&widget, const RecordNode *hints, QPoint &pos) const
{
	QMenuBar *menuBar = qobject_cast<QMenuBar *>(widget);
	RecordNode *child;
	QString wantedActionText;
	unsigned int width, x, y;

	if (hints == 0)
		return false;

	child = hints->child("action");
	if (child == 0)
		return false;

	wantedActionText = child->attribute("text");
	if (wantedActionText.isEmpty())
		return false;

	y = menuBar->height() / 2;
	width = menuBar->width();
	for (x = 5; x < width; x += 10) {
		QAction *a = menuBar->actionAt(QPoint(x, y));

		if (a == 0)
			continue;

		QString thisActionText(sanitizeButtonString(a->text()));
		if (thisActionText == wantedActionText) {
			printf("=== Action \"%s\" is at <%d,%d>\n", qPrintable(wantedActionText), x, y);
			pos = QPoint(x, y);
			return true;
		}
	}

	printf("QMenuBar object has no action named \"%s\" - or I'm too stupid to find it\n", qPrintable(wantedActionText));
	return false;
}

bool
Puppeteer::menuMouseEventTarget(QWidget *&widget, const RecordNode *hints, QPoint &pos) const
{
	QMenu *menu = qobject_cast<QMenu *>(widget);
	RecordNode *child;
	QString wantedActionText;
	unsigned int height, x, y;

	if (hints == 0)
		return false;

	child = hints->child("action");
	if (child == 0)
		return false;

	wantedActionText = child->attribute("text");
	if (wantedActionText.isEmpty())
		return false;

	x = menu->width() / 2;
	height = menu->height();
	for (y = 5; y < height; y += 9) {
		QAction *a = menu->actionAt(QPoint(x, y));

		if (a == 0)
			continue;

		QString thisActionText(sanitizeButtonString(a->text()));
		if (thisActionText == wantedActionText) {
			printf("=== Action \"%s\" is at <%d,%d>\n", qPrintable(wantedActionText), x, y);
			pos = QPoint(x, y);
			return true;
		}
	}

	printf("QMenu object has no action named \"%s\" - or I'm too stupid to find it\n", qPrintable(wantedActionText));
	return false;
}

bool
Puppeteer::comboBoxMouseEventTarget(QWidget *&widget, const RecordNode *hints, QPoint &pos) const
{
	QComboBox *combo;
	RecordNode *child;
	QString wantedItemText;
	int itemIndex;

	if ((combo = qobject_cast<QComboBox *>(widget)) == 0)
		return false;

	if (hints == 0)
		return false;

	child = hints->child("item");
	if (child == 0)
		return false;

	wantedItemText = child->attribute("text");
	if (wantedItemText.isEmpty())
		return false;

	itemIndex = combo->findText(wantedItemText);
	if (itemIndex < 0)
		return false;

	QAbstractItemView *view = combo->view();
	if (view->isHidden())
		return false;

	QModelIndex modelIndex = view->model()->index(itemIndex, 0);
	if (!modelIndex.isValid())
		return false;

	view->scrollTo(modelIndex);

	QRect r = view->visualRect(modelIndex);

	if (r.isValid()) {
		pos = r.center();

		// Now verify that what we're trying to do is
		// actually likely to work.
		QModelIndex targetIndex = view->indexAt(pos);
		if (targetIndex != modelIndex)
			printf("Hm, index doesn't agree. This is unlikely to work\n");

		QString targetItemText(view->model()->data(targetIndex).toString());
		if (targetItemText != wantedItemText) {
			printf("=== Wanted text \"%s\", but looks like I'm aiming for \"%s\"\n",
					qPrintable(wantedItemText),
					qPrintable(targetItemText));
			return false;
		}

		widget = view->viewport();
		return true;
	}

	return false;
}

QKeyEvent *
Puppeteer::buildKeyEvent(QWidget *&object, QEvent::Type type, const EventRecord *rec) const
{
	QKeyEvent *ev;
	Qt::KeyboardModifiers modifiers = 0;
	Qt::Key key = (Qt::Key) 0;
	QString value;

	value = rec->attribute("key");
	if (!value.isEmpty())
		key = keyFromString(value);

	if (key == 0) {
		fprintf(stderr, "=== No or invalid key in key event\n");
		return 0;
	}

	value = rec->attribute("modifiers");
	// TBD: map the string to Qt::KeyboardModifiers

	ev = new QKeyEvent(type, key, modifiers);
	return ev;
}

static bool
neverRecordEvent(QEvent::Type type)
{
	// At some point, we should use a lookup into a bool area to reduce the overhead
	switch (type) {
	case QEvent::Paint:
	case QEvent::PaletteChange:
	case QEvent::LayoutRequest:

	case QEvent::MouseMove:
	case QEvent::Enter:
	case QEvent::Leave:

	case QEvent::HoverMove:
	case QEvent::HoverEnter:
	case QEvent::HoverLeave:

	case QEvent::ChildAdded:
	case QEvent::ChildRemoved:
	case QEvent::ChildInserted:
	case QEvent::ChildInsertedRequest:
	case QEvent::DeferredDelete:
	case QEvent::UpdateRequest:

	case QEvent::PolishRequest:
	case QEvent::Polish:
	case QEvent::ChildPolished:

	case QEvent::StatusTip:
	case QEvent::ToolTip:
		return true;

	default:
		break;
	}

	return false;
}

RecordNode::RecordNode(const QDomElement &domElement)
: mName(domElement.tagName())
{
	fromDomElement(domElement);
}

RecordNode::~RecordNode()
{
	while (!mChildren.isEmpty()) {
		RecordNode *child = mChildren.takeFirst();

		delete child;
	}
}

RecordNode *
RecordNode::addChildUnique(const QString &name)
{
	QList<RecordNode *>::iterator it;
	RecordNode *child;

	for (it = mChildren.begin(); it != mChildren.end(); ++it) {
		child = *it;

		if (child->name() == name)
			return child;
	}

	child = new RecordNode(name);
	mChildren.append(child);

	return child;
}

RecordNode *
RecordNode::addChild(const QString &name)
{
	RecordNode *child;

	child = new RecordNode(name);
	mChildren.append(child);

	return child;
}

RecordNode *
RecordNode::child(const QString &name) const
{
	QList<RecordNode *>::const_iterator it;
	RecordNode *child;

	for (it = mChildren.begin(); it != mChildren.end(); ++it) {
		child = *it;

		if (child->name() == name)
			return child;
	}

	return 0;
}

void
RecordNode::addAttribute(const QString &name, const QString &value)
{
	mAttributes.append(Attribute(name, value));
}

QString
RecordNode::attribute(const QString &name) const
{
	for (Attribute::list::const_iterator it(mAttributes.begin()); it != mAttributes.end(); ++it) {
		const Attribute &a(*it);

		if (a.name == name)
			return a.value;
	}
	return QString::null;
}

bool
RecordNode::write(int indent) const
{
	printf("%*.*s", indent, indent, "");
	printf("<%s", qPrintable(mName));

	for (QList<Attribute>::const_iterator it(mAttributes.begin()); it != mAttributes.end(); ++it) {
		const Attribute &a(*it);
		printf(" %s=\"%s\"", qPrintable(a.name), qPrintable(a.value));
	}

	if (mChildren.count() == 0) {
		printf("/>\n");
	} else {
		printf(">\n");

		for (QList<RecordNode *>::const_iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
			(*it)->write(indent + 2);
		}

		printf("%*.*s", indent, indent, "");
		printf("</%s>\n", qPrintable(mName));
	}

	return true;
}

bool
RecordNode::fromDomElement(const QDomElement &domElement)
{
	for (QDomNode c = domElement.firstChild(); !c.isNull(); c = c.nextSibling()) {
		if (c.isElement()) {
			RecordNode *child;

			child = new RecordNode(c.toElement());
			mChildren.append(child);
		}
	}

	QDomNamedNodeMap attributes(domElement.attributes());
	for (unsigned int i = 0; i < attributes.length(); ++i) {
		QDomAttr a(attributes.item(i).toAttr());

		addAttribute(a.name(), a.value());
	}
	return true;
}

EventRecord::EventRecord(const QString &type, QString timestamp)
: RecordNode("event")
{
	if (!timestamp.isEmpty())
		addAttribute("timestamp", timestamp);
	addAttribute("type", type);
}

EventRecord::EventRecord(const QDomElement &domElement)
: RecordNode("event")
{
	fromDomElement(domElement);
}

RecordNode *
EventRecord::addClassHints(const QString &className)
{
	RecordNode *hints;
	QString nameAttr;

	hints = addChildUnique("classhints");
	nameAttr = hints->attribute("name");

	if (nameAttr.isEmpty()) {
		// Newly created
		hints->addAttribute("name", className);
	} else
	if (nameAttr != className) {
		fprintf(stderr, "Duplicate classhints record; changing from %s to %s\n",
				qPrintable(nameAttr), qPrintable(className));
	}

	return hints;
}

const RecordNode *
EventRecord::classHints() const
{
	return child("classhints");
}

RecordNode *
EventRecord::addTargetHints()
{
	return addChildUnique("target");
}

const RecordNode *
EventRecord::targetHints() const
{
	return child("target");
}


bool
EventRecord::write() const
{
	return RecordNode::write(0);
}
