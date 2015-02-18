//////////////////////////////////////////////////////////////////
//
//	Support for scripts that drive the Qt event machinery
//
//
//
//
//
//////////////////////////////////////////////////////////////////

#define QT3_SUPPORT

#include <qapplication.h>
#include <qevent.h>
#include <qmenu.h>
#include <qmenubar.h>

#include <qdom.h>
#include <qfile.h>

#include <stdio.h>
#include "puppeteer.h"
#include "namespace.h"


Script::Action::~Action()
{
	if (mEventRecord)
		delete mEventRecord;
}

unsigned long
Script::Action::timeout() const
{
	if (mTimeout)
		return mTimeout;

	switch (mType) {
	case WaitEvent:
	case WaitApplicationExit:
		/* Default timeout to wait for an event is 2 sec which should be way more
		 * than we'll ever need.
		 */
		return 2000;

	case SendEvent:
	case SetFocus:
		/* Before sending an event, allow things to settle for .5 sec */
		return 500;

	default:
		/* All else: 1 sec */
		return 1000;
	}
}

Script::Action *
Script::Action::waitApplicationExit()
{
	return new Action(WaitApplicationExit);
}

Script::Action *
Script::Action::waitEvent(EventRecord *record)
{
	return new Action(WaitEvent, record);
}

Script::Action *
Script::Action::sendEvent(EventRecord *record)
{
	return new Action(SendEvent, record);
}

Script::Action *
Script::Action::setFocus(EventRecord *record)
{
	return new Action(SetFocus, record);
}

Script::Action *
Script::Action::verifyProperties(EventRecord *record)
{
	return new Action(VerifyProperties, record);
}

Script::~Script()
{
	while (!mActions.isEmpty())
		delete mActions.takeFirst();
}

void
Script::actionDone()
{
	if (!mActions.isEmpty())
		delete mActions.takeFirst();
}

Script::Action *
Script::currentAction() const
{
	if (mActions.isEmpty())
		return 0;
	return mActions.first();
}

bool
Script::Action::matchCurrentEvent(const EventRecord *rec) const
{
	const EventRecord *match = mEventRecord;

	if (mType != WaitEvent || match == 0)
		return false;

	const Attribute::list &matchAttrs(match->attributes());
	for (int i = 0; i < matchAttrs.count(); ++i) {
		const Attribute &attr(matchAttrs[i]);

		QString actualValue(rec->attribute(attr.name));
		if (actualValue != attr.value)
			return false;
	}

	return true;
}

bool
Script::load(const QString &scriptFile)
{
	QDomDocument doc("script");
	QFile file(scriptFile);

	if (!file.open(QIODevice::ReadOnly))
		return false;

	if (!doc.setContent(&file)) {
		file.close();
		return false;
	}
	file.close();

	QDomElement docElem = doc.documentElement();

	QDomNode n = docElem.firstChild();
	while (!n.isNull()) {
		QDomElement e = n.toElement(); // try to convert the node to an element.

		if (!e.isNull()) {
			/* Check what type of element we have */
			if (e.tagName() == "wait-application-exit") {
				mActions.append(Action::waitApplicationExit());
			} else
			if (e.tagName() == "wait-event") {
				EventRecord *rec = new EventRecord(e);

				if (rec == 0) {
					fprintf(stderr, "wait-event: cannot process event data\n");
					return false;
				}
				mActions.append(Action::waitEvent(rec));
			} else
			if (e.tagName() == "send-event") {
				EventRecord *rec = new EventRecord(e);

				if (rec == 0) {
					fprintf(stderr, "send-event: cannot process event data\n");
					return false;
				}
				mActions.append(Action::sendEvent(rec));
			} else
			if (e.tagName() == "set-focus") {
				EventRecord *rec = new EventRecord(e);

				if (rec == 0) {
					fprintf(stderr, "set-focus: cannot process event data\n");
					return false;
				}
				mActions.append(Action::setFocus(rec));
			} else
			if (e.tagName() == "verify") {
				EventRecord *rec = new EventRecord(e);

				if (rec == 0) {
					fprintf(stderr, "verify: cannot process data\n");
					return false;
				}
				mActions.append(Action::verifyProperties(rec));
			} else {
				fprintf(stderr, "Unexpected element <%s> in script\n", qPrintable(e.tagName()));
			}
		}
		n = n.nextSibling();
	}

	return true;
}
