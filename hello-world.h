
#ifndef HELLO_WORLD_H
#define HELLO_WORLD_H

#include <qmainwindow.h>
#include <qstring.h>

class QComboBox;
class QLabel;
class QLineEdit;

class HelloWorld : public QMainWindow {
	Q_OBJECT

public:
	HelloWorld();

public slots:
	void doSomethingIrrelevant();
	void requestExit();
	void happySlot();
	void grumpySlot();
	void morningTypeChanged(int);
	void morningTypeEdited();

protected:
	void updateMorningLabel();

private:
	QString		morningType;
	QLabel *	mMorningLabel;
	QComboBox *	mMorningTypeCombo;
	QLineEdit *	mMorningTypeEdit;
};

#endif /* HELLO_WORLD_H */
