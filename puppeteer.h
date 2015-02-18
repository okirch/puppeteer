
#ifndef QT_PUPPETEER_H
#define QT_PUPPETEER_H

#include <qobject.h>
#include <qevent.h>
#include <qmap.h>
#include <qtimer.h>

class QMenuBar;
class QMenu;
class QDomElement;
class QComboBox;

class Attribute {
public:
	typedef QList<Attribute> list;

	Attribute(const QString &n, const QString &v)
	: name(n), value(v) {}

	QString		name, value;
};


// Does this look a lot like an XML DOM node? Naaah, just coincidence.
class RecordNode {
public:
	typedef QList<RecordNode *> list;

	RecordNode(const QString &name)
	: mName(name) {}
	RecordNode(const QDomElement &);
	~RecordNode();

	const QString &		name() const { return mName; }

	void			addAttribute(const QString &name, const QString &value);
	QString			attribute(const QString &name) const;
	const Attribute::list &	attributes() const { return mAttributes; }

	const RecordNode::list &children() const { return mChildren; }
	RecordNode *		addChildUnique(const QString &name);
	RecordNode *		addChild(const QString &name);
	RecordNode *		child(const QString &name) const;

	bool			write(int indent = 0) const;

protected:
	bool			fromDomElement(const QDomElement &);

private:
	QString			mName;
	Attribute::list		mAttributes;
	RecordNode::list	mChildren;
};

class EventRecord : public RecordNode {
public:
	EventRecord(const QString &type, QString timestamp = QString());
	EventRecord(const QDomElement &);

	RecordNode *		addClassHints(const QString &className);
	const RecordNode *	classHints() const;

	RecordNode *		addTargetHints();
	const RecordNode *	targetHints() const;

	bool			write() const;
};

class Script {
public:
	enum Type {
		WaitApplicationExit, WaitEvent, SendEvent,
		SetFocus,
		VerifyProperties,
	};
	class Action {
	private:
		Action(Type type, EventRecord *record = 0)
		: mType(type), mEventRecord(record), mTimeout(0) {}

	public:
		~Action();

		Type		type() const { return mType; }
		const EventRecord *event() const { return mEventRecord; }

		unsigned long	timeout() const;
		void		setTimeout(unsigned long timeout) { mTimeout = timeout; }

		// WaitEvent processing
		bool		matchCurrentEvent(const EventRecord *) const;

		static Action *	waitApplicationExit();
		static Action *	waitEvent(EventRecord *);
		static Action *	sendEvent(EventRecord *);
		static Action *	setFocus(EventRecord *);
		static Action *	verifyProperties(EventRecord *);

	private:
		Type		mType;
		EventRecord *	mEventRecord;
		unsigned long	mTimeout;
	};

	~Script();

	bool			load(const QString &filename);

	Action *		currentAction() const;
	void			actionDone();

private:
	QList<Action *>		mActions;
};

class Puppeteer : public QObject {
	Q_OBJECT;

public:
	Puppeteer();
	~Puppeteer();

	static void		start();

	static QString		timestamp();

protected slots:
	void			aboutToQuitSlot();
	void			actionTimeoutSlot();

protected:
	void			startRecording();

	void			playbackStart(QString);
	void			playbackDescribeAction(const Script::Action *);
	bool			playbackNextAction();
	bool			playbackEvent(const EventRecord *rec);
	bool			playbackSetFocus(const EventRecord *rec);
	bool			playbackVerifyProperties(const EventRecord *rec);
	void			playbackFailure();
	void			playbackFinished();

	bool			eventFilter(QObject *, QEvent *);

private:
	EventRecord *		recordEvent(QObject *, QEvent *);
	void			recordObjectPath(QObject *, EventRecord *);
	void			recordMouseEvent(QObject *, QMouseEvent *, EventRecord *);
	void			recordShowEvent(QObject *, QShowEvent *, EventRecord *);
	void			recordFocusEvent(QObject *, QFocusEvent *, EventRecord *);
	void			recordKeyEvent(QObject *, QKeyEvent *, EventRecord *);
	void			recordAction(QAction *, RecordNode *);

	QWidget *		objectForRecord(const EventRecord *rec) const;
	QWidgetList		filterObjectsByClasshints(const QWidgetList &, const RecordNode *) const;
	void			filterObjectsByClasshints(QWidget *, const QString &, const RecordNode *, QWidgetList &) const;
	bool			matchObjectProperties(QWidget *, const RecordNode *) const;

	QEvent *		buildEvent(QWidget *&, const EventRecord *rec) const;

	// Mouse events are complex things, especially if you need to figure out on the fly where to click
	QMouseEvent *		buildMouseEvent(QWidget *&, QEvent::Type, const EventRecord *rec) const;
	bool			menuBarMouseEventTarget(QWidget *&, const RecordNode *, QPoint &) const;
	bool			menuMouseEventTarget(QWidget *&, const RecordNode *, QPoint &) const;
	bool			comboBoxMouseEventTarget(QWidget *&, const RecordNode *, QPoint &) const;

	QKeyEvent *		buildKeyEvent(QWidget *&, QEvent::Type, const EventRecord *) const;

	QString			buildObjectPath(QObject *object, EventRecord *rec);
	bool			buildObjectProperty(QObject *, const char *, RecordNode *);
	bool			getObjectProperty(const QObject *, const char *, QString &);
	QString			sanitizeButtonString(QString text) const;

private:
	bool			applicationActive;
	Script *		mScript;
	QTimer			mTimer;
};

#endif /* QT_PUPPETEER_H */
