
#include <qapplication.h>
#include <qmenubar.h>
#include <qmenu.h>
#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qlineedit.h>

#include "hello-world.h"
#include "puppeteer.h"

void startRecording(QApplication *);

HelloWorld::HelloWorld()
: morningType("beautiful"), mMorningLabel(0), mMorningTypeCombo(0)
{
	QMenu *menu;

	menu = menuBar()->addMenu(tr("&File"));
	menu->addAction(tr("&Irrelevant"), this, SLOT(doSomethingIrrelevant()));
	menu->addAction(tr("&Quit"), this, SLOT(requestExit()));

	QFrame *frame = new QFrame(this);
	setCentralWidget(frame);

	QVBoxLayout *layout1 = new QVBoxLayout;
	frame->setLayout(layout1);

	mMorningLabel = new QLabel(tr("Hello world. What a %1 morning.").arg(morningType), frame);
	mMorningLabel->setObjectName("helloLabel");
	layout1->addWidget(mMorningLabel);

	QHBoxLayout *layout2 = new QHBoxLayout;
	layout1->addLayout(layout2);

	mMorningTypeCombo = new QComboBox(frame);
	mMorningTypeCombo->setObjectName("morningCombo");
	mMorningTypeCombo->addItem(tr("beautiful"));
	mMorningTypeCombo->addItem(tr("terrible"));
	mMorningTypeCombo->addItem(tr("hung-over"));
	mMorningTypeCombo->addItem(tr("heavenly"));
	mMorningTypeCombo->addItem(tr("other"), QVariant(42));
	connect(mMorningTypeCombo, SIGNAL(currentIndexChanged(int)), SLOT(morningTypeChanged(int)));
	layout2->addWidget(mMorningTypeCombo);

	mMorningTypeEdit = new QLineEdit(frame);
	mMorningTypeEdit->setObjectName("morningEdit");
	mMorningTypeEdit->setEnabled(false);
	mMorningTypeEdit->setText("otherworldly");
	connect(mMorningTypeEdit, SIGNAL(editingFinished()), SLOT(morningTypeEdited()));
	layout2->addWidget(mMorningTypeEdit);

	layout2 = new QHBoxLayout;
	layout1->addLayout(layout2);

	QPushButton *btn;
	btn = new QPushButton(tr("&Yeah"), frame);
	btn->setObjectName("yesButton");
	layout2->addWidget(btn);

	connect(btn, SIGNAL(clicked()), SLOT(happySlot()));

	btn = new QPushButton(tr("&Go Away"), frame);
	btn->setObjectName("noCoffeeButton");
	layout2->addWidget(btn);

	connect(btn, SIGNAL(clicked()), SLOT(grumpySlot()));
}

void
HelloWorld::requestExit()
{
	qApp->exit();
}

void
HelloWorld::doSomethingIrrelevant()
{
	printf("bweeee-kazong.\n");
}

void
HelloWorld::happySlot()
{
	qApp->exit();
}

void
HelloWorld::grumpySlot()
{
	qApp->exit();
}

void
HelloWorld::morningTypeChanged(int index)
{
	if (mMorningTypeCombo->itemData(index).isValid()) {
		// enable the editable
		mMorningTypeEdit->setEnabled(true);
		morningType = mMorningTypeEdit->text();
	} else {
		mMorningTypeEdit->setEnabled(false);
		morningType = mMorningTypeCombo->itemText(index);
	}

	updateMorningLabel();
}

void
HelloWorld::morningTypeEdited()
{
	if (mMorningTypeEdit->isEnabled()) {
		morningType = mMorningTypeEdit->text();
		updateMorningLabel();
	}
}

void
HelloWorld::updateMorningLabel()
{
	mMorningLabel->setText(tr("Hello world. What a %1 morning.").arg(morningType));
}

int
main(int argc, char **argv)
{
	QApplication *app;
	HelloWorld *main;

	app = new QApplication(argc, argv);

	Puppeteer::start();

	main = new HelloWorld;
	main->setObjectName("mainWindow");

	main->show();

	app->exec();
	return 0;
}
